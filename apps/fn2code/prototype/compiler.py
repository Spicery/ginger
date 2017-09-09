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

    def compileDeterministicUpdate( self, expr, contn_label ):
        DeterministicUpdateCompiler( share=self ).compile( expr, contn_label )

    def plantIfNot( self, goto_label ):
        if goto_label == Label.RETURN:
            self.plant( "return.ifnot" )
        else:
            self.plant( "ifnot", to_label=goto_label.id() )

    def plantIfSo( self, goto_label ):
        if goto_label == Label.RETURN:
            self.plant( "return.ifso" )
        else:
            self.plant( "ifso", to_label=goto_label.id() )

    def plantIfSoNot( self, ifso_label, ifnot_label ):
        if ifso_label == Label.CONTINUE:
            self.plantIfNot( ifnot_label )
        elif ifnot_label == Label.CONTINUE:
            self.plantIfSo( ifso_label )
        else:
            self.plantIfNot( ifnot_label )
            self.simpleContinuation( ifso_label )

    def plantIfLocalEqValue( self, local, value, ifso_label, ifnot_label ):
        # TODO: replace with the specialised instruction eq_si.
        self.plant( "push.local", local=local )
        self.plant( "pushq", value )
        self.plant( "eq" )
        self.plantIfSoNot( ifso_label, ifnot_label )

    def plantIfLocalEqLocal( self, local0, local1, ifso_label, ifnot_label ):
        # TODO: replace with the specialised instruction eq_ss.
        self.plant( "push.local", local=local0 )
        self.plant( "push.local", local=local1 )
        self.plant( "eq" )
        self.plantIfSoNot( ifso_label, ifnot_label )

    def plantIfLocalNotEqValue( self, local, value, ifso_label, ifnot_label ):
        # TODO: replace with the specialised instruction neq_si.
        self.plant( "push.local", local=local )
        self.plant( "pushq", value )
        self.plant( "neq" )
        self.plantIfSoNot( ifso_label, ifnot_label )

    def plantIfLocalNotEqLocal( self, local0, local1, ifso, ifnot ):
        # TODO: replace with the specialised instruction neq_ss.
        self.plant( "push.local", local=local0 )
        self.plant( "push.local", local=local1 )
        self.plant( "neq" )
        self.plantIfSoNot( ifso_label, ifnot_label )

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

class DeterministicUpdateCompiler( MiniCompiler ):
    '''
    Compiles the target of an assignment (or binding?) when it is known
    that there are no choice points in the assignment. Background: assignment
    and pattern matching are unified in Ginger and some of the targets have
    multiple possible bindings e.g. ( a..., b... ) := x... 
    The effect of this is that the general form of pattern-matching and
    assignment requires looping and backtracking.
    '''

    def compile( self, expr, contn_label ):
        if expr.hasName( "id" ):
            if expr.has( "scope", "local" ):
                self.plant( "pop.local", local=expr.get( "slot" ) ) 
            else:
                self.plant( "pop.global", **expr.getAttributes() )
        elif expr.hasName( "seq" ):
            for i in reversed( range( 0, len( expr ) ) ):
                DeterministicUpdateCompiler( share=self ).compile( expr[ i ], Label.CONTINUE )
        else:
            raise Exception( "Assignment is not fully implemented yet" )
        self.simpleContinuation( contn_label )


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



@RegisteredMiniCompiler( "bind" )
class BindCompiler( MiniCompiler ):

    def compile( self, expr, contn_label ):   
        # Expr = <set> SOURCE TARGET </set>
        src = expr[ 1 ]
        src_a = Arity( src.get( "arity.eval", otherwise="0+" ) )
        dst = expr[ 0 ]
        dst_a = Arity( dst.get( "arity.pattern", otherwise="0+" ) )
        if src_a.isExact() and dst_a.isExact():
            self.compileExpression( src, Label.CONTINUE )
            self.compileDeterministicUpdate( dst, contn_label )
        else:
            raise Exception( "Assignment is not fully implemented" )



@RegisteredMiniCompiler( "set" )
class SetCompiler( MiniCompiler ):

    def compile( self, expr, contn_label ):   
        # Expr = <set> SOURCE TARGET </set>
        src = expr[ 0 ]
        src_a = Arity( src.get( "arity.eval", otherwise="0+" ) )
        dst = expr[ 1 ]
        dst_a = Arity( dst.get( "arity.eval", otherwise="0+" ) )
        if src_a.isExact() and dst_a.isExact():
            self.compileExpression( src, Label.CONTINUE )
            self.compileDeterministicUpdate( dst, contn_label )
        else:
            raise Exception( "Assignment is not fully implemented: {} {}".format( str(src_a), str(dst_a) ) )


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



@RegisteredMiniCompiler( "switch" )
class SwitchCompiler( MiniCompiler ):
    '''
    This should implement full pattern matching - with variadicity.
    However our initial version will be trivial.
    '''

    def __init__( self, *args, **kwargs ):
        super().__init__( *args, **kwargs )
        self.tmp0 = None

    def deallocate( self ):
        if not self.tmp0 is None:
            self.deallocateSlot( self.tmp0 )

    def compileFrom( self, expr, offset, n_remaining, contn_label ):
        if n_remaining == 2:
            self.compileSingleValue( expr[offset], Label.CONTINUE )
            self.plant( "push.local", local=self.tmp0, comment="switch2_push" )
            self.deallocate()
            self.plant( "eq", comment="switch2_eq" )
            self.plantIfNot( contn_label )
            self.compileExpression( expr[offset + 1], contn_label )            
        elif n_remaining == 3:
            ELSE_label = Label( 'else' )
            self.compileSingleValue( expr[offset], Label.CONTINUE )
            self.plant( "push.local", local=self.tmp0, comment="switch3_push" )
            self.deallocate()
            self.plant( "eq", comment="switch3_eq" )
            self.plantIfNot( ELSE_label )
            self.compileExpression( expr[offset + 1], contn_label )
            self.setLabel( ELSE_label )
            self.compileExpression( expr[offset + 2], contn_label )            
        else:
            ELSEIF_label = Label( 'elseif' )
            self.compileSingleValue( expr[offset], Label.CONTINUE )
            self.plant( "push.local", local=self.tmp0, comment="switchN_push" )
            self.plant( "eq", comment="switchN_eq" )
            self.plantIfNot( ELSEIF_label )
            self.compileExpression( expr[offset + 1], contn_label )
            self.setLabel( ELSEIF_label )
            self.compileFrom( expr, offset+2, n_remaining-2, contn_label )

    def compile( self, expr, contn_label ): 
        self.compileSingleValue( expr[0], Label.CONTINUE )
        self.tmp0 = self.newTmpVar( 'switch' )
        self.plant( "pop.local", local=self.tmp0, comment="switch_tmp" )
        n = len( expr )
        if n == 1:
            # Extremely rare case, so there's no need to optimise.
            # Corresponds to: switch EXPR endswitch
            self.deallocate()
        elif n == 2:
            # Extremely rare case, so there's no need to optimise.
            # Corresponds to: switch EXPR else EXPR endswitch
            self.deallocate()
            self.compileExpression( expr[1], contn_label )
        else:
            self.compileFrom( expr, 1, n-1, contn_label )


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
            self.compileExpression( expr[offset + 2], contn_label )
        else:
            ELSEIF_label = Label( 'elseif' )
            self.compileSingleValue( expr[offset], Label.CONTINUE )
            self.plantIfNot( ELSEIF_label )
            self.compileExpression( expr[offset + 1], contn_label )
            self.setLabel( ELSEIF_label )
            self.compileFrom( expr, offset+2, n_remaining-2, contn_label )

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

    def newInstance( self, query ):
        if query.hasName( "from" ):
            return FromQueryCompiler( share=self )
        elif query.hasName( "in" ):
            return InQueryCompiler( share=self )
        elif query.hasName( "do" ):
            return DoQueryCompiler( share=self )
        else:
            raise Exception( "To be implemented: {}".format( query.getName() ) )

    def compile( self, expr, contn_label ):
        query = expr[ 0 ]
        if query.hasName( "from" ):
            FromQueryCompiler( share=self )( query, contn_label )
        elif query.hasName( "in" ):
            InQueryCompiler( share=self )( query, contn_label )
        elif query.hasName( "do" ):
            DoQueryCompiler( share=self )( query, contn_label )
        elif query.hasName( "zip" ):
            ZipQueryCompiler( share=self )( query, contn_label )
        elif query.hasName( "cross" ):
            CrossQueryCompiler( share=self )( query, contn_label )
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
    def compileLoopBody( self, query, contn=Label.CONTINUE ): pass

    @abstractmethod
    def compileLoopNext( self, query, contn=Label.CONTINUE ): pass

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


class DoQueryCompiler( QueryCompiler ):

    def compileLoopDeclarations( self, query ):
        self.LHS = LoopCompiler( share=self ).newInstance( query[0] )
        self.LHS.compileLoopDeclarations( query[0] )

    def compileLoopInit( self, query, contn=Label.CONTINUE ):
        self.LHS.compileLoopInit( query[0], contn )

    def compileLoopTest( self, query, ifso=Label.CONTINUE, ifnot=Label.CONTINUE ):
        self.LHS.compileLoopTest( query[0], ifso, ifnot )

    def compileLoopBody( self, query, contn=Label.CONTINUE ):
        self.LHS.compileLoopBody( query[0], Label.CONTINUE )
        self.compileExpression( query[1], contn )        

    def compileLoopNext( self, query, contn=Label.CONTINUE ):
        self.LHS.compileLoopNext( query[0], contn )
        
    def compileLoopFini( self, query, contn=Label.CONTINUE ):
        self.LHS.compileLoopFini( query[0], contn )


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

    def compileLoopBody( self, query, contn=Label.CONTINUE ):
        self.simpleContinuation( contn )

    def compileLoopNext( self, query, contn=Label.CONTINUE ):
        self.plant( "incr.local.by", local=str( self.loop_var_slot ), by="1" )
        self.simpleContinuation( contn )

    def compileLoopFini( self, query, contn=Label.CONTINUE ):
        self.deallocateSlot( self.loop_var_slot )  
        self.simpleContinuation( contn )

# This is the terminating value that indicates the end of a stream.
TERMIN = MinXML( "constant", type="termin", value="termin" )
SYS_TRUE = MinXML( "constant", type="boolean", value="true" )
SYS_FALSE = MinXML( "constant", type="boolean", value="false" )

class InQueryCompiler( QueryCompiler ):

    def compileLoopDeclarations( self, query ):
        # Should really bind against a pattern - but we'll take the simplest 
        # case of a single <var> element.
        if query[0].hasName( "var" ):
            self.loop_var_slot = self.allocateSlot( query[0] )
            self.tmp_next_fn = self.newTmpVar( 'tmp_next_fn' )
            self.tmp_context = self.newTmpVar( 'tmp_context' )
            self.tmp_state = self.newTmpVar( 'tmp_state' );       
        else:
            raise Exception( "TBD: {}".format( query.name ) )

    def compileLoopInit( self, query, contn=Label.CONTINUE ):
        self.compileSingleValue( query[1], Label.CONTINUE )
        self.plant( "getiterator" )
        self.plant( "pop.local", local=self.tmp_next_fn )
        self.plant( "pop.local", local=self.tmp_context )
        self.plant( "pop.local", local=self.tmp_state )
        self.simpleContinuation( contn )        

    def compileLoopTest( self, query, pass_label=Label.CONTINUE, fail_label=Label.CONTINUE ):
        self.plant( "push.local", local=self.tmp_state )
        self.plant( "push.local", local=self.tmp_context )
        self.plant( "set.count.call.local", count=2, local=self.tmp_next_fn )       
        self.plant( "pop.local", local=self.tmp_state )
        self.plant( "pop.local", local=self.loop_var_slot )
        self.plantIfLocalNotEqValue( self.tmp_state, TERMIN, pass_label, fail_label )       

    def compileLoopBody( self, query, contn=Label.CONTINUE ):
        self.simpleContinuation( contn )

    def compileLoopNext( self, query, contn=Label.CONTINUE ):
        self.simpleContinuation( contn )

    def compileLoopFini( self, query, contn=Label.CONTINUE ):
        self.deallocateSlot( self.tmp_next_fn )
        self.deallocateSlot( self.tmp_context )
        self.deallocateSlot( self.tmp_state )
        self.simpleContinuation( contn )


class ZipQueryCompiler( QueryCompiler ):

    def compileLoopDeclarations( self, query ):
        self.LHS = LoopCompiler( share=self ).newInstance( query[0] )
        self.RHS = LoopCompiler( share=self ).newInstance( query[1] )
        self.LHS.compileLoopDeclarations( query[0] )
        self.RHS.compileLoopDeclarations( query[1] )

    def compileLoopInit( self, query, contn=Label.CONTINUE ):
        self.LHS.compileLoopInit( query[0], Label.CONTINUE )
        self.LHS.compileLoopInit( query[1], contn )

    def compileLoopTest( self, query, ifso=Label.CONTINUE, ifnot=Label.CONTINUE ):
        DONE_label = Label( 'zip.done' )
        self.LHS.compileLoopTest( query[0], Label.CONTINUE, DONE_label.replacesCONTNUE( ifnot ) )
        self.RHS.compileLoopTest( query[1], ifso, contn )
        self.setLabel( DONE_label )

    def compileLoopBody( self, query, contn=Label.CONTINUE ):
        self.LHS.compileLoopBody( query[0], label=Label.CONTINUE )
        self.RHS.compileLoopBody( query[1], label=contn )
        
    def compileLoopNext( self, query, contn=Label.CONTINUE ):
        self.LHS.compileLoopNext( query[0], label=Label.CONTINUE )
        self.RHS.compileLoopNext( query[1], label=contn )
        
    def compileLoopFini( self, query, contn=Label.CONTINUE ):
        # I note that the C++ JIT compiler does not recursively apply
        # the finish parts. Surely that is not correct?!
        # TODO: verify this interpretation is the right one.
        self.LHS.compileLoopFini( query[0], Label.CONTINUE )
        self.RHS.compileLoopFini( query[1], contn )

class CrossQueryCompiler( QueryCompiler ):

    def compileLoopDeclarations( self, query ):
        self.OUTER = LoopCompiler( share=self ).newInstance( query[0] )
        self.INNER = LoopCompiler( share=self ).newInstance( query[1] )
        self.OUTER.compileLoopDeclarations( query[0] )
        self.INNER.compileLoopDeclarations( query[1] )

    def compileLoopInit( self, query, contn=Label.CONTINUE ):
        '''Only initialise the outer loop'''
        # We need a flag to say if we're on the outer loop (True) or 
        # inner loop (False).
        self.on_outer_loop_flag = self.newTmpVar( 'on_outer_loop_flag' )
        self.plant( "pushq.pop.local", SYS_TRUE, local=self.on_outer_loop_flag )
        self.OUTER.compileLoopInit( query[0], contn )

    def compileLoopTest( self, query, ifso=Label.CONTINUE, ifnot=Label.CONTINUE ):
        outer_loop_label = Label( 'cross.outer.loop.label' )
        inner_loop_label = Label( 'cross.inner.loop.label' )
        done_loop_label = Label( 'cross.done.loop.label' )

        # Are we running the inner or outer loop?
        self.plant( "push.local", local=self.on_outer_loop_flag )
        self.plantIfNot( inner_loop_label )

        # At this point we are on the outer loop. So we should run the 
        # outer test and, if it passes, drop into the inner loop.
        self.setLabel( outer_loop_label )
        self.OUTER.compileLoopTest( query[0], Label.CONTINUE, done_loop_label.replaceCONTINUE( contn ) )
        # Set the flag so we know we're on the inner loop.
        self.plant( "pushq.pop.local", SYS_FALSE, local=self.on_outer_loop_flag )
        # Before we enter the inner loop, we must run its body and
        # next parts and then initialise the inner loop.
        self.OUTER.compileLoopBody( query[0], Label.CONTINUE )
        self.OUTER.compileLoopNext( query[0], Label.CONTINUE )
        self.INNER.compileLoopInit( query[1], Label.CONTINUE )

        # Now we're in the inner loop. Let's see if we pass.
        self.setLabel( inner_loop_label )
        self.INNER.compileLoopTest( query[1], done_loop_label.replaceCONTINUE( contn ), Label.CONTINUE )

        # If we get here then the inner loop has failed. So we ought to run
        # the finish action of the inner loop, set the inner-vs-outer flag to 
        # outer, and then jump back to the outer-loop test.
        self.plant( "pushq.pop.local", SYS_TRUE, local=self.on_outer_loop_flag )
        self.simpleContinuation( outer_loop_label )

        self.setLabel( done_loop_label )

    def compileLoopBody( self, query, contn=Label.CONTINUE ):
        self.INNER.compileLoopBody( query[1], label=contn )

    def compileLoopNext( self, query, contn=Label.CONTINUE ):
        self.INNER.compileLoopNext( query[1], label=contn )
        
    def compileLoopFini( self, query, contn=Label.CONTINUE ):
        # When the outer loop finally fails, we should run
        # the finish-action of the outer loop.
        self.OUTER.compileLoopFini( query[0], contn )
        self.deallocateSlot( self.on_outer_loop_flag )

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
