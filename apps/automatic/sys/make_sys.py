#!/usr/bin/python

################################################################################
#   Utilities
################################################################################

def capitalise( field ):
	return field[ 0 ].upper() + field[ 1: ]


################################################################################
#   Arity
################################################################################

class Arity:
	
	def __init__( self, count, more = False ):
		self.count = count
		self.more = more
		
	def __str__( self ):
		m = str( self.more ).lower()
		return "Arity({},{})".format( self.count, m )

Arity.MANY = Arity( 0, True )

def makeArity( a ):
	if isinstance( a, int ):
		return Arity( a )
	elif isinstance( a, Arity ):
		return a
	else:
		raise Exception

################################################################################
#   SysInfo
################################################################################

class SysInfo:

	def __init__( self, name, in_arity, out_arity, d ):
		self.name = name
		self.in_arity = makeArity( in_arity )
		self.out_arity = makeArity( out_arity )
		self.doc_string = d

	def getName( self ):
		return self.name

	def getInArity( self ):
		return self.in_arity

	def getOutArity( self ):
		return self.out_arity

	def getDocString( self ):
		return self.doc_string


################################################################################
#   LogCreated
################################################################################

class LogCreated:
	
	def __init__( self ):
		#  final Map< String, SysInfo > mapping = new HashMap< String, SysInfo >();
		self.mapping = {}
		self.order = []
	
		#   private final List< File > cpp_file_list = new ArrayList< File >();
		self.cpp_file_list = []

		#	private final List< File > hpp_file_list = new ArrayList< File >();
		self.hpp_file_list = []
	
	def addSysConst( self, ginger_name, in_arity, out_arity, internal_name, doc_string = None ):
		info = self.mapping.get( ginger_name )
		if info is None:
			self.mapping[ ginger_name ] = SysInfo( internal_name, in_arity, out_arity, doc_string )
			self.order.append( ginger_name )
		else:
			raise Exception( "Repeated name: " + info.getName() )

		
	def addCppFile( self, name ):
		self.cpp_file_list.append(  name  )
	
	
	def addHppFile( self, name ):
		self.hpp_file_list.append( name )

	def generateSysMapInclude( self, inc ):
		for ginger_name in self.order:
			p = self.mapping[ ginger_name ]
			inc.write(
				"SysMap::value_type( \"{}\", SysInfo( {}, {}, {}, nullptr ) ),\n".format(
					ginger_name,
					p.getInArity(),
					p.getOutArity(),
					p.getName()
				)
			)

	def generateSysConstTableInclude( self, inc ):
		for ginger_name in self.mapping:
			p = self.mapping[ ginger_name ]
			inc.write(
				"this->table[ \"{}\" ] = new SysConst( \"sysfn\", \"{}\" );\n".format(
					ginger_name,
					ginger_name
				)
			)

	def includes( self, inc, files ):
		for f in files:
			inc.write( "#include \"{}\"\n".format( f ) )

	def generateDatatypesCPP( self, inc ):
		self.includes( inc, self.cpp_file_list )

	def generateDatatypesHPP( self, inc ):
		self.includes( inc, self.hpp_file_list )

	

################################################################################
#   MethodOptions class
################################################################################

class MethodOptions:

	"""
		C = constructor
		T = initialiser (using T temporarily as a refactoring step to switching to I) 
		D = deconstructor
		L/l = liker (using IsLike${Name} versus testing against sys${Name}Key)
		R/r = recogniser (using Is${Name} versus testing against sys${Name}Key)
		F = record fields
	 	G = vector get-indexer
	 	S = vector set-indexer
	"""
	def __init__( self, opts = "CTDLRFGS" ):
		self.gen_constructor = ( opts.find( 'C' ) != -1 )
		self.gen_initialiser = ( opts.find( 'T' ) != -1 )
		self.gen_deconstructor = ( opts.find( 'D' ) != -1 )
		self.gen_recogniser_via_key = ( opts.find( 'r' ) != -1 )
		self.gen_recogniser = ( opts.find( 'R' ) != -1 or self.gen_recogniser_via_key )
		self.gen_liker_via_key = ( opts.find( 'l' ) != -1 )
		self.gen_liker = ( opts.find( 'L' ) != -1 or self.gen_liker_via_key )
		self.gen_fields = ( opts.find( 'F' ) != -1 )
		self.gen_get_indexer = ( opts.find( 'G' ) != -1 )
		self.gen_set_indexer = ( opts.find( 'S' ) != -1 )

	def add( self, opts ):
		if opts.find( 'C' ) != -1:
			self.gen_constructor = True
		if opts.find( 'T' ) != -1:
			self.gen_initialiser = True
		if opts.find( 'D' ) != -1:
			self.gen_deconstructor = True
		if opts.find( 'r' ) != -1:
			self.gen_recogniser_via_key = True
		if opts.find( 'R' ) != -1 or self.gen_recogniser_via_key:
			self.gen_recogniser = True
		if opts.find( 'l' ) != -1:
			self.gen_liker_via_key = True
		if opts.find( 'L' ) != -1 or self.gen_liker_via_key:
			self.gen_liker = True
		if opts.find( 'F' ) != -1:
			self.gen_fields = True
		if opts.find( 'G' ) != -1:
			self.gen_get_indexer = True
		if opts.find( 'S' ) != -1:
			self.gen_set_indexer = True

	def isGenConstructor( self ):
		return self.gen_constructor

	def isGenDeconstructor( self ):
		return self.gen_deconstructor

	def isGenRecogniser( self ):
		return self.gen_recogniser

	def isGenRecogniserViaKey( self ):
		return self.gen_recogniser_via_key

	def isGenLiker( self ):
		return self.gen_liker

	def isGenLikerViaKey( self ):
		return self.gen_liker_via_key

	def isGenFields( self ):
		return self.gen_fields

	def isGenGetIndexer( self ):
		return self.gen_get_indexer

	def isGenSetIndexer( self ):
		return self.gen_set_indexer

	def isGenInitialiser( self ):
		return self.gen_initialiser


################################################################################
#   DataClassGenerator
################################################################################


class DataClassGenerator( object ):

	def __init__( self, sysconsts, classroot, prefixes = [ "" ] ):
		self.alt_key_roots = prefixes
		self.data_key_root = classroot
		self.log = sysconsts

	def allKeyRoots( self ):
		if self.alt_key_roots == None:
			self.alt_key_roots.append( "" )
		return self.alt_key_roots

	def hasSingleClass( self ):
		return len( self.alt_key_roots ) <= 1

	#def addKeyRoots( self, *alt ):
	#	if not self.alt_key_roots:
	#		self.alt_key_roots = []
	#	for a in alt:
	#		self.alt_key_roots.append( a )

	def keyName( self ):
		return "sys" + self.data_key_root + "Key";

	def altKeyName( self, aname ):
		return "sys" + aname + self.data_key_root + "Key";

	def isName( self ):
		return "Is" + self.data_key_root

	def altIsName( self, aname ):
		return "Is" + aname + self.data_key_root

	def isLikeName( self ):
		return "IsLike" + self.data_key_root


	def isUpdateableName( self ):
		return "IsUpdateable" + self.data_key_root

	def consName( self ):
		return "sysNew" + self.data_key_root

	def altConsName( self, aname ):
		return "sysNew" + aname + self.data_key_root

	def getIndexName( self ):
		return "sysGetIndex" + self.data_key_root	

	def setIndexName( self ):
		return "sysSetIndex" + self.data_key_root	

	def altInitialiserName( self, aname ):
		return "sysInit" + aname + self.data_key_root

	# This is an abstract method - not sure how you do this in Python.
	def generate( self, cpp, hpp ):
		raise Exception( "Abstract Method" )

	def generateFilesWithPrefix( self, stem ):
		cppfile = stem + ".cpp.auto"
		hppfile = stem + ".hpp.auto"
		cpp = open( cppfile, 'w' )
		hpp = open( hppfile, 'w' )
		self.generate( cpp, hpp )
		cpp.close()
		hpp.close()
		self.log.addCppFile( cppfile )
		self.log.addHppFile( hppfile )

	def addSysConst( self, gname, in_arity, out_arity, cname ):
		self.log.addSysConst( gname, in_arity, out_arity, cname )

	def addSysConstWithIntegerArities( self, gname, a, b, cname ):
		self.log.addSysConst( gname, Arity( a ), Arity( b ), cname )

	def writeTestPart( self, v, cpp ):
		if self.hasSingleClass():
			if self.options.isGenRecogniserViaKey():
				cpp.write( "IsObj( {} ) && ( *RefToPtr4( {} ) == {} )".format( v,v, self.keyName() ) )
			else:
				cpp.write( "{}( {} )".format( self.isName(), v ) )
		else:
			if self.options.isGenLikerViaKey():
				cpp.write( "IsObj( {} ) && ( false".format( v ) )
				for alt_name in allKeyRoots():
					cpp.write( " || ( *RefToPtr4( {} ) == {} )".format( v, self.altKeyName( alt_name ) ) )
				cpp.write( " )" )
			else:
				cpp.write( "{}( {} )".format( self.isLikeName(), v ) )

	def generateRecogniser( self, cpp, hpp ):
		if self.hasSingleClass():
			self.generateSingleRecogniser( "", cpp, hpp )
		else:
			all_roots = self.allKeyRoots()
			for alt_name in all_roots:
				self.generateSingleRecogniser( alt_name, cpp, hpp )
			if self.options.isGenLiker():
				self.generateLikeRecogniser( all_roots, cpp, hpp )

	def generateLikeRecogniser( self, all_roots, cpp, hpp ):
		recogniser_name = "sysIsLike" + self.data_key_root
		cpp.write( "Ref * {}( Ref * pc, class MachineClass * vm ) {{\n".format( recogniser_name ) )
		cpp.write( "    if ( vm->count == 1 ) {\n" )
		cpp.write( "        Ref r = vm->fastPeek();\n" )
		if self.options.isGenLikerViaKey():
			cpp.write( "        if ( not IsObj( r ) ) { vm->fastPeek() = SYS_FALSE; return pc; }\n" );
			cpp.write( "        Ref k = *RefToPtr4( r );\n" )
			for alt_name in all_roots:
				cpp.write( "        if ( k == {} ) {{ vm->fastPeek() = SYS_TRUE; return pc; }}\n".format( self.altKeyName( alt_name ) ) )
			cpp.write( "        vm->fastPeek() = SYS_FALSE;\n" )
		else:
			cpp.write( "        vm->fastPeek() = {}( r ) ? SYS_TRUE : SYS_FALSE;\n".format( self.isLikeName() ) )
		
		cpp.write( "        return pc;\n" )
		cpp.write( "    } else {\n" )
		cpp.write( "        throw Mishap( \"Wrong number of arguments for recogniser\" );\n" )
		cpp.write( "    }\n" )
		cpp.write( "}\n\n" )

		hpp.write( "extern Ref * {}( Ref * pc, MachineClass * vm );\n".format( recogniser_name ) )
		self.addSysConst( recogniser_name, 1, 1, recogniser_name );

	def generateSingleRecogniser( self, alt_name, cpp, hpp ):
		recogniser_name = "sysIs" + alt_name + self.data_key_root
		cpp.write( "Ref * {}( Ref * pc, class MachineClass * vm ) {{\n".format( recogniser_name ) )
		cpp.write( "    if ( vm->count == 1 ) {\n" )
		cpp.write( "        Ref r = vm->fastPeek();\n" )
		if self.options.isGenRecogniserViaKey():
			cpp.write( "        vm->fastPeek() = ( IsObj( r ) && ( *RefToPtr4( r ) == {} ) ) ? SYS_TRUE : SYS_FALSE;\n".format( self.altKeyName( alt_name ) ) )
		else:
			cpp.write( "        vm->fastPeek() = {}( r ) ? SYS_TRUE : SYS_FALSE;\n".format( self.altIsName( alt_name ) ) )
		cpp.write( "        return pc;\n" )
		cpp.write( "    } else {\n" )
		cpp.write( "        throw Mishap( \"Wrong number of arguments for recogniser\" );\n" )
		cpp.write( "    }\n" )
		cpp.write( "}\n\n" )

		hpp.write( "extern Ref * {}( Ref * pc, MachineClass * vm );\n".format( recogniser_name ) )
		self.addSysConst( recogniser_name, 1, 1, recogniser_name );

################################################################################
#   WordRecordClassGenerator
################################################################################

class WordRecordClassGenerator( DataClassGenerator ):
	
	memcpy = False

	def __init__( self, sysconsts, className, options, prefixes = [ "" ] ):
		super( WordRecordClassGenerator, self ).__init__( sysconsts, className, prefixes )
		self.options = options
		
	def generate( self, cpp, hpp ):
		if self.options.isGenRecogniser():
			self.generateRecogniser( cpp, hpp )


################################################################################
#   FullRecordClassGenerator
################################################################################

class FullRecordClassGenerator( DataClassGenerator ):
	
	memcpy = False

	def __init__( self, sysconsts, className, fieldNames, prefixes = [ "" ] ):
		super( FullRecordClassGenerator, self ).__init__( sysconsts, className, prefixes )
		self.options = MethodOptions()
		self.fieldNames = fieldNames

	def generate( self, cpp, hpp ):
		for aname in self.allKeyRoots():
			if self.options.isGenConstructor():
				self.generateConstructor( aname, cpp, hpp )
		if self.options.isGenRecogniser():
			self.generateRecogniser( cpp, hpp )
		for i in range( len( self.fieldNames ) ):
			self.generateField( i, cpp, hpp )

	def generateField( self, n, cpp, hpp ):
		field = self.fieldNames[ n ];
		cap_field = capitalise( field );
		accessorName = "sys" + cap_field + "Field"
		cpp.write( "Ref * {}( Ref * pc, class MachineClass * vm ) {{\n".format( accessorName ) )
		cpp.write( "    if ( vm->count == 1 ) {\n" )
		cpp.write( "        Ref x = vm->fastPeek();\n" )

		#cpp.write( "        if ( {}( x ) ) {{\n".format( self.isName() ) )
		cpp.write( "        if ( " )
		self.writeTestPart( "x", cpp ) 
		cpp.write( " ) {\n" )
		
		cpp.write( "            vm->fastPeek() = RefToPtr4( x )[ {} ];\n".format( n + 1 ) )
		cpp.write( "        } else {\n" )
		cpp.write( "            throw Mishap( \"Trying to take the {} of non-{}\" );\n".format( field, self.data_key_root ) )
		cpp.write( "        }\n" )
		cpp.write( "        return pc;\n" )
		cpp.write( "    } else {\n" )
		cpp.write( "        throw Mishap( \"Wrong number of arguments\" );\n" )
		cpp.write( "    }\n" )
		cpp.write( "}\n\n" )
		
		hpp.write( "extern Ref * {}( Ref * pc, MachineClass * vm );\n".format( accessorName ) )
		self.addSysConst( field, 1, 1, accessorName );

	def generateConstructor( self, altName, cpp, hpp ):
		constructorName = self.altConsName( altName )
		keyName = self.altKeyName( altName )
		cpp.write( "Ref * {}( Ref * pc, class MachineClass * vm ) {{\n".format( constructorName ) )
		cpp.write( "    if ( vm->count == {} ) {{\n".format( len( self.fieldNames ) ) )
		cpp.write( "        XfrClass xfr( vm->heap().preflight( pc, {} ) );\n".format( len( self.fieldNames ) + 1 ) )
		cpp.write( "        xfr.setOrigin();\n" )
		cpp.write( "        xfr.xfrRef( {} );\n".format( keyName ) )
		
		#	Not sure which is better.
		if FullRecordClassGenerator.memcpy:
			cpp.write( "        xfr.xfrCopy( ++vm->vp -= {0}, {0} );\n".format( len( self.fieldNames ) ) )
			cpp.write( "        vm->fastSet( xfr.makeRef() );\n" );
		else:
			for i in range( len( self.fieldNames ) - 1, -1, -1 ):
				cpp.write( "        Ref f{} = vm->fastPop();\n".format( i ) )
			for i in range( len( self.fieldNames ) ):
				cpp.write( "        xfr.xfrRef( f{} );\n".format( i ) )
			cpp.write( "        vm->fastPush( xfr.makeRef() );\n" )
			cpp.write( "        return pc;\n" )
		
		cpp.write( "    } else {\n" );
		cpp.write( "        throw Mishap( \"Wrong number of arguments for {}\" );\n".format( constructorName ) )
		cpp.write( "    }\n" )
		cpp.write( "}\n\n" )

		hpp.write( "extern Ref * {}( Ref * pc, MachineClass * vm );\n".format( constructorName ) )
		self.addSysConst( "new" + altName + self.data_key_root, len( self.fieldNames ), 1, constructorName )


################################################################################
#   FullVectorClassGenerator
################################################################################
	
class FullVectorClassGenerator( DataClassGenerator ):
	
	def __init__( self, sysconsts, className, options = MethodOptions( "CTGRS" ), prefixes = [ "" ] ):
		super( FullVectorClassGenerator, self ).__init__( sysconsts, className, prefixes )
		self.options = options

	def generate( self, cpp, hpp ):
		for aname in self.allKeyRoots():
			if self.options.isGenConstructor():
				self.generateConstructor( aname, cpp, hpp )
			if self.options.isGenInitialiser():
				self.generateInitialiser( aname, cpp, hpp )
		if self.options.isGenRecogniser():
			self.generateRecogniser( cpp, hpp )
		if self.options.isGenGetIndexer(): 
			self.generateGetIndex( cpp, hpp )
		if self.options.isGenSetIndexer():
			self.generateSetIndex( cpp, hpp )
		
	def generateGetIndex( self, cpp, hpp ):
		cpp.write( "Ref * {}( Ref * pc, MachineClass * vm ) {{\n".format( self.getIndexName() ) )
		cpp.write( "   if ( vm->count == 2 ) {\n" )
		cpp.write( "       Ref v = vm->fastPop();\n" )
		cpp.write( "       Ref n = vm->fastPeek();\n" )
		cpp.write( "       Ref * p = RefToPtr4( v );\n" )
		cpp.write( "       if ( " )
		self.writeTestPart( "v", cpp )
		cpp.write( " ) {\n" )
		#cpp.write( "       if ( {}( v ) ) {{\n".format( self.isName() ) )
		cpp.write( "           if ( IsSmall( n ) && LongToSmall( 1 ) <= n && n <= p[ -1 ] ) {\n" )
		cpp.write( "               vm->fastPeek() = p[ SmallToLong( n ) ];\n" )
		cpp.write( "           } else {\n" )
		cpp.write( "               throw Mishap( \"Invalid index\" ).culprit( \"Index\", refToShowString( n ) ).culprit( \"Vector\", refToShowString( v ) );\n" )
		cpp.write( "           }\n" )
		cpp.write( "           return pc;\n" )
		cpp.write( "       } else {\n" )
		cpp.write( "           throw Mishap( \"Vector needed\" );\n" )
		cpp.write( "       }\n" )
		cpp.write( "   } else {\n" )
		cpp.write( "       throw Mishap( \"Wrong number of arguments for index\" );\n" )
		cpp.write( "   }\n" )
		cpp.write( "}\n\n" )
		
		hpp.write( "extern Ref * {}( Ref * pc, MachineClass * vm );\n".format( self.getIndexName() ) )
		self.addSysConst( "getIndex" + self.data_key_root, 2, 1, self.getIndexName() );

	def generateSetIndex( self, cpp, hpp ):
		cpp.write( "Ref * {}( Ref * pc, MachineClass * vm ) {{\n".format( self.setIndexName() ) )
		cpp.write( "   if ( vm->count == 3 ) {\n" )
		cpp.write( "       Ref v = vm->fastPop();\n" )
		cpp.write( "       Ref n = vm->fastPop();\n" )
		cpp.write( "       Ref x = vm->fastPop();\n" )
		cpp.write( "       Ref * p = RefToPtr4( v );\n" )
		cpp.write( "       if ( {}( v ) ) {{\n".format( self.isUpdateableName() ) )
		cpp.write( "           if ( IsSmall( n ) && LongToSmall( 1 ) <= n && n <= p[ -1 ] ) {\n" )
		cpp.write( "               p[ SmallToLong( n ) ] = x;\n" )
		cpp.write( "           } else {\n" )
		cpp.write( "               throw Mishap( \"Small integer index needed\" );\n" )
		cpp.write( "           }\n" )
		cpp.write( "           return pc;\n" )
		cpp.write( "       } else {\n" )
		cpp.write( "           throw Mishap( \"Updateable vector needed\" );\n" )
		cpp.write( "       }\n" )
		cpp.write( "   } else {\n" )
		cpp.write( "       throw Mishap( \"Wrong number of arguments for index-based update\" );\n" )
		cpp.write( "   }\n" )
		cpp.write( "}\n\n" )
		
		hpp.write( "extern Ref * {}( Ref * pc, MachineClass * vm );\n".format( self.setIndexName() ) )
		self.addSysConst( "setIndex" + self.data_key_root, 2, 1, self.setIndexName() );

	def generateConstructor( self, altName, cpp, hpp ):
		constructorName = self.altConsName( altName );
		keyName = self.altKeyName( altName );
		cpp.write( "Ref * {}( Ref * pc, MachineClass * vm ) {{\n".format( constructorName ) )
		cpp.write( "    int n = vm->count;\n" )
		cpp.write( "    XfrClass xfr( vm->heap().preflight( pc, 2 + n ) );\n" )
		cpp.write( "    xfr.xfrRef( LongToSmall( n ) );\n" )
		cpp.write( "    xfr.setOrigin();\n" )
		cpp.write( "    xfr.xfrRef( {} );\n".format( keyName ) )
		cpp.write( "    xfr.xfrCopy( ++vm->vp -= n, n );\n" )
		cpp.write( "    vm->fastSet( xfr.makeRef() );\n" )
		cpp.write( "    return pc;\n" )
		cpp.write( "}\n\n" )

		hpp.write( "extern Ref * {}( Ref * pc, MachineClass * vm );\n".format( constructorName ) )
		self.addSysConst( 
			"new" + altName + self.data_key_root, 
			Arity.MANY, 
			Arity( 1 ), 
			constructorName
		)

	def generateInitialiser( self, altName, cpp, hpp ):
		initName = self.altInitialiserName( altName );
		keyName = self.altKeyName( altName );
		cpp.write( "Ref * {}( Ref * pc, MachineClass * vm ) {{\n".format( initName ) )
		cpp.write( "    if ( vm->count == 2 ) {\n" );
		cpp.write( "        Ref def = vm->fastPop();\n" )
		cpp.write( "        Ref count = vm->fastPeek();\n" )
		cpp.write( "        if ( IsSmall( count ) ) {\n" )
		cpp.write( "            long n = SmallToLong( count );\n" )
		cpp.write( "            XfrClass xfr( vm->heap().preflight( pc, 2 + n ) );\n" )
		cpp.write( "            xfr.xfrRef( count );\n" )
		cpp.write( "            xfr.setOrigin();\n" )
		cpp.write( "            xfr.xfrRef( {} );\n".format( keyName ) )
		cpp.write( "            xfr.xfrDup( def, n );\n" )
		cpp.write( "            vm->fastSet( xfr.makeRef() );\n" )
		cpp.write( "            return pc;\n" )
		cpp.write( "        } else {\n" )
		cpp.write( "            throw Mishap( \"Small integer needed as a size for initialiser\" );\n" )
		cpp.write( "        }\n" )
		cpp.write( "    } else {\n" )
		cpp.write( "        throw Mishap( \"Wrong number of arguments for initialiser\" );\n" )
		cpp.write( "    }\n" )
		cpp.write( "}\n\n" )

		hpp.write( "extern Ref * {}( Ref * pc, MachineClass * vm );\n".format( initName ) )
		self.addSysConst( 
			"init" + altName + self.data_key_root, 
			Arity( 2 ), 
			Arity( 1 ), 
			initName
		)


################################################################################
#   FullMixedClassGenerator
################################################################################

class FullMixedClassGenerator( DataClassGenerator ):

	def __init__( self, sysconsts, className, options, fieldNames, prefixes = [ "" ] ):
		super( FullMixedClassGenerator, self ).__init__( sysconsts, className, prefixes )
		self.options = options
		self.fieldNames = fieldNames

	def generate( self, cpp, hpp ):
		for aname in self.allKeyRoots():
			if self.options.isGenConstructor():
				self.generateConstructor( aname, cpp, hpp )
		if self.options.isGenRecogniser(): 
			self.generateRecogniser( cpp, hpp );
		for i in range( len( self.fieldNames ) ):
			if self.options.isGenFields():
				self.generateField( i, cpp, hpp );
		if self.options.isGenGetIndexer(): 
			self.generateGetIndexer( cpp, hpp ); 
		if self.options.isGenSetIndexer(): 
			self.generateSetIndexer( cpp, hpp ); 

	def generateField( self, n, cpp, hpp ):
		field = self.fieldNames[ n ];
		cap_field = capitalise( field );
		accessorName = "sys" + cap_field + "Field";

		cpp.write( "Ref * {}( Ref * pc, class MachineClass * vm ) {{\n".format( accessorName ) )
		cpp.write( "    if ( vm->count == 1 ) {\n" );
		cpp.write( "        Ref x = vm->fastPeek();\n" );
		cpp.write( "        if ( {}( x ) ) {{\n".format( self.isName() ) )
		cpp.write( "            vm->fastPeek() = RefToPtr4( x )[ {} ];\n".format( n + 1 ) )
		cpp.write( "        } else {\n" );
		cpp.write( "            throw Mishap( \"Trying to take the {} of non-{}\" );\n".format( field, self.data_key_root ) )
		cpp.write( "        }\n" );
		cpp.write( "        return pc;\n" );
		cpp.write( "    } else {\n" );
		cpp.write( "        throw Mishap( \"Wrong number of arguments\" );\n" );
		cpp.write( "    }\n" );
		cpp.write( "}\n\n" );
		
		hpp.write( "extern Ref * {}( Ref * pc, MachineClass * vm );\n".format( accessorName ) )
		self.addSysConst( field, 1, 1, accessorName );

	def generateGetIndexer( self, cpp, hpp ):
		cpp.write( "Ref * {}( Ref * pc, MachineClass * vm ) {{\n".format( self.getIndexName() ) )
		cpp.write( "   if ( vm->count == 2 ) {\n" );
		cpp.write( "       Ref n = vm->fastPop();\n" );
		cpp.write( "       Ref v = vm->fastPeek();\n" );
		cpp.write( "       Ref * p = RefToPtr4( v );\n" );

		cpp.write( "       if ( {}( v ) ) {{\n".format( self.isName() ) )
		cpp.write( "           if ( IsSmall( n ) && LongToSmall( 1 ) <= n && n <= p[ -1 ] ) {\n" );
		cpp.write( "               vm->fastPeek() = p[ {} + SmallToLong( n ) ];\n".format( len( self.fieldNames ) ) )
		cpp.write( "           } else {\n" );
		cpp.write( "               throw Mishap( \"Small integer index needed\" );\n" );
		cpp.write( "           }\n" );
		cpp.write( "           return pc;" );
		cpp.write( "       } else {\n" );
		cpp.write( "           throw Mishap( \"Vector needed\" );\n" );
		cpp.write( "       }\n" );
		cpp.write( "   } else {\n" );
		cpp.write( "       throw Mishap( \"Wrong number of arguments for index\" );\n" );
		cpp.write( "   }\n" );
		cpp.write( "}\n\n" );
		
		hpp.write( "extern Ref * {}( Ref * pc, MachineClass * vm );\n".format( self.getIndexName() ) )
		self.addSysConst( "getIndex" + self.data_key_root, 2, 1, self.getIndexName() )

	def generateSetIndexer( self, cpp, hpp ):
		cpp.write( "Ref * {}( Ref * pc, MachineClass * vm ) {{\n".format( self.setIndexName() ) )
		cpp.write( "   if ( vm->count == 2 ) {\n" );
		cpp.write( "       Ref n = vm->fastPop();\n" );
		cpp.write( "       Ref v = vm->fastPeek();\n" );
		cpp.write( "       Ref * p = RefToPtr4( v );\n" );
		cpp.write( "       if ( {}( v ) ) {{\n".format( self.isUpdateableName() ) )
		cpp.write( "           if ( IsSmall( n ) && LongToSmall( 1 ) <= n && n <= p[ -1 ] ) {\n" );
		cpp.write( "               vm->fastPeek() = p[ {} + SmallToLong( n ) ];\n".format( len( self.fieldNames ) ) )
		cpp.write( "           } else {\n" );
		cpp.write( "               throw Mishap( \"Small integer index needed\" );\n" );
		cpp.write( "           }\n" );
		cpp.write( "           return pc;\n" );
		cpp.write( "       } else {\n" );
		cpp.write( "           throw Mishap( \"Vector needed\" );\n" );
		cpp.write( "       }\n" );
		cpp.write( "   } else {\n" );
		cpp.write( "       throw Mishap( \"Wrong number of arguments for index\" );\n" );
		cpp.write( "   }\n" );
		cpp.write( "}\n\n" );
		
		hpp.write( "extern Ref * {}( Ref * pc, MachineClass * vm );\n".format( self.setIndexName() ) )
		self.addSysConst( "setIndex" + self.data_key_root, 2, 1, self.setIndexName() )

	def generateConstructor( self, altName, cpp, hpp ):
		constructorName = self.altConsName( altName );
		keyName = self.altKeyName( altName );
		cpp.write( "Ref * {}( Ref * pc, class MachineClass * vm ) {{\n".format( constructorName ) )
		cpp.write( "    const int N = vm->count;\n" );
		cpp.write( "    const int length = N - {};\n".format( len( self.fieldNames ) ) )
		cpp.write( "    if ( length >= 0 ) {\n" );
		cpp.write( "        XfrClass xfr( vm->heap().preflight( pc, N + 2 ) );\n" );
		cpp.write( "        xfr.xfrRef( LongToSmall( length ) );\n" );
		cpp.write( "        xfr.setOrigin();\n" );
		cpp.write( "        xfr.xfrRef( {} );\n".format( keyName ) )
		cpp.write( "        xfr.xfrCopy( ++vm->vp -= N, N );\n" );
		cpp.write( "        vm->fastSet( xfr.makeRef() );\n" );
		cpp.write( "        return pc;\n" );
		cpp.write( "    } else {\n" );
		cpp.write( "        throw Mishap( \"Wrong number of arguments for {}\" );\n".format( constructorName  ) )
		cpp.write( "    }\n" );
		cpp.write( "}\n\n" );

		hpp.write( "extern Ref * {}( Ref * pc, MachineClass * vm );\n".format( constructorName ) )
		self.addSysConst( 
			"new" + altName + self.data_key_root, 
			Arity( len( self.fieldNames ), True ), 
			Arity( 1 ), 
			constructorName 
		)


################################################################################
#   Main
################################################################################

sysconsts = LogCreated()

ref = FullRecordClassGenerator( sysconsts, "Ref", [ "refCont" ], prefixes = [ "Hard", "Soft", "Weak" ] )
ref.options.add( "r" )
ref.generateFilesWithPrefix( "ref" )

ref = FullRecordClassGenerator( sysconsts, "Pair", [ "head", "tail" ] )
ref.generateFilesWithPrefix( "pair" )

ref = FullRecordClassGenerator( sysconsts, "Maplet", [ "mapletKey", "mapletValue" ] )
ref.generateFilesWithPrefix( "maplet" )

ref = FullVectorClassGenerator( sysconsts, "Vector", prefixes = [ "", "Updateable" ] )
ref.generateFilesWithPrefix( "vector" )

ref = FullMixedClassGenerator( sysconsts, "AttrMap", MethodOptions( "CRF" ), [ "attrMapName" ] )
ref.generateFilesWithPrefix( "attrmap" )

ref = FullMixedClassGenerator( sysconsts, "Element", MethodOptions( "CRGF" ), [ "elementAttrMap" ] )
ref.generateFilesWithPrefix( "element" )

ref = WordRecordClassGenerator( sysconsts, "Double", MethodOptions( "R" ) )
ref.generateFilesWithPrefix( "double" )
ref = WordRecordClassGenerator( sysconsts, "External", MethodOptions( "r" ) )
ref.generateFilesWithPrefix( "external" )

#	N.B. I am making external records FULL RECORDS on the basis that the
#	pointers appear as integer tags.
ref = FullRecordClassGenerator( sysconsts, "InputStream", []  )
ref.options = MethodOptions( "r" )
ref.generateFilesWithPrefix( "inputstream" )

ref = FullRecordClassGenerator( sysconsts, "OutputStream", [] )
ref.options = MethodOptions( "r" )
ref.generateFilesWithPrefix( "outputstream" )

ref = FullRecordClassGenerator( sysconsts, "BigInt", [] )
ref.options = MethodOptions( "r" )
ref.generateFilesWithPrefix( "bigint" )

ref = FullRecordClassGenerator( sysconsts, "Rational", [] )
ref.options = MethodOptions( "r" )
ref.generateFilesWithPrefix( "rational" )

ref = WordRecordClassGenerator( sysconsts, "VirtualMachine", [] )
ref.options = MethodOptions( "r" )
ref.generateFilesWithPrefix( "virtualmachine" )

#	Create the insert for the sysMap table.
inc = open( "sysmap.inc.auto", 'w' )
sysconsts.generateSysMapInclude( inc )
inc.close()

#   Generate the outputs for the common2gnx translator.
inc = open( "sysconsts.inc.auto", 'w' )
sysconsts.generateSysConstTableInclude( inc )
inc.close()

#	Generate the unified include files.
inc = open( "datatypes.cpp.auto", 'w' )
sysconsts.generateDatatypesCPP( inc )
inc.close()

#	Generate the unified include files.
inc = open( "datatypes.hpp.auto", 'w' )
sysconsts.generateDatatypesHPP( inc )
inc.close()
