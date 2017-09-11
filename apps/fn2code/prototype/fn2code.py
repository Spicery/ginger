#!/usr/bin/env python3

from minxml import MinXML, readMinXML
import sys
import compiler
import backend

def enterInstruction( nargs ):
    if nargs == "0":
        return MinXML( "enter0" )
    elif nargs == "1":
        return MinXML( "enter1" )
    else:
        return MinXML( "enter" )

def transformFn( fn ):
    '''Applies the compiler to a <fn> element'''
    args = fn[0]
    body = fn[1]
    nargs = int( fn.get( "args.count" ) )
    nlocals = int( fn.get( "locals.count" ) )
    fn2 = MinXML( "fn.code", **fn.attributes )
    ( cbody, max_slots ) = compiler.compile( body, nargs, nlocals )
    fn2.children = [ enterInstruction( nlocals ) ]
    fn2.children.extend( backend.backEnd( cbody.getChildren() ) )
    fn2.put( "locals.count", str( max_slots ) )
    return fn2

def transform( gnx ):
    '''Finds <fn> elements to compile. Note that this function chews up the
    supplied data structure.'''
    if gnx.hasName( "fn" ):
        return transformFn( gnx )
    else:
        gnx.children = [ transform( i ) for i in gnx ]
        return gnx

def main():
    '''Reads in GNX expressions and then compile any fn elements'''
    while True:
        gnx = readMinXML( sys.stdin )
        if gnx == None:
            break
        if gnx.hasName( 'seq' ) and not( gnx ):
            print( gnx )
        else:
            print( "Received: ", file=sys.stderr )
            gnx.pretty( file=sys.stderr, indent=1 )
            gnx = transform( gnx )
            print( "Generated: ", file=sys.stderr )
            gnx.pretty( file=sys.stderr, indent=1 )
            print( gnx )
        sys.stdout.flush()  # Required to ensure proper switch of control between processes.

if __name__ == "__main__":
    main()
