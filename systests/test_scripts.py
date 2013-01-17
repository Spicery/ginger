import os
import re
import glob
import commands
import nose.tools

def runtest( in_file_name ):
    out_file_name = re.sub( r'\.in.sh$', '.out.txt', in_file_name )
    res = commands.getoutput( "{0}".format( in_file_name ) )
    nose.tools.assert_equal( res.strip(), open( out_file_name ).read().strip() )

def test_examples():
    for in_file_name in glob.glob( "scripts/*.in.sh" ):
        yield runtest, in_file_name

def runproblem( in_file_name ):
    res = commands.getoutput( "{0}".format( command, flags, in_file_name ) )
    nose.tools.assert_true( res.startswith( "<problem " ) )


def test_problems():
    for in_file_name in glob.glob( "scripts/*.fail.sh" ):
        yield runproblem, in_file_name
