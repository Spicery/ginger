#!/usr/bin/python3
# Generator for:
#   codegen.hpp.auto - a list of C++ method prototypes to be included in the CodeGenClass.
#   codegen.cpp.auto - a balanced binary chop that dispatches the CodeGenClass::method that 
#       implements an instruction.
#

#
# This is a list of instructions understood by the runtime system.
# It is a mapping from the name of the instruction to a list of
# required fields.
#
INSTRUCTIONS = {
    'seq':                      [],
    'enter':                    [],
    'enter0':                   [],
    'enter1':                   [],
    'return':                   [],
    "return.ifso":              [],
    "return.ifnot":             [],
    
    'pushq':                    [],
    'pushq.pop.local':          [ 'local' ],
    'pushq.ret':                [],

    'pop.local':                [ 'local' ],
    'push.local':               [ 'local' ],
    'push.local0':              [],
    'push.local1':              [],

    'push.local.ret':           [ 'local' ],
    'push.local0.ret':          [],
    'push.local1.ret':          [],
    
    "push.global":              [],
    'pop.global':               [],
    'end.call.global':          [ 'local' ],
    "set.count.call.global":    [ 'count' ],
    "set.count.call.local":     [ 'local', 'count' ],

    "calls":                    [],
    "set.count.calls":          [ 'count' ],
    "self.call":                [],
    "self.constant":            [],
    "self.call.n":              [ 'count' ],
    "end1.calls":               [ 'local' ],

    "and":                      [ 'to' ],
    "or":                       [ 'to' ],
    "absand":                   [ 'to' ],
    "absor":                    [ 'to' ],

    "add":                      [],
    "mul":                      [],
    "sub":                      [],
    "div":                      [],
    "quo":                      [],
    "neg":                      [],
    "pos":                      [],
    "not":                      [],
    "mod":                      [],

    "lt":                       [],
    "lte":                      [],
    "gt":                       [],
    "gte":                      [],
    "eq":                       [],
    "neq":                      [],

    "lte.ss":                   [ 'local0', 'local1', 'to' ],
    "neq.si":                   [ 'local', 'to' ],
    "eq.si":                    [ 'local', 'to' ],
    "neq.ss":                   [ 'local0', 'local1', 'to' ],
    "eq.ss":                    [ 'local0', 'local1', 'to' ],


    "dup":                      [],
    "erase":                    [],
    "erase.num":                [ 'n' ],
    "decr":                     [],
    "incr":                     [],
    "incr.by":                  [ 'by' ],

    "incr.local.by":            [ 'local', 'by' ],
    "incr.local.by1":           [ 'local' ],

    "start.mark":               [ 'local' ],
    "end.mark":                 [ 'local' ],
    "set.count.mark":           [ 'local' ],
    "erase.mark":               [ 'local' ],
    "check.mark.gte":           [ 'local', 'count' ],
    "check.mark":               [ 'local', 'count' ],
    "check.mark0":              [ 'local' ],
    "check.mark1":              [ 'local' ],
    "check.count":              [ 'count' ],

    "goto":                     [ 'to' ],
    "bypass":                   [ 'to' ],
    "ifnot":                    [ 'to' ],
    "ifso":                     [ 'to' ],
    "ifso.local":               [ 'local', 'to' ],
    "ifnot.local":              [ 'local', 'to' ],

    "syscall":                  [ 'name' ],
    "sysreturn":                [],
    "set.count.syscall":        [ 'name', 'count' ],

    "escape":                   [],
    "fail":                     [],

    "getiterator":              [],

    "field":                    [ 'n' ],

}

def cppName( codename ):
    return codename.replace( '.', '_' ).upper()

def generateInstructionsList():
    print( 'namespace Ginger {' );
    for codename, keywords in INSTRUCTIONS.items():
        print( '    static const char * VM_{} = "{}";'.format( cppName( codename ), codename ) )
        for kw in keywords:
            print( '    static const char * VM_{}_{} = "{}";'.format( cppName( codename ), kw.upper(), kw ) )
    print( '}' )

def labelName( codename ):
    return 'label_' + codename.replace( '.', '_' )

def baseName( codename ):
    return codename.replace( '.', '_' )


TAB = '  '

def generateSwitch( codenames, depth ):
    # print( 'CODENAMES', codenames )
    n = len( codenames )
    indent = TAB * depth
    if n == 0:
        raise Exception( 'Internal error' )
    elif n <= 2:
        for name in codenames:
            print( indent, 'if ( name == "{}" ) goto {};'.format( name, labelName( name ) ) )
        print( indent, 'goto fail;' )
    else:
        n2 = n // 2
        name = codenames[ n2 ]
        print( indent, 'tmp = name.compare( "{}" )'.format( name ) )
        print( indent, 'if ( tmp < 0 ) {' )
        # print( 'SPLIT LEFT', n, codenames[0:n2] )
        generateSwitch( codenames[0:n2], depth + 1 )
        print( indent, '} else if ( tmp > 0 ) {' )
        # print( 'SPLIT RIGHT', n, codenames[n2+1:] )
        generateSwitch( codenames[n2+1:], depth + 1 )
        print( indent, '} else {' )
        print( TAB * ( depth + 1 ), 'goto {};'.format( labelName( name ) ) )
        print( indent, '}' )

def generateSwitchToGotos():
    generateSwitch( sorted( INSTRUCTIONS.keys() ), 0 )

def generatePrototypes( fileobj ):
    for codename in INSTRUCTIONS.keys():
        print( 'void plant_{}( Gnx instruction );'.format( baseName( codename ) ), file=fileobj )

def tmpNames():
    n = 0
    while True:
        n += 1
        yield 'tmp{}'.format( n )


class GenSwitchToMethods:

    def __init__( self, fileobj ):
        self._tmpcounter = tmpNames()
        self._fileobj = fileobj

    def generate( self, codenames, depth ):
        # print( 'CODENAMES', codenames )
        n = len( codenames )
        indent = TAB * depth
        if n == 0:
            raise Exception( 'Internal error' )
        elif n <= 2:
            for name in codenames:
                print( indent, 'if ( name == "{}" ) {{ this->plant_{}( instruction ); return; }}'.format( name, baseName( name ) ), file=self._fileobj )
        else:
            n2 = n // 2
            name = codenames[ n2 ]
            tmp = self._tmpcounter.__next__()
            print( indent, 'int {} = name.compare( "{}" );'.format( tmp, name ), file=self._fileobj )
            print( indent, 'if ( {} < 0 ) {{'.format( tmp ), file=self._fileobj )
            # print( 'SPLIT LEFT', n, codenames[0:n2] )
            self.generate( codenames[0:n2], depth + 1 )
            print( indent, '}} else if ( {} > 0 ) {{'.format( tmp ), file=self._fileobj )
            # print( 'SPLIT RIGHT', n, codenames[n2+1:] )
            self.generate( codenames[n2+1:], depth + 1 )
            print( indent, '} else {', file=self._fileobj )
            print( TAB * ( depth + 1 ), 'this->plant_{}( instruction ); return;'.format( baseName( name ) ), file=self._fileobj )
            print( indent, '}', file=self._fileobj )

def generateSwitchToMethods( fileobj ):
    GenSwitchToMethods( fileobj ).generate( sorted( INSTRUCTIONS.keys() ), 0 )



if __name__ == "__main__":
    with open( 'codegen.hpp.auto', 'w' ) as f:
        generatePrototypes( f )
    with open( 'codegen.cpp.auto', 'w' ) as f:
        generateSwitchToMethods( f )