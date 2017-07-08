import os
import re
import glob
import subprocess

here = os.path.dirname( os.path.realpath( __file__ ) )
home = os.path.join( here, ".." )
cmn2gnx = os.path.join( here, "simplifygnx" )
executable = os.path.join( home, "apps/simplifygnx/cpp/simplifygnx")

def runtest( in_file_name ):
    out_file_name = re.sub( r'\.in.gnx$', '.out.gnx', in_file_name )
    res = subprocess.getoutput( "cat {0} | {1} -suA".format( in_file_name, executable ) )
    assert res == open( out_file_name ).read().strip()

def test_examples():
    for in_file_name in glob.glob( "simplifygnx/*.in.gnx" ):
        yield runtest, in_file_name

def runproblem( in_file_name ):
    res = subprocess.getoutput( "cat {0} | {1} -suA".format( in_file_name, executable ) )
    assert res.startswith( "<problem " )

def test_problems():
    for in_file_name in glob.glob( "simplifygnx/*.fail.gnx" ):
        yield runproblem, in_file_name
