import os
import re
import glob
import subprocess

here = os.path.dirname( os.path.realpath( __file__ ) )
home = os.path.join( here, ".." )
executable = os.path.join( home, "apps/appginger/cpp/ginger-script")

def runtest( in_file_name ):
    out_file_name = re.sub( r'\.in.gnx$', '.out.txt', in_file_name )
    res = subprocess.getoutput( "cat {0} | {1}  -i -g gnx".format( in_file_name, executable ) )
    assert res == open( out_file_name ).read().strip()

def test_examples():
    for in_file_name in glob.glob( "appginger/*/*.in.gnx" ):
        yield runtest, in_file_name

def runproblem( in_file_name ):
    res = subprocess.getoutput( "cat {0} | {1} -i -g gnx".format( in_file_name, executable ) )
    #print "res = '" + res + "'"
    assert res.startswith( "\n### Mishap " )


def test_problems():
    for in_file_name in glob.glob( "appginger/*/*.fail.gnx" ):
        yield runproblem, in_file_name
