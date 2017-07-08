#!/usr/bin/python3

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
	elif isinstance( a, tuple ):
		return Arity( a[0], more=a[1] )
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
#   Create maths functions
################################################################################

MATHS_SYS_FN = [
	( 'truncate', 1, 1, "Rounds x toward zero, returning the nearest integral value that is not larger in magnitude than x." ),
	( 'round', 1, 1, "Returns the integral value that is nearest to x, with halfway cases rounded away from zero." ),
	( 'floor', 1, 1, "Rounds x downward, returning the largest integral value that is not greater than x." ),
	( 'ceiling', 1, 1, "Returns the ceiling of a real value." ),
	( 'erfc', 1, 1, "Returns the complementary error function value for a real value." ),
	( 'erf', 1, 1, "Returns the error function value for a real value." ),
	( 'lgamma', 1, 1, "Returns the natural logarithm of the absolute value of gamma of a real value." ),
	( 'tgamma', 1, 1, "Returns the gamma function of a real value." ),
	( 'sqrt', 1, 1, "Returns the square root of a real value." ),
	( 'cbrt', 1, 1, "Returns the cube root of a real value." ),
	( 'log', 1, 1, "Returns the natural logarithm of a real value." ),
	( 'log2', 1, 1, "Returns the logarithm to base 2 of a real value." ),
	( 'log10', 1, 1, "Returns the logarithm to base 10 of a real value." ),
	( 'hypot', 2, 1, "hypot(x, y) returns sqrt(x*x + y*y)." ),
	( 'exp', 1, 1, "Returns the exponential of a real value (e**x)." ),
	( 'exp2', 1, 1, "Returns the two to the power of a real value (2**x)." ),
	( 'sin', 1, 1, "Returns sine of an angle in radians." ),
	( 'cos', 1, 1, "Returns cosine of an angle in radians." ),
	( 'tan', 1, 1, "Returns tangent of an angle in radians." ),
	( 'asin', 1, 1, "Returns arcsine in radians." ),
	( 'acos', 1, 1, "Returns arccosine in radians." ),
	( 'atan', 1, 1, "Returns arctangent in radians." ),
	( 'atan2', 2, 1, "Returns arctan2( y, x ) in radians." ),
	( 'sinh', 1, 1, "Returns sinh, the hyperbolic sine." ),
	( 'cosh', 1, 1, "Returns sinh, the hyperbolic cosine." ),
	( 'tanh', 1, 1, "Returns sinh, the hyperbolic tangent." ),
	( 'asinh', 1, 1, "Returns asinh, the inverse of the hyperbolic sine." ),
	( 'acosh', 1, 1, "Returns asinh, the inverse of the hyperbolic cosine." ),
	( 'atanh', 1, 1, "Returns asinh, the inverse of the hyperbolic tangent." )
]

def invoke( arity_args, arity_rtn ):
	if arity_args == 1 and arity_rtn == 1:
		return 	[ '    vm->fastPeek() = vm->heap().copyDouble( pc, refToDouble( vm->fastPeek() ).{name}().asDouble() );' ]
	elif arity_args == 2 and arity_rtn == 1:
		return [
			'    TransDouble rhs = refToDouble( vm->fastPop() );'
			'    TransDouble result = refToDouble( vm->fastPeek() ).{name}( rhs ).asDouble();',
			'    vm->fastPeek() = vm->heap().copyDouble( pc, result );'
		]
	else:
		raise Exception( 'This combination of arities not yet defined' )


def generateMathSysFn( sysfn_name, arity_args, arity_rtn, doc_string ):
	return(
		'\n'.join(
			[
				'Ref * sys_{name}( Ref * pc, class MachineClass * vm ) {{',
				'    if ( vm->count != {args} ) throw Ginger::Mishap( "ArgsMismatch" );',
			] + 
			invoke( arity_args, arity_rtn ) +
			[
				'    return pc;',
				'}}',
				'SysInfo info_{name}(',
				'    FullName( "{name}" ),',
				'    Ginger::Arity( {args} ),',
				'    Ginger::Arity( {rtn} ),',
				'    sys_{name},',
				'    "{doc}"',
				');'
			]
		).format( name=sysfn_name, args=arity_args, rtn=arity_rtn, doc=doc_string )
	)

if __name__ == "__main__":
	with open( 'sysmaths.cpp.auto', 'w' ) as output:
		for sys_fn in MATHS_SYS_FN:
			print( generateMathSysFn( *sys_fn ), file=output )
