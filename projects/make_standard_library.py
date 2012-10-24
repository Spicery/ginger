import subprocess
import json
import os

################################################################################
#   generateGingerLibrary
################################################################################


def stdLibProjectDir():
	"""Returns the standard library public.auto directory"""
	return "standard_library/ginger.library/public.auto"

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

def createDefinitionFile( ename, name ):
	escname = escapeName( name )
	df = open( ename, 'w' )
	df.write( '<bind><var name="{0}"/><sysfn value="{0}"/></bind>\n'.format( escname ) )
	df.close()


def generateGingerLibrary( stdinfo ):
	"""Generates a definition file for all the sysfns (built-in functions)"""
	dir = stdLibProjectDir()
	if not os.path.exists( dir ):
		os.makedirs( dir )
	for k in stdinfo:
		ename = os.path.join( dir, encodeName( k  ) + ".gnx" )
		createDefinitionFile( ename, k )

################################################################################
#   generateGingerInteractive
################################################################################

def gingerInteractiveDir():
	"""Returns the directory for the ginger.interactive package"""
	return "standard_library/ginger.interactive"

def generateGingerInteractive():
	dir = gingerInteractiveDir()
	if not os.path.exists( dir ):
		os.makedirs( dir )
	f = open( os.path.join( dir, "imports.gnx" ), 'w' )
	f.write( "<package><import from=\"ginger.library\" match0=\"public\" /></package>\n" )
	f.close()

################################################################################
#   Main
################################################################################


def standardLibraryFiles():
	metainfo = json.loads( subprocess.check_output( [ "../apps/appginger/cpp/ginger-info", "-j" ] ) )
	stdinfo = metainfo[ "std" ]
	generateGingerLibrary( stdinfo )
	generateGingerInteractive()

standardLibraryFiles()