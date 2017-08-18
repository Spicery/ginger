#!/usr/bin/python3

class Arity:

    def __init__( self, *arity_strings ):
        self._count = 0
        self._exact = True
        for a in arity_strings:
            self._exact = a and a[-1] == '+'
            if self._exact:
                self._count += int( a[:-1] )
            else:
                self._count += int( a )

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

    def add( self, other ):
        a = Arity()
        a._count = self._count + other._count
        a._exact = self._exact and other._exact
        return a


