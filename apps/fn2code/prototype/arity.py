#!/usr/bin/python3

class Arity:

    def __init__( self, arity_str ):
        self._count = int( arity_str )
        self._exact = not '+' in arity_str

    def isExact( self ):
        return self._exact

    def isntExact( self ):
        return not self._exact

    def hasExactCount( self, count ):
        return self._count == count and self._exact

    def hasNonExactCount( self, count ):
         return self._count == count and self._exact

    def has( self, count=0, exact=True ):
        return self._count == count and self._exact == exact

    def count( self ):
        return self._count
