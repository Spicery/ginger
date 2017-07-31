from abc import abstractmethod
from minxml import MinXML

class Label:

    next_id = 0

    def __init__( self, name, is_return=False, is_continue=False ):
        self.label_name = name
        self.is_return = is_return
        self.is_continue = is_continue
        self.uid = '{}_{}'.format( self.label_name, Label.next_id )
        Label.next_id += 1

    def id( self ):
        return self.uid

Label.CONTINUE = Label( 'CONTINUE', is_continue=True )
Label.RETURN = Label( 'RETURN', is_return=True )

class SlotAllocations:
    '''Stuff to track the way variables are allocated to slots.'''

    def __init__( self ):
        self.hi_tide = 0    # One larger than the highest slot allocated so far.
        self.slots = {}     # slot -> title, titles do not have to be unique.
        self.available = [] # List of available slots (< hi_tide)

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
    ongiong shared context, such as slot-allocations should always be
    shared with mini-compilers, so if 'share' is omitted then 'parent' should
    be supplied.
    '''

    def __init__( self, share=None, parent=None ):
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
            self.allocations = SlotAllocations()
        else:
            # Invoked as a sub-compiler, so share all global context.
            # (At the moment that is just slot-allocations.)
            self.allocations = parent.allocations

    def add( self, *args ):
        '''Extends the instructions with an arbitrary number of GNX values'''
        self.instructions.add( *args )

    def plant( self, name, *kids, **attributes ):
        '''Creates a single GNX element and adds it to the instruction list'''
        self.add( MinXML( name, *kids, **attributes ) )

    def setLabel( self, label ):
        '''Adds a no-op into the tree with a label on it. This will have
        no effect during the calculation of jump distances and will 
        be eliminated entirely in a final backend phase.'''
        self.plant( "seq", to_label=label.id() )

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

class ExprCompiler( MiniCompiler ):
    '''
    Compiles a general expression by handing off to 
    subexpression compilers.
    '''

    def __init__( self, *args, **kwargs ):
        super().__init__( *args, **kwargs )

    def compile( self, expr, contn_label ):
        if expr.getName() == "constant":
            ConstantCompiler( share=self ).compile( expr, contn_label )
        elif expr.getName() == "and":
            AndCompiler( share=self )( expr, contn_label )
        elif expr.hasName( "for" ):
            LoopCompiler( share=self )( expr, contn_label )
        else:
            raise Exception( "To be implemented" )

class SingleValueCompiler( MiniCompiler ):
    '''Compiles a general expression but ensures it generates a single
    value'''

    def __init__( self, *args, **kwargs ):
        super().__init__( *args, **kwargs )

    def compile( self, expr, contn_label ):
        tmp0 = self.newTmpVar( 'mark' )
        self.plant( "start.mark", local=str(tmp0) )
        ExprCompiler( share=self )( expr, Label.CONTINUE )
        self.plant( "check.mark", local=str(tmp0) )
        self.simpleContinuation( contn_label )
        self.deallocateSlot( tmp0 )

class ConstantCompiler( MiniCompiler ):

    def __init__( self, *args, **kwargs ):
        super().__init__( *args, **kwargs )

    def compile( self, expr, contn_label ):   
        self.plant( "pushq", expr )
        self.simpleContinuation( contn_label )

class AndCompiler( MiniCompiler ):

    def __init__( self, *args, **kwargs ):
        super().__init__( *args, **kwargs )

    def compile( self, expr, contn_label ):   
        # First expression must carry on in this sequence
        # so we pass the fake label Label.CONTINUE.
        self.compileSingleValue( expr[0], Label.CONTINUE );
        
        # If false jump to the label immediately.
        self.plant( "and", to_label=contn_label.id() )

        # Run the rhs & continue to the label.
        self.compileExpression( expr[1], contn_label ) 

class LoopCompiler( MiniCompiler ):

    def __init__( self, *args, **kwargs ):
        super().__init__( *args, **kwargs )

    def compile( self, expr, contn_label ):
        query = expr[ 0 ]
        if query.hasName( "from" ):
            FromQueryCompiler( share=self )( query, contn_label )
        elif query.hasName( "in" ):
            InQueryCompiler( share=self )( query, contn_label )
        else:
            raise Exception( "To be implemented: {}".format( query.getName() ) )

class QueryCompiler( MiniCompiler ):

    def __init__( self, *args, **kwargs ):
        super().__init__( *args, **kwargs )

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
        EXIT_label = Label( 'exit' )
        self.compileLoopDeclarations( query )
        self.compileLoopInit( query, contn=TEST_label )
        self.setLabel( NEXT_label )
        self.compileLoopBody( query )
        self.compileLoopNext( query )
        self.setLabel( TEST_label )
        self.compileLoopTest( query, ifso=NEXT_label, ifnot=Label.CONTINUE )
        self.compileLoopFini( query, contn=contn_label )

class FromQueryCompiler( QueryCompiler ):

    def __init__( self, *args, **kwargs ):
        super().__init__( *args, **kwargs )

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

    def compileLoopTest( self, query, ifso=Label.CONTINUE, ifnot=Label.CONTINUE ):
        if not ifso is Label.CONTINUE:
            self.plant( "lte.ss", local0=str(self.loop_var_slot), local1=str(self.end_value_slot), to_label=ifso.id() )
            self.simpleContinuation( ifnot )
        else:
            raise Exception( "Not implemented yet" )

    def compileLoopBody( self, query ):
        pass

    def compileLoopNext( self, query ):
        # Great candidate for a merged instruction push-incr-pop.
        self.plant( "push.local", slot=str( self.loop_var_slot ) )
        self.plant( "incr" )
        self.plant( "pop.local", slot=str( self.loop_var_slot ) )

    def compileLoopFini( self, query, contn=Label.CONTINUE ):
        self.deallocateSlot( self.loop_var_slot )  

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