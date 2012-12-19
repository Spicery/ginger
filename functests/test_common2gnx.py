import unittest
import os
import commands
# import textwrap
# from tempfile import mkdtemp

here = os.path.dirname( os.path.realpath( __file__ ) )
home = os.path.join( here, ".." )
cmn2gnx = os.path.join( here, "common2gnx" )
executable = os.path.join( home, "apps/common2gnx/cpp/common2gnx")

class Common2GNXTestCase(unittest.TestCase):

    def setUp(self):
        self.infn = cmn2gnx + "/%s.cmn"
        self.outfn = cmn2gnx  + "/%s.gnx"

    def test_common2gnx_print_define(self):
        """common2gnx - simple print define"""
        fn = "print_define"
        res = commands.getoutput( "cat {0} | {1} -s".format( (self.infn % fn), executable ) )
        # print("Res %s" % res)
        self.assertEqual(res, open(self.outfn % fn).read())

    def test_common2gnx_print_fncall(self):
        """common2gnx - print fncall"""
        fn = 'print_fncall'
        res = commands.getoutput( "cat {0} | {1} -s".format( (self.infn % fn), executable ) )
        # print("Res %s" % res)
        self.assertEqual(res, open(self.outfn % fn).read())

    def test_common2gnx_nfib_define(self):
        """common2gnx - nfib define"""
        fn = "nfib_define"
        res = commands.getoutput( "cat {0} | {1} -s".format( (self.infn % fn), executable ) )
        # print("Res %s" % res)
        self.assertEqual(res, open(self.outfn % fn).read())

    def test_common2gnx_nfib_fncall(self):
        """common2gnx - nfib fncall"""
        fn = 'nfib_fncall'
        res = commands.getoutput( "cat {0} | {1} -s".format( (self.infn % fn), executable ) )
        # print("Res %s" % res)
        self.assertEqual(res, open(self.outfn % fn).read())

    def test_common2gnx_factorial_define(self):
        """common2gnx - factorial define"""
        fn = "factorial_define"
        res = commands.getoutput( "cat {0} | {1} -s".format( (self.infn % fn), executable ) )
        # print("Res %s" % res)
        self.assertEqual(res, open(self.outfn % fn).read())

    def test_common2gnx_factorial_fncall(self):
        """common2gnx - factorial fncall"""
        fn = 'factorial_fncall'
        res = commands.getoutput( "cat {0} | {1} -s".format( (self.infn % fn), executable ) )
        # print("Res %s" % res)
        self.assertEqual(res, open(self.outfn % fn).read())

    def test_common2gnx_hello_define(self):
        """common2gnx - hello define - multiple definitions in source"""
        fn = "hello_define"
        res = commands.getoutput( "cat {0} | {1} -s".format( (self.infn % fn), executable ) )
        # print("Res %s" % res)
        self.assertEqual(res, open(self.outfn % fn).read())

    def test_common2gnx_hello_fncall(self):
        """common2gnx - hello fncall"""
        fn = 'hello_fncall'
        res = commands.getoutput( "cat {0} | {1} -s".format( (self.infn % fn), executable ) )
        # print("Res %s" % res)
        self.assert_(res == open(self.outfn % fn).read())
