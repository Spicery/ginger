#!/usr/bin/python

import subprocess
import json
import os


################################################################################
#   Arity
################################################################################

class Arity:
	
	def __init__( self, arity ):
		self.count = int( arity.replace( "+", "" ) )
		self.more = arity.find( "+" ) != -1
		
	def __str__( self ):
		m = str( self.more ).lower()
		return "Arity( {}, {} )".format( self.count, m )




################################################################################
#   generateMetaInfoTable
################################################################################

def generateMetaInfo():
	metainfo = json.loads( subprocess.check_output( [ "../../appginger/cpp/ginger-info", "-j" ] ) )
	stdinfo = metainfo[ "std" ]

	pw = open( "metainfo.cpp.inc", 'w' )
   	
   	for name in stdinfo:
   		e = stdinfo[ name ]
		pw.write(
			"MAPLET( \"{}\", MetaInfo( {}, {} ) ),\n".format(
				name, 
				Arity( e[ "in" ] ), 
				Arity( e[ "out" ] )
			)
		)
	pw.close()


generateMetaInfo()
