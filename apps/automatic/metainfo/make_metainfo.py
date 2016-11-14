#!/usr/bin/python3

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

def writeMaplet( pw, alt_name, base_name, e ):
	pw.write(
		"MAPLET( \"{}\", MetaInfo( \"{}\", {}, {} ) ),\n".format(
			alt_name, 
			base_name,
			Arity( e[ "in" ] ), 
			Arity( e[ "out" ] )
		)
	)

def generateMetaInfo():
	metainfo = json.loads( subprocess.check_output( [ "../../appginger/cpp/ginger-info", "-j" ] ) )
	stdinfo = metainfo[ "std" ]
	synonyms = metainfo[ "synonyms" ]

	pw = open( "metainfo.cpp.inc", 'w' )
   	
   	for name in stdinfo:
   		e = stdinfo[ name ]
   		writeMaplet( pw, name, name, e )

	for sn in synonyms:
		alt_name = sn[ "alt.name" ]
		base_name = sn[ "base.name" ]
		e = stdinfo[ base_name ]
		writeMaplet( pw, alt_name, base_name, e )

	pw.close()


generateMetaInfo()
