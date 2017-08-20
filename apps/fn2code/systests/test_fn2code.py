import os
import re
import glob
import subprocess
import sys

def runtest( in_file_name ):
    out_file_name = re.sub( r'\.in.sh$', '.out.txt', in_file_name )
    p = subprocess.Popen( "{0}".format( in_file_name ), shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE )
    ( out, err ) = p.communicate()
    actual = out.decode( 'UTF-8' ).strip()
    with open( out_file_name, 'r' ) as ex:
    	expected = ex.read().strip()
    if actual != expected:
	    print( 'ACTUAL   : ', actual, file=sys.stderr )
	    print( 'EXPECTED : ', expected, file=sys.stderr )
    assert actual == expected

def test_examples():
    for in_file_name in glob.glob( "scripts/*.in.sh" ):
        yield runtest, in_file_name
