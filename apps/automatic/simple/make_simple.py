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

################################################################################
#	Utility functions
################################################################################

def counter():
	"""A simple helper routine for allocating serial numbers."""
	n = 0
	while True:
		yield n
		n += 1

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
		self._ordinal = Layout._count.next()
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
Layout.UNUSED5_LAYOUT	= Layout( "UNUSED5_LAYOUT" )
Layout.UNUSED6_LAYOUT	= Layout( "UNUSED6_LAYOUT" )
Layout.UNUSED7_LAYOUT	= Layout( "UNUSED7_LAYOUT" )


################################################################################
#	The Kind class denotes a 4-bit refinement of a layout that provides
#	enough type information for general low-level operators, such as 
#	explode ('...'). [Kinds represent built-in essential classes.]
################################################################################

class Kind:
	
	_values = []
	_next = counter()

	def __init__( self, name, layout ):
		self._name = name
		self._ordinal = Kind._next.next()
		self._layout = layout
		Kind._values.append( self )

	def ordinal( self ):
		return self._ordinal

	def layout( self ):
		return self._layout

	def name( self ):
		return self._name

	@staticmethod
	def values():
		for i in Kind._values:
			yield i	

Kind.KEYLESS_KIND 	= Kind( "KEYLESS", Layout.KEYLESS_LAYOUT )
Kind.RECORD_KIND	= Kind( "RECORD", Layout.RECORD_LAYOUT )
Kind.PAIR_KIND 		= Kind( "PAIR", Layout.RECORD_LAYOUT )
Kind.MAP_KIND 		= Kind( "MAP", Layout.RECORD_LAYOUT )
Kind.VECTOR_KIND 	= Kind( "VECTOR", Layout.VECTOR_LAYOUT )
Kind.STRING_KIND 	= Kind( "STRING", Layout.STRING_LAYOUT )
Kind.ATTR_KIND 		= Kind( "ATTR", Layout.MIXED_LAYOUT )
Kind.MIXED_KIND 	= Kind( "MIXED", Layout.MIXED_LAYOUT )	


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

keys = [
	KeyData( "Absent", 			0, 	0, 	Kind.KEYLESS_KIND 	),
	KeyData( "Bool", 			1, 	0, 	Kind.KEYLESS_KIND	),
	KeyData( "Key",				2, 	4, 	Kind.RECORD_KIND	),
	KeyData( "Termin",			3, 	0, 	Kind.KEYLESS_KIND	),
	KeyData( "Nil", 			4, 	0, 	Kind.KEYLESS_KIND	),
	KeyData( "Pair", 			5, 	2, 	Kind.PAIR_KIND		),
	KeyData( "Vector",			6, 	0, 	Kind.VECTOR_KIND	),
	KeyData( "String", 			7, 	0, 	Kind.STRING_KIND	),
	KeyData( "Symbol", 			8, 	1, 	Kind.KEYLESS_KIND	),
	KeyData( "Small", 			9, 	0, 	Kind.KEYLESS_KIND	),
	KeyData( "Float", 			10, 0, 	Kind.KEYLESS_KIND	),
	KeyData( "Unicode", 		11, 0, 	Kind.KEYLESS_KIND	),
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
	KeyData( "Exception",      	30, 3,  Kind.RECORD_KIND	)
]


################################################################################
#	Generates the code snippet for the key.hpp header file.
################################################################################

def generateHPP():
	"""Creates the simple.hpp.auto file."""
	file = open( "simplekey.hpp.auto", 'w' )
	
	# Generate the Kinds #define's.
	for k in Kind.values():
		file.write( "#define {}_KIND {}\n".format( k.name(), k.ordinal() ) )

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
				skey.kind().ordinal(), 
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

################################################################################
#	Main
################################################################################

def generate():
	generateHPP()
	generateCPP()

generate()

################################################################################

