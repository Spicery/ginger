import subprocess
import json
import os
import math
import struct

################################################################################
#	Libraries
################################################################################

def stdLibProjectDir():
	"""Returns the standard library public.auto directory"""
	return "standard_library/ginger.library/public.auto"

def stdLibConstantsDir():
	"""Returns the standard library definitions for constants"""
	return "standard_library/ginger.constants/public.auto"

def gingerInteractiveDir():
	"""Returns the directory for the ginger.interactive package"""
	return "standard_library/ginger.interactive"



################################################################################
#   generateGingerLibrary
################################################################################


def encodeName( name ):
	"""
	 Uses URL encoding but does so quite aggressively as it will be used as a file name.
	 File names get substituted in Unix command line scripts, so can be used for injection
	 attacks. This is a form of neutralisation.
	 	@param s String to encode
	 	@return URL encoded String
	"""
	list = []
	for c in name:
		n = ord( c )
		if ( n < 256 and ( c.isdigit() or c.isalpha() or c == '-' or c == '_' ) ):
			list.append( c )
		else:
			list.append( '%' )
			list.append( hex( n )[2:] )
	return "".join( list )

def escapeName( name ):
	"""
	This escapes a name for inclusion in an XML (MinX) document.
	"""
	list = []
	for c in name:
		n = ord( c )
		if ( n < 256 and ( c.isdigit() or c.isalpha() or c == '-' or c == '_' ) ):
			list.append( c )
		else:
			list.append( "&#x" )
			list.append( hex( n )[2:] )
			list.append( ";" )
	return "".join( list )

def createDefinitionAbsFile( ename, varname, sysname ):
	escvarname = escapeName( varname )
	escsysname = escapeName( sysname )
	df = open( ename, 'w' )
	df.write( '<bind><var name="{0}"/><constant type="sysfn" value="{1}"/></bind>\n'.format( escvarname, escsysname ) )
	df.close()

def makeDefinitionFile( dir, alt_name, base_name ):
	ename = os.path.join( dir, encodeName( alt_name ) + ".gnx" )
	createDefinitionAbsFile( ename, alt_name, base_name )

def generateGingerLibrary( stdinfo, synonyms ):
	"""Generates a definition file for all the sysfns (built-in functions)"""
	dir = stdLibProjectDir()
	if not os.path.exists( dir ):
		os.makedirs( dir )
	for k in stdinfo:
		makeDefinitionFile( dir, k, k )
	for sn in synonyms:
		makeDefinitionFile( dir, sn[ "alt.name" ], sn[ "base.name" ] )

################################################################################
#   generateGingerLibraryExtras
################################################################################

class ConstantPackage:

	def __init__( self ):
		self.dir = stdLibConstantsDir()
		if not os.path.exists( self.dir ):
			os.makedirs( self.dir )	

	def createConstantFile( self, nam, typ, val ):
		encname = encodeName( nam )
		ename = os.path.join( self.dir, encname + ".gnx" )
		escnam = escapeName( nam )
		esctyp = escapeName( typ )
		escval = escapeName( str( val ) )
		df = open( ename, 'w' )
		df.write( '<bind><var name="{0}"/><constant type="{1}" value="{2}"/></bind>\n'.format( escnam, esctyp, escval ) )
		df.close()	

	def createConstantClassFile( self, nam ):
		self.createConstantFile( nam, "sysclass", nam )

def generateGingerLibraryExtras( stdinfo ):
	pkg = ConstantPackage()
	pkg.createConstantFile( "PI", "double", math.pi )
	pkg.createConstantFile( "E", "double", math.e )
	# TODO: Really we should move the calcsize( "P" ) out into a shared script.
	pkg.createConstantFile( "mostNegativeSmall", "int", -( 1 << struct.calcsize( "P" ) * 8 - 2 ) )
	pkg.createConstantFile( "mostPositiveSmall", "int",  ( 1 << struct.calcsize( "P" ) * 8 - 2 ) - 1 )
	pkg.createConstantClassFile( "Absent" )
	pkg.createConstantClassFile( "Bool" )
	pkg.createConstantClassFile( "Small" )
	pkg.createConstantClassFile( "Double" )
	pkg.createConstantClassFile( "String" )
	pkg.createConstantClassFile( "Char" )
	pkg.createConstantClassFile( "Nil" )
	pkg.createConstantClassFile( "Pair" )
	pkg.createConstantClassFile( "Vector" )
	pkg.createConstantClassFile( "Class" )


################################################################################
#   generateGingerInteractive
################################################################################

def generateGingerInteractive():
	dir = gingerInteractiveDir()
	if not os.path.exists( dir ):
		os.makedirs( dir )
	f = open( os.path.join( dir, "imports.gnx" ), 'w' )
	f.write( "<package>" );
	f.write( "<import from=\"ginger.library\" match0=\"public\" />" )
	f.write( "<import from=\"ginger.constants\" match0=\"public\" />" )
	f.write( "</package>\n" )
	f.close()

################################################################################
#   Main
################################################################################


def standardLibraryFiles():
	metainfo = json.loads( subprocess.check_output( [ "../apps/appginger/cpp/ginger-info", "-j" ] ) )
	stdinfo = metainfo[ "std" ]
	synonyms = metainfo[ "synonyms" ]
	generateGingerLibrary( stdinfo, synonyms )
	generateGingerLibraryExtras( stdinfo )
	generateGingerInteractive()

standardLibraryFiles()