#!/usr/bin/env python3

from minxml import MinXML, readMinXML
import sys
import compiler
import backend

def transformFn( fn ):
    args = fn[0]
    body = fn[1]
    fn2 = MinXML( "fn2code", **fn.attributes )
    fn2.children = backend.backEnd( compiler.compile( body ).children )
    return fn2


def transform( gnx ):
    if gnx.hasName( "fn" ):
        return transformFn( gnx )
    else:
        gnx.children = [ transform( i ) for i in gnx ]
        return gnx

def main():
    while True:
        gnx = readMinXML( sys.stdin )
        if gnx == None:
            break
        gnx = transform( gnx )
        print( gnx )
        sys.stdout.flush()  # Required to ensure proper switch of control between processes.

if __name__ == "__main__":
    main()
