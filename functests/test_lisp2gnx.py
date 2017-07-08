import os
import re
import glob
import subprocess

here = os.path.dirname( os.path.realpath( __file__ ) )
home = os.path.join( here, ".." )
cmn2gnx = os.path.join( here, "lisp2gnx" )
executable = os.path.join( home, "apps/lisp2gnx/lsp/lisp2gnx")

def runtest( in_file_name ):
    out_file_name = re.sub( r'\.lisp$', '.gnx', in_file_name )
    res = subprocess.getoutput( "cat {0} | {1}".format( in_file_name, executable ) )
    assert res == open( out_file_name ).read().strip()

def test_examples():
    for in_file_name in glob.glob( "lisp2gnx/*.lisp" ):
        yield runtest, in_file_name

def runproblem( in_file_name ):
    res = subprocess.getoutput( "cat {0} | {1}".format( in_file_name, executable ) )
    assert res.startswith( "<problem " )

def test_problems():
    for in_file_name in glob.glob( "common2gnx/*.lisp.fail" ):
        yield runproblem, in_file_name
