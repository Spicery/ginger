import unittest
import os
import commands
# import textwrap
# from tempfile import mkdtemp

gnx2ginger = os.path.dirname(os.path.realpath( __file__ )) + "/gnx2ginger/"

class GNX2GingerTestCase(unittest.TestCase):

    def setUp(self):
        self.infn = gnx2ginger  + "%s.gnx"

    def test_gnx2ginger_print(self):
        """gnx2ginger - print"""
        fn = "print"
        res = commands.getoutput("""echo %s | appginger""" % (self.infn % fn))
        print("Res %s" % res)
        self.assert_(res == None)

    def test_gnx2ginger_nfib(self):
        """gnx2ginger - nfib"""
        fn = "nfib"
        res = commands.getoutput("""echo '%s' | appginger""" % (self.infn % fn))
        print("Res %s" % res)
        self.assert_(res == None)

    def test_gnx2ginger_factorial(self):
        """gnx2ginger - factorial"""
        fn = "factorial"
        res = commands.getoutput("""echo '%s' | appginger""" % (self.infn % fn))
        print("Res %s" % res)
        self.assert_(res == None)

