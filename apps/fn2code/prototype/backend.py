from minxml import MinXML
import sys
import sqlite3
from pathlib import Path
from label import Label

################################################################################
# system.db
################################################################################

SYSTEM_DB = Path( __file__ ).parent.joinpath( 'system.db' )

if not SYSTEM_DB.exists():
    raise Exception( 'The system.db file is missing (please rebuild $GINGER_DEV_HOME/apps/appginger/cpp' )


################################################################################
# Sysfn_details provides a mapping from system functions to vm-instructions.
################################################################################

DETAILS = {}

with sqlite3.connect( SYSTEM_DB.as_posix() ) as conn:
    conn.row_factory = sqlite3.Row
    for row in conn.execute( "SELECT name, op FROM sysfn_detail WHERE OP <> ''" ):
        DETAILS[ row[ 'name' ] ] = row[ 'op' ]


################################################################################
# Set of peephole transformations. Note that the peephole transformations
# are free to ignore the issue of labels. The function that applies them
# will capture any existing label and re-apply those labels (in addition to
# any added by the optimisation).
################################################################################

PEEPHOLE = {}

def PeepHole( instruction_name ):
    def peephole_decorator( func ):
        PEEPHOLE[ instruction_name ] = func
        return func
    return peephole_decorator

@PeepHole( "syscall" )
def optSysCall( ixml ):
    name = ixml.getAttribute( "name" )
    if name in DETAILS:
        return MinXML( DETAILS[ name] )
    else:
        return ixml

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
        return MinXML( "seq" )
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
        return MinXML( "seq" )
    elif d == "1":
        return MinXML( "incr.local.by1", local=ixml.get( "local" ) )
    else:
        return ixml

@PeepHole( "erase.num" )
def optEraseNum( ixml ):
    n = ixml.get( "n" )
    if n == '0':
        return MinXML( "seq" )
    elif n == '1':
        return MinXML( "erase" )
    else:
        return ixml       

class PeepHole:

    def __init__( self, initial=None ):
        self.instructions = [] if initial is None else initial

    def add( self, ixml ):
        self.instructions.append( ixml )

    def assemble( self, ixml ):
        name = ixml.getName()
        if name == 'seq':
            if ixml.hasAttributes():
                subtask = PeepHole( initial=ixml.copy() )
                for i in ixml:
                    subtask.assemble( i )
                self.add( subtask.instructions )
            else:
                for i in ixml:
                    self.assemble( i )
        elif name in PEEPHOLE:
            saved_label = ixml.get( 'label', otherwise='' )
            ixml = PEEPHOLE[ name ]( ixml )
            if saved_label:
                # Extend the labels on ixml by saved_label.
                labs = saved_label.split()
                labs.extend( ixml.get( 'label', otherwise='' ).split() )
                ixml.put( 'label', ' '.join( labs ) )
            self.assemble( ixml )
        else:
            self.add( ixml )

    def assembleList( self, ixmllist ):
        for ixml in ixmllist:
            self.assemble( ixml )

    def __call__( self, *args, **kwargs ):
        '''
        Invokes the abstract method assemble and then returns the instruction list.
        This is the primary way of using these objects.
        '''
        self.assembleList( *args, **kwargs )
        return self.instructions

class Flatten:
    '''This callable flattens out all the instructions.
    '''

    def __init__( self ):
        self.instructions = []
        self.pending_labels = []

    def _usePendingLabels( self, ixml ):
        '''Modifies the instruction in place - but supplied a copy'''
        labs = ixml.get( 'label', otherwise='' ).split()
        labs.extend( self.pending_labels )
        ' '.join( labs )
        ixml.put( 'label',  ' '.join( labs ) )
        self.pending_labels = []        

    def add( self, ixml ):
        if self.pending_labels:
            ixml = ixml.copy()
            self._usePendingLabels( ixml )
        self.instructions.append( ixml )

    def addPendingLabels( self, ixml ):
        if ixml.hasAttribute( 'label' ):
            self.pending_labels.extend( ixml.get( 'label' ).split() )

    def flatten( self, ixml ):
        name = ixml.getName()
        if name == 'seq':
            self.addPendingLabels( ixml )
            for i in ixml:
                self.flatten( i )
        else:
            self.add( ixml )

    def flattenList( self, ixmlList ):
        for ixml in ixmlList:
            self.flatten( ixml )

    def __call__( self, *args, **kwargs ):
        '''
        Invokes the abstract method assemble and then returns the instruction list.
        This is the primary way of using these objects.
        '''
        self.flattenList( *args, **kwargs )
        if self.pending_labels:
            print( 'WARNING: pending-labels were not resolved!', file=sys.stderr )
            self.add( MinXML( "return", label=' '.join( self.pending_labels ) ) )
        return self.instructions

class Widths:

    INSTANCE = None

    def __init__( self ):
        self.widths = None

    def populate( self ):
        with sqlite3.connect( SYSTEM_DB.as_posix() ) as conn:
            cursor = conn.execute( 'SELECT codename, width FROM instruction;' )
            self.widths = {}
            for row in cursor:
                self.widths[ row[0] ] = row[1]

    def widthOf( self, iname ):
        if self.widths is None:
            self.populate()
        return self.widths[ iname ]

    @staticmethod
    def width( iname ):
        if not Widths.INSTANCE:
            Widths.INSTANCE = Widths()
        return Widths.INSTANCE.widthOf( iname )

class ResolveLabels:
    '''This callable pass assumes a flat structure (no sequences) and calculates the
    jump distances for labels.
    '''

    def __init__( self ):
        self.instructions = []
        self.label_offsets = {}

    def _calcOffsets( self, seqixml ):
        sofar = 0
        for i in seqixml:
            w = Widths.width( i.getName() )
            i.put( "width", str( w ) )
            i.put( "offset", str( sofar ) )
            lab = i.get( 'label', None )
            if lab:
                self.label_offsets[ lab ] = sofar
            sofar += w

    def _resolveJumps( self, seqixml ):
        for ixml in seqixml:
            offset = int( ixml.get( 'offset' ) )
            lab = ixml.get( 'to.label', None )
            if lab and lab != Label.RETURN.id():
                d = self.label_offsets[ lab ] - offset
                ixml.put( 'to', str( d - 1 ) )

    def edit( self, seqixml ):
        self._calcOffsets( seqixml )
        self._resolveJumps( seqixml )
        return seqixml

    def __call__( self, *args, **kwargs ):
        return self.edit( *args, **kwargs )

def backEnd( xmllist ):
    '''Result may share store with the input but the input is not updated.
    seqixml is typically a seq of instructions but may be a single instruction.
    At the end it is guaranteed to be a seq element with instructions.
    '''
    xmllist = PeepHole()( xmllist )
    xmllist = Flatten()( xmllist )
    return ResolveLabels()( xmllist )

