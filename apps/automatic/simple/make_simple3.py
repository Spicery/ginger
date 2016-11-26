#!/usr/bin/python
################################################################################
#
#	This file automates the production of C++ code to support the
#	complex tagging scheme used for simple keys. Simple keys are 
#	simple values (i.e. non-references) that represent keys. They
#	are heavily used for built-in types. [Simple keys exist to avoid
#	populating the heap at startup with built-in key objects. They
#	are not strictly required and might be phased out eventually.]
#
#	It generates two C++ code snippet files
#		*	${GINGERDEV}/apps/automatic/simple/simplekey.cpp.auto
#		*	${GINGERDEV}/apps/automaticsimple/simplekey.hpp.auto
#	These snippets are included into 
#		*	${GINGERDEV}/apps/appginger/cpp/key.cpp and 
#		*	${GINGERDEV}/apps/automatic/hpp/key.hpp
#	respectively.
#
################################################################################

import os
import struct 	# For calculating the size of doubles.

################################################################################
#	Utility functions
################################################################################

def counter():
	"""A simple helper routine for allocating serial numbers."""
	n = 0
	while True:
		yield n
		n += 1

def counterTo( mx ):
	"""A simple helper routine for allocating serial numbers upto a max."""
	n = 0
	while n < mx:
		yield n
		n += 1
	raise CountLimitExceeded


################################################################################
#	The Layout class specifies a 3-bit field that describes the store
#	layout of an object. Typically this is all the information the 
#	garbage collector requires to relocate an object.
################################################################################

class Layout:

	_count = counter()
	_values = []

	def __init__( self, name ):
		self._name = name
		self._ordinal = Layout._count.__next__()
		Layout._values.append( self )

	def name( self ):
		return self._name

	def ordinal( self ):
		return self._ordinal

	@staticmethod
	def values():
		for i in Layout._values:
			yield i	

Layout.KEYLESS_LAYOUT 	= Layout( "KEYLESS_LAYOUT" )
Layout.RECORD_LAYOUT	= Layout( "RECORD_LAYOUT" )
Layout.VECTOR_LAYOUT	= Layout( "VECTOR_LAYOUT" )
Layout.STRING_LAYOUT	= Layout( "STRING_LAYOUT" )
Layout.MIXED_LAYOUT		= Layout( "MIXED_LAYOUT" )
Layout.WRECORD_LAYOUT	= Layout( "WRECORD_LAYOUT" )
Layout.UNUSED6_LAYOUT	= Layout( "UNUSED6_LAYOUT" )
Layout.UNUSED7_LAYOUT	= Layout( "UNUSED7_LAYOUT" )


################################################################################
#	The Kind class denotes a 4-bit refinement of a layout that provides
#	enough type information for general low-level operators, such as 
#	explode ('...'). [Kinds represent built-in essential classes.]
################################################################################

class Kind:
	LAYOUT_WIDTH = 3
	_values = []
	#_next = counter()

	def __init__( self, name, layout, sublayout ):
		#print "Kind( ", name, layout.ordinal(), sublayout, ")"
		self._name = name
		#self._ordinal = Kind._next.next()
		self._layout = layout
		self._sublayout = sublayout
		Kind._values.append( self )

	#def ordinal( self ):
	#	return self._ordinal

	def kind( self ):
		return ( self._sublayout << Kind.LAYOUT_WIDTH ) | self._layout.ordinal() 

	def layout( self ):
		return self._layout

	def name( self ):
		return self._name

	def sublayout( self ):
		return self._sublayout

	@staticmethod
	def values():
		for i in Kind._values:
			yield i	

Kind.SUBLAYOUT_WIDTH 		= 3
Kind.ATOMIC_SUBLAYOUT       = ( 1 << Kind.SUBLAYOUT_WIDTH ) - 1

Kind.KEYLESS_SUBLAYOUT 		= counterTo( Kind.ATOMIC_SUBLAYOUT )
Kind.RECORD_SUBLAYOUT 		= counterTo( Kind.ATOMIC_SUBLAYOUT )
Kind.VECTOR_SUBLAYOUT 		= counterTo( Kind.ATOMIC_SUBLAYOUT )
Kind.STRING_SUBLAYOUT 		= counterTo( Kind.ATOMIC_SUBLAYOUT )
Kind.MIXED_SUBLAYOUT 		= counterTo( Kind.ATOMIC_SUBLAYOUT )
Kind.WRECORD_SUBLAYOUT 		= counterTo( Kind.ATOMIC_SUBLAYOUT )

Kind.KEYLESS_KIND 	= Kind( "KEYLESS", Layout.KEYLESS_LAYOUT, Kind.KEYLESS_SUBLAYOUT.__next__() )
Kind.RECORD_KIND	= Kind( "RECORD", Layout.RECORD_LAYOUT, Kind.RECORD_SUBLAYOUT.__next__() )
Kind.PAIR_KIND 		= Kind( "PAIR", Layout.RECORD_LAYOUT, Kind.RECORD_SUBLAYOUT.__next__() )
Kind.MAP_KIND 		= Kind( "MAP", Layout.RECORD_LAYOUT, Kind.RECORD_SUBLAYOUT.__next__() )
Kind.VECTOR_KIND 	= Kind( "VECTOR", Layout.VECTOR_LAYOUT, Kind.VECTOR_SUBLAYOUT.__next__() )
Kind.STRING_KIND 	= Kind( "STRING", Layout.STRING_LAYOUT, Kind.STRING_SUBLAYOUT.__next__() )
Kind.ATTR_KIND 		= Kind( "ATTR", Layout.MIXED_LAYOUT, Kind.MIXED_SUBLAYOUT.__next__() )
#print "ATTR KIND", Kind.ATTR_KIND.kind()
Kind.MIXED_KIND 	= Kind( "MIXED", Layout.MIXED_LAYOUT, Kind.MIXED_SUBLAYOUT.__next__() )	
#print "MIXED KIND", Kind.MIXED_KIND.kind()
Kind.WRECORD_KIND 	= Kind( "WRECORD", Layout.WRECORD_LAYOUT, Kind.WRECORD_SUBLAYOUT.__next__() )	
Kind.EXTERNAL_KIND 	= Kind( "EXTERNAL", Layout.RECORD_LAYOUT, Kind.RECORD_SUBLAYOUT.__next__() )	
Kind.ATOMIC_WRECORD_KIND = Kind( "ATOMIC_WRECORD", Layout.WRECORD_LAYOUT, Kind.ATOMIC_SUBLAYOUT )

################################################################################
#	The KeyData class is used to represent all the information of a simple
#	key. This includes:
#		*	name: the print-name of the key. 
#		*	nfields: for record/mixed objects, how many fields it has.
#		*	kind: the essential class of the simple key.
################################################################################

class KeyData:

	def __init__( self, name, ordinal, nfields, kind ):
		self._name = name
		self._nfields = nfields
		self._kind = kind
		self._ordinal = ordinal

	def name( self ):
		return self._name

	def nfields( self ):
		return self._nfields

	def ordinal( self ):
		return self._ordinal

	def kind( self ):
		return self._kind

# 	ATTENTION!
#	Calculating how large a Double object needs to be is a non-trivial
#	exercise. In particular it depends on whether or not the -m32 or
#	-m64 flags are supplied to the g++ compiler. The following is a good
#	first draft. (However it does mean that we should really amend the
#	build script so that there's an initial 32/64 bit selection
#	phase i.e. make all32 and make all64.
wordsPerDouble = ( struct.calcsize( "d" ) + struct.calcsize( "P" ) - 1 ) // struct.calcsize( "P" )

keys = [
	KeyData( "Absent", 			0, 	0, 	Kind.KEYLESS_KIND 	),
	KeyData( "Bool", 			1, 	0, 	Kind.KEYLESS_KIND	),
	KeyData( "Class",			2, 	4, 	Kind.RECORD_KIND	),
	KeyData( "Nil", 			3, 	0, 	Kind.KEYLESS_KIND	),
	KeyData( "Pair", 			4, 	2, 	Kind.PAIR_KIND		),
	KeyData( "Small", 			5, 	0, 	Kind.KEYLESS_KIND	),
	KeyData( "String", 			6, 	0, 	Kind.STRING_KIND	),
	KeyData( "Symbol", 			7, 	1, 	Kind.KEYLESS_KIND	),

	#	Do NOT move references without changing IsVectorID and
	#	IsUpdateableVectorID. Must be on a multiple of 2 boundary.
	# 	Also reserving 10 & 11 to make way for Updateable Vector.
	KeyData( "Vector",			8, 	0, 	Kind.VECTOR_KIND	),
	KeyData( "UpdateableVector",   9,  0,  Kind.VECTOR_KIND    ),

	#KeyData( "Unicode", 		10, 0, 	Kind.KEYLESS_KIND	),	# Free, making room for UpdateableVector.
	KeyData( "Termin",			11, 0, 	Kind.KEYLESS_KIND	),
	KeyData( "Char", 			12, 0, 	Kind.KEYLESS_KIND	),
	KeyData( "Maplet",  		13, 2, 	Kind.RECORD_KIND	),
	KeyData( "Indirection", 	14, 1, 	Kind.RECORD_KIND	),
	KeyData( "Assoc", 			15, 3, 	Kind.RECORD_KIND	),
	
	#	Do NOT move references without changing IsRefSimpleKey
	#	Must be on a multiple of 4 boundary.
	KeyData( "HardRef", 		16, 1, 	Kind.RECORD_KIND	),
	KeyData( "SoftRef", 		17, 1, 	Kind.RECORD_KIND	),
	KeyData( "WeakRef", 		18, 1, 	Kind.RECORD_KIND	),
	KeyData( "ReservedRef", 	19, 1, 	Kind.RECORD_KIND	),
	
	#	Do NOT move references without changing IsMapSimpleKey
	#	Must be on a multiple of 4 boundary.
	KeyData( "CacheEqMap",		20, 3, 	Kind.MAP_KIND		),
	KeyData( "HardEqMap", 		21, 3,  Kind.MAP_KIND		),
	KeyData( "HardIdMap",		22, 3,  Kind.MAP_KIND		),
	KeyData( "WeakIdMap",		23, 3,  Kind.MAP_KIND		),
	
	#	More arbitrary keys.
	KeyData( "HashMapData", 	24, 0,	Kind.VECTOR_KIND	),
	KeyData( "Unassigned", 		25, 0,	Kind.KEYLESS_KIND	),
	KeyData( "Undefined", 		26, 0,  Kind.KEYLESS_KIND	),
	KeyData( "Indeterminate",  	27, 0,  Kind.KEYLESS_KIND	),
	KeyData( "Element",      	28, 1, 	Kind.MIXED_KIND		),
	KeyData( "AttrMap",			29,	1,	Kind.ATTR_KIND		),
	KeyData( "Exception",      	30, 3,  Kind.RECORD_KIND	),
	KeyData( "Double",          31, wordsPerDouble,  Kind.WRECORD_KIND   ),
	KeyData( "Present",         32, 0,  Kind.KEYLESS_KIND   ),
	KeyData( "External",		33, 1, 	Kind.EXTERNAL_KIND  ),
	KeyData( "InputStream",		34, 2, 	Kind.EXTERNAL_KIND  ),
	KeyData( "OutputStream",	35, 1, 	Kind.EXTERNAL_KIND  ),
	KeyData( "BigInt",	        36, 1, 	Kind.EXTERNAL_KIND  ),
	KeyData( "Rational",	    37, 1, 	Kind.EXTERNAL_KIND  ),
	KeyData( "VirtualMachine",	38, 1,  Kind.EXTERNAL_KIND ),
]


################################################################################
#	Generates the code snippet for the key.hpp header file.
################################################################################

def generateHPP():
	"""Creates the simple.hpp.auto file."""
	file = open( "simplekey.hpp.auto", 'w' )
	
	# Generate the Kinds #define's.
	for k in Kind.values():
		file.write( "#define {}_KIND {}\n".format( k.name(), k.kind() ) )

	# Generate the Layout #define's.
	for x in Layout.values():
		file.write( "#define {} {}\n".format( x.name(), x.ordinal() ) )

	# Generate the SimpleKey #define's.
	for skey in keys:
		file.write( "#define {}ID {}\n".format( skey.name(), skey.ordinal() ) )
		file.write( 
			"#define sys{}Key MAKE_KEY( {}, {}, {}, {} )\n".format( 
				skey.name(), 
				skey.ordinal(), 
				skey.nfields(), 
				skey.kind().sublayout(), 
				skey.kind().layout().ordinal() 
			) 
		)

	file.close()


################################################################################
#	Generates the code snippet for the key.cpp source file.
################################################################################

def generateCPP():
	"""Creates the simplekey.cpp.auto file"""
	file = open( "simplekey.cpp.auto", 'w' )
	for a in keys:
		file.write( 'case {}: return "{}";\n'.format( a.ordinal(), a.name() ) )
	file.close()

def generateNameToKeyCPP():
	file = open( "name_to_simplekey.cpp.auto", 'w' )
	for a in keys:
		file.write( 'if ( name == "{}" ) return sys{}Key;\n'.format( a.name(), a.name() ) )
	file.close()

################################################################################
#	Main
################################################################################

def generate():
	generateHPP()
	generateCPP()
	generateNameToKeyCPP()

generate()

################################################################################

