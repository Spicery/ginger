import os
import re
import glob
import commands
import nose.tools

here = os.path.dirname( os.path.realpath( __file__ ) )
home = os.path.join( here, ".." )
executable = os.path.join( home, "apps/common2gnx/cpp/cstyle2gnx")

def trimMnxString( m ):
	m = re.sub( r'\>\s+\<', '><', m )
	return re.sub( r'\s\s+', ' ', m )

def runtest( in_file_name ):
    out_file_name = re.sub( r'\.in\.cst$', r'.out.gnx', in_file_name )
    res = commands.getoutput( "cat {0} | {1} -s".format( in_file_name, executable ) )
    nose.tools.assert_equal( trimMnxString( res ), trimMnxString( open( out_file_name ).read().strip() ) )

def test_examples():
    for in_file_name in glob.glob( "cstyle2gnx/*.in.cst" ):
        yield runtest, in_file_name

def runproblem( in_file_name ):
    res = commands.getoutput( "cat {0} | {1} -s".format( in_file_name, executable ) )
    nose.tools.assert_true( res.startswith( "<problem " ) )


def test_problems():
    for in_file_name in glob.glob( "cstyle2gnx/*.fail.cst" ):
        yield runproblem, in_file_name
