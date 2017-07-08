#!/usr/bin/python3

import sys
import os
import glob
import re

line_a = "/*****************************************************************************\\\n"
line_b = "\\*****************************************************************************/\n"

def code_header():
	print()
	print( ".. code-block:: c++" )
	print()

def code_footer():
	print()

def extractDocs( file ):
	state = 0
	for line in file:
		#print( "> ", line )
		if line == line_a:
			state = 1
		elif line == line_b:
			state = 2
			code_header()
		elif state == 1:
			if line[ 0 ].isalpha():
				print( "**{}**".format( line.rstrip() ) )
			else:
				print( line, end='' )
		elif state == 2:
			print( "    ", line, end='' )
	if state == 2:
		code_footer()

def file_header( fname ):
	print( ".. Start of file", fname )
	print()
	m = re.match( "([^.]*)", fname )
	title = "Instruction {}".format( m.group( 1 ) )
	print( title )
	print( "-" * len( title ) )
	print()

def file_footer( fname ):
	print( ".. End of file", fname )
	print()

def doc_header():
	print( "======================" )
	print( "Ginger Instruction Set" )
	print( "======================" )
	print()
	with open( "README.rst" ) as intro:
		print( intro.read() )
	print()

def doc_footer():
	print( ".. End of document" )

def processAllFiles():
	doc_header()
	for f in glob.glob( "*.i*" ):
		file_header( f )
		extractDocs( open( f ) )
		file_footer( f )
	doc_footer()

os.chdir( sys.argv[1] )
processAllFiles()
