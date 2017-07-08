import os
import re
import glob
import subprocess

here = os.path.dirname( os.path.realpath( __file__ ) )
home = os.path.join( here, ".." )
executable = os.path.join( home, "apps/common2gnx/cpp/common2gnx")

def runtest( in_file_name ):
    out_file_name = re.sub( r'\.cmn$', '.gnx', in_file_name )
    res = subprocess.getoutput( "cat {0} | {1} -s".format( in_file_name, executable ) )
    assert res == open( out_file_name ).read().strip()

def test_examples():
    for in_file_name in glob.glob( "common2gnx/*.cmn" ):
        yield runtest, in_file_name

def runproblem( in_file_name ):
    res = subprocess.getoutput( "cat {0} | {1} -s".format( in_file_name, executable ) )
    assert res.startswith( "<problem " )

def test_problems():
    for in_file_name in glob.glob( "common2gnx/*.cmn.fail" ):
        yield runproblem, in_file_name
