from abc import abstractmethod
from minxml import MinXML
from label import Label
from arity import Arity
import sys
import abc

class SlotAllocations:
    '''Stuff to track the way variables are allocated to slots.'''

    def __init__( self, preallocated=0 ):
        self.hi_tide = preallocated     # One larger than the highest slot allocated so far.
        self.slots = {                  # slot -> title, titles do not have to be unique.
            n: 'preallocated' for n in range( 0, preallocated )
        }
        self.available = []             # List of available slots (< hi_tide)

    def slotsNeeded( self ):
        return self.hi_tide

    def fetchSlot( self, title ):
        if not self.available:
            if len( self.slots ) >= self.hi_tide:   # defensive >=, could be ==.
                self.available.append( self.hi_tide )
                self.hi_tide += 1
            else:
                for n in range( 0, self.hi_tide ):
                    if not n in self.slots:
                        self.available.append( n )
            assert self.available
        n = self.available.pop()
        self.slots[ n ] = title
        return n

    def newTmpVar( self, title ):
        return self.fetchSlot( title )

    def allocateSlot( self, var ):
        return self.fetchSlot( var.get( 'name' ) )

    def deallocateSlot( self, slot ):
        del self.slots[ slot ]



class MiniCompiler:
    '''This is an abstract class for 'compilers' that are specialised to
    a particular type of expression. The main way they are invoked is by
    call (i.e. via __call__). They work by appending to an internal list
    of instructions. When they invoke another mini-compiler they can 
    optionally share their list of instructions (via the optional parameter
    share), which just saves a bit of unnecessary copying. However the
    ongoing shared context, such as slot-allocations should always be
    shared with mini-compilers, so if 'share' is omitted then 'parent' should
    be supplied.
    '''

    def __init__( self, share=None, parent=None, preallocated=0 ):
        if parent == None:
            # If parent is not supplied then the intention is to share everything
            # (or this is a top-level invocation)
            parent = share
        if share == None:
            # Not sharing an instruction stream, so create a new one.
            self.instructions = MinXML( "seq" )
        else:
            # Share the instruction stream.
            self.instructions = share.instructions
        if parent == None:
            # This is a top-level invocation i.e. used to create the body
            # of a lambda expression.
            self.allocations = SlotAllocations( preallocated )
        else:
            # Invoked as a sub-compiler, so share all global context.
            # (At the moment that is just slot-allocations.)
            self.allocations = parent.allocations

    def plant( self, _name, *_kids, **_attributes ):
        '''
        Creates a single GNX element and adds it to the instruction list. Underscores
        in attributes are replaced by full-stops - which is simply a convenience.
        '''
        self.instructions.add( 
            MinXML( 
                _name, 
                *_kids, 
                **{ k.replace( '_', '.' ): v for ( k, v ) in _attributes.items() } 
            ) 
        )

    def setLabel( self, label ):
        '''Adds a no-op into the tree with a label on it. This will have
        no effect during the calculation of jump distances and will 
        be eliminated entirely in a final backend phase.'''
        self.plant( "seq", label=label.id() )

    def newTmpVar( self, title ):
        return self.allocations.newTmpVar( title )

    def allocateSlot( self, var ):
        return self.allocations.allocateSlot( var )

    def deallocateSlot( self, slot ):
        self.allocations.deallocateSlot( slot )

    def __call__( self, *args, **kwargs ):
        '''
        Invokes the abstract method compile and then returns the instruction list.
        This is the primary way of using these objects.
        '''
        self.compile( *args, **kwargs )
        return self.instructions

    def compileSingleValue( self, expr, contn_label ):
        SingleValueCompiler( share=self ).compile( expr, contn_label )

    def compileExpression( self, expr, contn_label ):
        ExprCompiler( share=self ).compile( expr, contn_label )

    def plantIfNot( self, goto_label ):
        if goto_label == Label.RETURN:
            self.plant( "return.ifnot" )
        else:
            self.plant( "ifnot", to_label=goto_label.id() )

    def plantIfSo( self, label ):
        if label == Label.RETURN:
            self.plant( "return.ifso" )
        else:
            self.plant( "ifso", label=contn_label.id() )

    def compileChildren( self, expr, contn_label ):
        if expr:
            kids = expr.children
            for i in kids[:-1]:
                self.compileExpression( i, Label.CONTINUE )
            self.compileExpression( kids[-1], contn_label )
        else:
            self.simpleContinuation( contn_label )

    def simpleContinuation( self, contn_label ):
        '''Compiles an explicit jump to the label'''
        if contn_label == Label.CONTINUE:
            pass
        elif contn_label == Label.RETURN:
            self.plant( "return" )
        else:
            self.plant( "goto", to_label=contn_label.id() )

    @abstractmethod
    def compile( self, *args, **kwargs ):
        pass

SUB_COMPILER_INDEX = {}

def RegisteredMiniCompiler( element_name ):
    def sub_compiler_decorator( func ):
        SUB_COMPILER_INDEX[ element_name ] = func
        return func
    return sub_compiler_decorator

class ExprCompiler( MiniCompiler ):
    '''
    Compiles a general expression by handing off to 
    subexpression compilers.
    '''

    def __init__( self, *args, **kwargs ):
        super().__init__( *args, **kwargs )

    def compile( self, expr, contn_label ):
        try:
            SUB_COMPILER_INDEX[ expr.getName() ]( share=self )( expr, contn_label )
        except KeyError:
            raise Exception( "To be implemented: " + expr.getName() )

class SingleValueCompiler( MiniCompiler ):
    '''Compiles a general expression but ensures it generates a single
    value'''

    def __init__( self, *args, **kwargs ):
        super().__init__( *args, **kwargs )

    def compile( self, expr, contn_label ):
        if expr.has( "arity.eval", value="1" ):
            self.compileExpression( expr, contn_label )
        else:
            tmp0 = self.newTmpVar( 'mark' )
            self.plant( "start.mark", local=str(tmp0) )
            ExprCompiler( share=self )( expr, Label.CONTINUE )
            self.plant( "check.mark1", local=str(tmp0) )
            self.simpleContinuation( contn_label )
            self.deallocateSlot( tmp0 )

@RegisteredMiniCompiler( "seq" )
class SeqCompiler( MiniCompiler ):

    def compile( self, expr, contn_label ):
        self.compileChildren( expr, contn_label )

@RegisteredMiniCompiler( "sysapp" )
class SysAppCompiler( MiniCompiler ):

    def compile( self, expr, contn_label ):
        a = Arity( *( a.get( "arity.eval", otherwise="0+" ) for a in expr.getChildren() ) )
        aa = Arity( expr.get( "args.arity" ) )
        if aa.isExact() and a.hasExactCount( aa.count() ):
            self.compileChildren( expr, Label.CONTINUE )
            self.plant( "set.count.syscall", count=a.count(), name=expr.get( "name" ) )
        else:
            name = expr.get( "name" )
            tmp0 = self.newTmpVar( 'args_mark' )
            self.plant( "start.mark", local=tmp0 )
            self.compileChildren( expr, Label.CONTINUE )
            self.plant( "check.mark", local=tmp0, count=expr.get( "args.arity" ) )
            self.plant( "syscall", name=name )
            self.deallocateSlot( tmp0 )
        self.simpleContinuation( contn_label )

@RegisteredMiniCompiler( "self.app" )
class SelfAppCompiler( MiniCompiler ):

    def compile( self, expr, contn_label ):
        a = Arity( *( a.get( "arity.eval", otherwise="0+" ) for a in expr.getChildren() ) )
        aa = Arity( expr.get( "args.arity" ) )
        if aa.isExact() and a.hasExactCount( aa.count() ):
            self.compileChildren( expr, Label.CONTINUE )
            self.plant( "self.call.n", count=a.count() )
        else:
            tmp0 = self.newTmpVar( 'args_mark' )
            self.plant( "start.mark", local=tmp0 )
            self.compileChildren( expr, Label.CONTINUE )
            self.plant( "set.count.mark", local=tmp0 )
            self.plant( "self.call" )
            self.deallocateSlot( tmp0 )
        self.simpleContinuation( contn_label )

@RegisteredMiniCompiler( "app" )
class AppCompiler( MiniCompiler ):

    def compile( self, expr, contn_label ):
        fn_expr = expr[0]
        arg_expr = expr[1]
        arg_arity = Arity( arg_expr.get( "arity.eval", otherwise="+0" ) )
        # print( 'EXACT', arg_arity.count(), arg_arity.isExact(), file=sys.stderr )
        if fn_expr.hasName( "id" ) and fn_expr.has( "scope", value="global" ) and arg_arity.isExact():
            self.compileExpression( arg_expr, Label.CONTINUE )
            self.plant( "set.count.call.global", count=arg_arity.count(), **fn_expr.getAttributes() )
        elif fn_expr.hasName( "id" ) and fn_expr.has( "scope", value="local" ) and arg_arity.isExact():
            # TODO: UNTESTED As the test framework isn't ready!
            self.compileExpression( arg_expr, Label.CONTINUE )
            slot = expr.get( "slot" )
            self.plant( "set.count.call.local", local=slot )
        else:
            tmp0 = self.newTmpVar( 'args_mark' )
            self.plant( "start.mark", local=tmp0 )
            self.compileExpression( arg_expr, Label.CONTINUE )
            self.compileSingleValue( fn_expr, Label.CONTINUE )
            self.plant( "end1.calls", local=tmp0 )
            self.simpleContinuation( contn_label )
            self.deallocateSlot( tmp0 )

class ContainerMiniCompiler( MiniCompiler ):

    def __init__( self, *args, **kwargs ):
        super().__init__( *args, **kwargs )

    @abc.abstractmethod
    def sysFnConstructor( self ):
        pass

    def compile( self, expr, contn_label ):   
        tmp0 = self.newTmpVar( 'mark' )
        self.plant( "start.mark", local=tmp0 )
        self.compileChildren( expr, Label.CONTINUE )
        self.plant( "set.count.mark", local=tmp0 )
        self.plant( "syscall", name=self.sysFnConstructor() )
        self.simpleContinuation( contn_label )
        self.deallocateSlot( tmp0 )

@RegisteredMiniCompiler( "vector" )
class VectorCompiler( ContainerMiniCompiler ):

    def sysFnConstructor( self ):
        return "newVector"

@RegisteredMiniCompiler( "list" )
class ListCompiler( ContainerMiniCompiler ):

    def sysFnConstructor( self ):
        return "newList"

@RegisteredMiniCompiler( "constant" )
class ConstantCompiler( MiniCompiler ):

    def compile( self, expr, contn_label ):   
        self.plant( "pushq", expr )
        self.simpleContinuation( contn_label )

@RegisteredMiniCompiler( "id" )
class IdCompiler( MiniCompiler ):

    def compile( self, expr, contn_label ):   
        slot = expr.get( "slot", otherwise=None )
        if slot:
            self.plant( "push.local", local=slot )
        else:
            self.plant( "push.global", **expr.getAttributes() )
        self.simpleContinuation( contn_label )

@RegisteredMiniCompiler( "and" )
class AndCompiler( MiniCompiler ):

    def compile( self, expr, contn_label ):   
        # First expression must carry on in this sequence
        # so we pass the fake label Label.CONTINUE.
        self.compileSingleValue( expr[0], Label.CONTINUE );
        # If false jump to the label immediately.
        self.plant( "and", to_label=contn_label.id() )
        # Run the rhs & continue to the label.
        self.compileExpression( expr[1], contn_label ) 

@RegisteredMiniCompiler( "or" )
class OrCompiler( MiniCompiler ):

    def compile( self, expr, contn_label ):   
        # First expression must carry on in this sequence
        # so we pass the fake label Label.CONTINUE.
        self.compileSingleValue( expr[0], Label.CONTINUE );
        # If true jump to the label immediately.
        self.plant( "or", to_label=contn_label.id() )
        # Run the rhs & continue to the label.
        self.compileExpression( expr[1], contn_label ) 

@RegisteredMiniCompiler( "if" )
class IfCompiler( MiniCompiler ):

    def compileFrom( self, expr, offset, n_remaining, contn_label ):
        if n_remaining == 2:
            self.compileSingleValue( expr[offset], Label.CONTINUE )
            self.plantIfNot( contn_label )
            self.compileExpression( expr[offset + 1], contn_label )
        elif n_remaining == 3:
            ELSE_label = Label( 'else' )
            self.compileSingleValue( expr[offset], Label.CONTINUE )
            self.plantIfNot( ELSE_label )
            self.compileExpression( expr[offset + 1], contn_label )
            self.setLabel( ELSE_label )
            self.compile( expr[offset + 2], contn_label )
        else:
            ELSEIF_label = Label( 'elseif' )
            self.compileSingleValue( expr[offset], Label.CONTINUE )
            self.plantIfNot( ELSEIF_label )
            self.compileExpression( expr[offset + 1], contn_label )
            self.setLabel( ELSEIF_label )
            self.compileFrom( self, expr, offset+2, n_remaining-2, contn_label )

    def compile( self, expr, contn_label ): 
        n = len( expr )
        if n == 0:
            pass
        elif n == 1:
            self.compileExpression( expr[0], contn_label )
        else:
            self.compileFrom( expr, 0, n, contn_label )

@RegisteredMiniCompiler( "for" )
class LoopCompiler( MiniCompiler ):

    def compile( self, expr, contn_label ):
        query = expr[ 0 ]
        if query.hasName( "from" ):
            FromQueryCompiler( share=self )( query, contn_label )
        elif query.hasName( "in" ):
            InQueryCompiler( share=self )( query, contn_label )
        else:
            raise Exception( "To be implemented: {}".format( query.getName() ) )

class QueryCompiler( MiniCompiler ):

    @abstractmethod
    def compileLoopDeclarations( self, query ): pass

    @abstractmethod
    def compileLoopInit( self, query, contn=Label.CONTINUE ): pass

    @abstractmethod
    def compileLoopTest( self, query, ifso=Label.CONTINUE, ifnot=Label.CONTINUE ): pass

    @abstractmethod
    def compileLoopBody( self, query ): pass

    @abstractmethod
    def compileLoopNext( self, query ): pass

    @abstractmethod
    def compileLoopFini( self, query, contn=Label.CONTINUE ): pass

    def compile( self, query, contn_label ):
        '''See below for an explanation of this way this works'''
        TEST_label = Label( 'test' )
        NEXT_label = Label( 'next' )
        self.compileLoopDeclarations( query )
        self.compileLoopInit( query, contn=TEST_label )
        self.setLabel( NEXT_label )
        self.compileLoopBody( query )
        self.compileLoopNext( query )
        self.setLabel( TEST_label )
        self.compileLoopTest( query, ifso=NEXT_label, ifnot=Label.CONTINUE )
        self.compileLoopFini( query, contn=contn_label )

class FromQueryCompiler( QueryCompiler ):

    def compileLoopDeclarations( self, query ):
        '''Hand-waving allocation of slot to the variable'''
        self.loop_var_slot = self.allocateSlot( query[0] )
        self.end_value_slot = self.newTmpVar( 'from_end_value' )

    def compileLoopInit( self, query, contn=Label.CONTINUE ):
        '''For simplicity we assume the BY part is always the constant 1 and
        that there's always 4 arguments: loop variable, start, by & end.
        '''
        SingleValueCompiler( share=self )( query[1], Label.CONTINUE )
        self.plant( "pop.local", local=str(self.loop_var_slot) )            
        SingleValueCompiler( share=self )( query[3], Label.CONTINUE )
        self.plant( "pop.local", local=str(self.end_value_slot) )
        self.simpleContinuation( contn )

    def compileLoopTest( self, query, ifso=Label.CONTINUE, ifnot=Label.CONTINUE ):
        if not ifso is Label.CONTINUE:
            self.plant( "lte.ss", local0=str( self.loop_var_slot ), local1=str( self.end_value_slot ), to_label=ifso.id() )
            self.simpleContinuation( ifnot )
        else:
            raise Exception( "Not implemented yet" )

    def compileLoopBody( self, query ):
        pass

    def compileLoopNext( self, query ):
        self.plant( "incr.local.by", local=str( self.loop_var_slot ), by="1" )


    def compileLoopFini( self, query, contn=Label.CONTINUE ):
        self.deallocateSlot( self.loop_var_slot )  

def compile( gnx, nargs ):
    ecompiler = ExprCompiler( preallocated=nargs )
    cbody = ecompiler( gnx, Label.RETURN )
    max_slots = ecompiler.allocations.slotsNeeded()
    return ( cbody, max_slots )

if __name__ == "__main__":
    for_expr = MinXML( "for" )
    for_expr.add(
        MinXML( 
            "from", 
            MinXML( "var", name="i" ),
            MinXML( "constant", type="int", value="0" ),
            MinXML( "constant", type="int", value="1" ),
            MinXML( "constant", type="int", value="10" )
        )
    )
    print( 
        str(
            ExprCompiler()( for_expr, Label.RETURN )
        )
    )
