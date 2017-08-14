#!/usr/bin/env python3

from minxml import MinXML, readMinXML
import sys
import compiler
import backend

def transformFn( fn ):
    '''Applies the compiler to a <fn> element'''
    args = fn[0]
    body = fn[1]
    fn2 = MinXML( "fn2code", **fn.attributes )
    fn2.children = backend.backEnd( compiler.compile( body ).children )
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
        # print( "Received: ", str( gnx ), file=sys.stderr )
        gnx = transform( gnx )
        # print( "Generated: ", str( gnx ), file=sys.stderr )
        print( gnx )
        sys.stdout.flush()  # Required to ensure proper switch of control between processes.

if __name__ == "__main__":
    main()
