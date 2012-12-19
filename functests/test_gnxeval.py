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

    def test_gnx2ginger_print(self):
        """gnx2ginger - print"""
        fn = "print"
        res = commands.getoutput( """cat {0} | {1} -i --grammar=gnx""".format( self.infn % fn, executable ) )
        # print("Res %s" % res)
        self.assertEqual( res, "Hello" )

    def test_gnx2ginger_nfib(self):
        """gnx2ginger - nfib"""
        fn = "nfib"
        res = commands.getoutput( """cat {0} | {1} -m3 -i --grammar=gnx""".format( self.infn % fn, executable ) )
       # print("Res: %s" % res)
        self.assertEqual(res, "18454929")

    def test_gnx2ginger_factorial(self):
        """gnx2ginger - factorial"""
        fn = "factorial"
        res = commands.getoutput( """cat {0} | {1} -i --grammar=gnx""".format( self.infn % fn, executable ) )
        # print("Res %s" % res)
        self.assertEqual( res, "6" )