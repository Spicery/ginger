import unittest
import os
import commands

# import textwrap
# from tempfile import mkdtemp

here = os.path.dirname( os.path.realpath( __file__ ) )
home = os.path.join( here, ".." )
gnx2ginger = os.path.join( here, "gnxeval" )
executable = os.path.join( home, "apps/appginger/cpp/ginger-script" ) 

class GNX2GingerTestCase(unittest.TestCase):

    def setUp(self):
        self.infn = gnx2ginger  + "/%s.gnx"

    def runtest( self, fn, out ):
        res = commands.getoutput( """cat {0} | {1} -i --grammar=gnx""".format( self.infn % fn, executable ) )
        self.assertEqual( res, out )

    def test_gnx2ginger_print(self):
        """gnx2ginger - print"""
        self.runtest( "print", "Hello" )

    def test_gnx2ginger_nfib(self):
        """gnx2ginger - nfib"""
        self.runtest( "nfib", "18454929" )
 
    def test_gnx2ginger_factorial(self):
        """gnx2ginger - factorial"""
        self.runtest( "factorial", 6 )
