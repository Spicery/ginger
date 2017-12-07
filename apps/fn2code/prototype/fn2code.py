#!/usr/bin/env python3

from minxml import MinXML, readMinXML
import sys
import compiler
import backend
import problem

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
    fn2.children = backend.backEnd( [ enterInstruction( nlocals ), *cbody.getChildren() ] )
    fn2.put( "locals.count", str( max_slots ) )
    return fn2


def doCollectGlobals( expr, collection ):
    # print( 'EXPR', str(expr), expr.hasName( "var" ), file=sys.stderr )
    if expr.hasName( "var" ) and expr.hasAttributeValue( "scope", "global" ):
        d = MinXML( "var" )
        d.put( 'name', expr.get( 'name' ) )
        d.put( 'def.pkg', expr.get( 'def.pkg' ) )
        if expr.hasAttribute( 'protected' ):
            d.put( 'protected', expr.get( 'protected' ) )
        collection.append( d )
    else:
        for e in expr:
            doCollectGlobals( e, collection )

def collectGlobals( expr ):
    globals_collection = []
    doCollectGlobals( expr, globals_collection )
    # print( globals_collection, file=sys.stderr )
    return globals_collection
       

def transformTopLevel( top_level_expr ):
    '''Applies the compiler to a <fn> element'''
    body = top_level_expr[0]
    nargs = 0
    nlocals = 0
    kwargs = { 'args.count':"0", 'arity.eval':"1", 'locals.count':"0" }
    fn2 = MinXML( "fn.code", **kwargs )
    ( cbody, max_slots ) = compiler.compile( body, nargs, nlocals )
    fn2.children = backend.backEnd( [ enterInstruction( nlocals ), *cbody.getChildren() ] )
    fn2.put( "locals.count", str( max_slots ) )
    globals_collection = collectGlobals( top_level_expr )
    return MinXML( "unit", MinXML( "declarations", *globals_collection ), fn2 )


def transform( gnx ):
    '''Finds <fn> elements to compile. Note that this function chews up the
    supplied data structure.'''
    if gnx.hasName( "fn" ):
        return transformFn( gnx )
    elif gnx.hasName( "top.level" ):
        return transformTopLevel( gnx )
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
            # TODO: Not quite right.
            print( gnx )
        else:
            try:
                # print( "Received: ", file=sys.stderr )
                # gnx.pretty( file=sys.stderr, indent=1 )
                gnx = transform( gnx )
                # print( "Generated: ", file=sys.stderr )
                # gnx.pretty( file=sys.stderr, indent=1 )
                print( gnx )
            except problem.ProblemException as p:
                print( p.problem() )

        sys.stdout.flush()  # Required to ensure proper switch of control between processes.

if __name__ == "__main__":
    main()
