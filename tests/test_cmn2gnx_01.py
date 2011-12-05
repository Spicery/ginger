import unittest
import os
import commands
# import textwrap
# from tempfile import mkdtemp

cmn2gnx = os.path.dirname(os.path.realpath( __file__ )) + "/cmn2gnx/"

class Common2GNXTestCase(unittest.TestCase):

    def setUp(self):
        self.infn = cmn2gnx + "%s.cmn"
        self.outfn = cmn2gnx  + "%s.gnx"

    def test_common2gnx_print_define(self):
        """common2gnx - print define"""
        fn = "print_define"
        res = commands.getoutput('cat %s | common2gnx' % (self.infn % fn))
        # print("Res %s" % res)
        self.assert_(res == open(self.outfn % fn).read())

    def test_common2gnx_print_fncall(self):
        """common2gnx - print fncall"""
        fn = 'print_fncall'
        res = commands.getoutput('cat %s | common2gnx' % (self.infn % fn))
        # print("Res %s" % res)
        self.assert_(res == open(self.outfn % fn).read())

    def test_common2gnx_nfib_define(self):
        """common2gnx - nfib define"""
        fn = "nfib_define"
        res = commands.getoutput('cat %s | common2gnx' % (self.infn % fn))
        # print("Res %s" % res)
        self.assert_(res == open(self.outfn % fn).read())

    def test_common2gnx_nfib_fncall(self):
        """common2gnx - nfib fncall"""
        fn = 'nfib_fncall'
        res = commands.getoutput('cat %s | common2gnx' % (self.infn % fn))
        # print("Res %s" % res)
        self.assert_(res == open(self.outfn % fn).read())

    def test_common2gnx_factorial_define(self):
        """common2gnx - factorial define"""
        fn = "factorial_define"
        res = commands.getoutput('cat %s | common2gnx' % (self.infn % fn))
        # print("Res %s" % res)
        self.assert_(res == open(self.outfn % fn).read())

    def test_common2gnx_factorial_fncall(self):
        """common2gnx - factorial fncall"""
        fn = 'factorial_fncall'
        res = commands.getoutput('cat %s | common2gnx' % (self.infn % fn))
        # print("Res %s" % res)
        self.assert_(res == open(self.outfn % fn).read())
