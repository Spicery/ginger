from minxml import MinXML
import sys

# Set of peephole transformations. Note that the peephole transformations
# are free to ignore the issue of labels. The function that applies them
# will capture any existing label and re-apply those labels (in addition to
# any added by the optimisation).

PEEPHOLE = {}
    
def PeepHole( instruction_name ):
    def peephole_decorator( func ):
        PEEPHOLE[ instruction_name ] = func
        return func
    return peephole_decorator

@PeepHole( "push.local" )
def optPushLocal( ixml ):
    local = ixml.get( "local" )
    if local == "0":
        return MinXML( "push.local0" )
    elif local == "1":
        return MinXML( "push.local1" )
    else:
        return ixml

@PeepHole( "push.local.ret" )
def optPushLocal( ixml ):
    local = ixml.get( "local" )
    if local == "0":
        return MinXML( "push.local0.ret" )
    elif local == "1":
        return MinXML( "push.local1.ret" )
    else:
        return ixml

@PeepHole( "incr.by" )
def optIncrBy( ixml ):
    d = ixml.get( "by" )
    if d == "0":
        return MinXML( "noop" )
    elif d == "1":
        return MinXML( "incr" )
    elif d == "-1":
        return MinXML( "decr" )
    else:
        return ixml

@PeepHole( "incr.local.by" )
def optIncrBy( ixml ):
    d = ixml.get( "by" )
    if d == 0:
        return MinXML( "noop" )
    elif d == "1":
        return MinXML( "incr.local.by1", local=ixml.get( "local" ) )
    else:
        return ixml

@PeepHole( "erase.num" )
def optEraseNum( ixml ):
    n = ixml.get( "n" )
    if n == '0':
        return MinXML( "noop" )
    elif n == '1':
        return MinXML( "erase" )
    else:
        return ixml       

class PeepHole:
    def __init__( self ):
        self.instructions = MinXML( 'seq' )

    def add( self, ixml ):
        self.instructions.add( ixml )

    def addPendingLabels( self, ixml ):
        if ixml.hasAttribute( 'label' ):
            self.pending_labels.extend( ixml.get( 'label' ).split() )

    def assemble( self, ixml ):
        name = ixml.getName()
        if name == 'seq':
            self.addPendingLabels( ixml )
            for i in ixml:
                self.assemble( i )
        elif name == 'noop':
            self.addPendingLabels( ixml )
        else:
            if name in peephole:
                saved_label = ixml.get( 'label', otherwise='' )
                ixml = peephole[ name ]( ixml )
                if saved_label:
                    # Extend the labels on ixml by saved_label.
                    labs = saved_label.split()
                    labs.extend( ixml.get( 'label', otherwise='' ).split() )
                    ixml.put( 'label', ' '.join( labs ) )
                self.assemble( ixml )
            else:
                self.add( ixml )

    def __call__( self, *args, **kwargs ):
        '''
        Invokes the abstract method assemble and then returns the instruction list.
        This is the primary way of using these objects.
        '''
        self.assemble( *args, **kwargs )
        if self.pending_labels:
            print( 'WARNING: pending-labels were not resolved!', file=sys.stderr )
            self.add( MinXML( "return", label=' '.join( self.pending_labels ) ) )
        return self.instructions

class Flatten:
    '''This callable flattens out all the instructions.
    '''

    def __init__( self ):
        self.instructions = MinXML( 'seq' )
        self.pending_labels = []

    def _usePendingLabels( self, ixml ):
        '''Modifies the instruction in place'''
        labs = ixml.get( 'label', otherwise='' ).split()
        labs.extend( self.pending_labels )
        ' '.join( labs )
        xml.put( 'label',  ' '.join( labs ) )
        self.pending_labels = []        

    def add( self, ixml ):
        if self.pending_labels:
            ixml = ixml.copy()
            self._usePendingLabels( ixml )
        self.instructions.add( ixml )

    def addPendingLabels( self, ixml ):
        if ixml.hasAttribute( 'label' ):
            self.pending_labels.extend( ixml.get( 'label' ).split() )

    def flatten( self, ixml ):
        name = ixml.getName()
        if name == 'seq':
            self.addPendingLabels( ixml )
            for i in ixml:
                self.flatten( i )
        elif name == 'noop':
            self.addPendingLabels( ixml )
        else:
            self.add( ixml )

    def __call__( self, *args, **kwargs ):
        '''
        Invokes the abstract method assemble and then returns the instruction list.
        This is the primary way of using these objects.
        '''
        self.flatten( *args, **kwargs )
        if self.pending_labels:
            print( 'WARNING: pending-labels were not resolved!', file=sys.stderr )
            self.add( MinXML( "return", label=' '.join( self.pending_labels ) ) )
        return self.instructions


class ResolveLabels:
    '''This callable pass assumes a flat structure (no sequences) and calculates the
    jump distances for labels.
    '''

    def __init__( self ):
        self.instructions = MinXML( "seq" )

    def resolve( self, ixml ):
        pass # GOT HERE

    def __call__( self, *args, **kwargs ):
        self.resolve( *args, **kwargs )

def backEnd( ixml ):
    ixml = PeepHole()( ixml )
    ixml = Flatten()( ixml )
    return ResolveLabels()( ixml )

