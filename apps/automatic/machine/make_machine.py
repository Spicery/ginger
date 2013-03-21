#!/usr/bin/python

import os

################################################################################
#    Instruction
################################################################################

class Instruction:

	def __init__( self, name, type ):
		self.name = name
		self.type = type

	def getName( self ):
		return self.name

	def getType( self ):
		return self.type

	def getFileName( self ):
		return self.getName() + "." + self.getType()

	def generateBody( self, output, indents ):
		instFile = os.path.join( "../../../instruction_set",  self.getFileName() )
		r = open( instFile, 'r' )
		while True:
			line = r.readline()
			if not line:
				break
			for i in range( indents ):
				output.write( "    " )
			output.write( line )
		r.close()

################################################################################
#    Instruction Set
################################################################################

instructionSet = [
	Instruction( "add", "i" ),
	Instruction( "decr", "i" ),
	Instruction( "div", "i" ),
	Instruction( "eq", "i" ),
	#Instruction( "eqq", "ir" ),
	Instruction( "neq", "i" ),
	Instruction( "gt", "i" ),
	Instruction( "gte","i" ),
	Instruction( "incr", "i" ),
	Instruction( "incr_by", "ir" ),
	Instruction( "lt", "i" ),
	Instruction( "lte", "i" ),
	Instruction( "mul", "i" ),
	Instruction( "sub", "i" ),
	Instruction( "calls", "i" ),
	Instruction( "chainlite", "icr" ) ,
	Instruction( "check_count", "ir" ),
	Instruction( "check_mark0", "ir" ),
	Instruction( "check_mark1", "ir" ),
	Instruction( "end_mark", "ir" ),
	Instruction( "end1_calls", "ir" ),
	Instruction( "end_call_global", "irv" ),
	Instruction( "enter", "i" ),
	Instruction( "enter0", "i" ),
	Instruction( "enter1", "i" ),
	Instruction( "eq_si", "irrr" ),
	Instruction( "eq_ss", "irrr" ),
	Instruction( "erase", "i" ),
	Instruction( "erase_mark", "ir" ),
	Instruction( "field", "ir" ),
	Instruction( "getiterator", "i" ),
	Instruction( "goto", "ir" ),
	Instruction( "gt_si", "irrr" ),
	Instruction( "gt_ss", "irrr" ),
	Instruction( "gte_si", "irrr" ),
	Instruction( "gte_ss", "irrr" ),
	Instruction( "halt", "i" ),
	Instruction( "ifnot", "ir" ),
	Instruction( "ifso", "ir" ),
	Instruction( "and", "ir" ),
	Instruction( "or", "ir" ),
	Instruction( "absand", "ir" ),
	Instruction( "absor", "ir" ),
	Instruction( "invoke", "icc" ),
	Instruction( "listiterate", "i" ),
	Instruction( "lt_si", "irrr" ),
	Instruction( "lt_ss", "irrr" ),
	Instruction( "lte_si", "irrr" ),
	Instruction( "lte_ss", "irrr" ),
	Instruction( "mod", "i" ),
	Instruction( "neq_si", "irrr" ),
	Instruction( "neq_ss", "irrr" ),
	Instruction( "neg", "i" ),
	Instruction( "not", "i" ),
	Instruction( "pop_global", "iv" ),
	Instruction( "pop_local", "ir"),
	Instruction( "push_global", "iv" ),
	Instruction( "push_local", "ir" ),
	Instruction( "push_local0", "i" ),
	Instruction( "push_local1", "i" ),
	Instruction( "pushq", "ic" ),
	Instruction( "push_local_ret", "ir" ),
	Instruction( "push_local0_ret", "i" ),
	Instruction( "push_local1_ret", "i" ),
	Instruction( "pushq_ret", "ic" ),
	Instruction( "quo", "i" ),
	Instruction( "reset", "i" ),
	Instruction( "return", "i" ),
	Instruction( "return_ifso", "i" ),
	Instruction( "return_ifnot", "i" ),
	Instruction( "self_call", "i" ),
	Instruction( "self_call_n", "ir" ),
	Instruction( "self_constant", "i" ),
	Instruction( "set", "ir" ),
	Instruction( "set_call_global", "irv" ),
	Instruction( "set_calls", "ir" ),
	Instruction( "set_syscall", "irr" ),
	Instruction( "start_mark", "ir" ),
	Instruction( "stringiterate", "i" ),
	Instruction( "syscall", "ir" ),
	Instruction( "syscall_arg", "irc" ),
	Instruction( "syscall_dat", "irr" ),
	Instruction( "syscall_argdat", "ircr" ),
	Instruction( "sysreturn", "i" ),
	Instruction( "vectoriterate", "i" ),
	Instruction( "mixediterate", "i" )
]

################################################################################
#    Implementation
################################################################################

class Implementation( object ):

	def generateInstructionSetHPP( self, output ):
		for inst in instructionSet:
			output.write( "	Ref spc_{};\n".format( inst.getName() ) )
	
	def generateInstructionHPP( self, output ):
		sep = "";
		for inst in instructionSet:
			name = inst.getName()
			output.write( "{}    vmc_{}".format( sep, name ) )
			sep = ",\n"
		output.write( "\n" )
	
	def generateInstructionSetCPP( self, output ):
		output.write( "const char * InstructionSet::name( Ref spc ) const {\n" );
		output.write( "    return(\n" );
		for inst in instructionSet:
			name = inst.getName()
			output.write(  "        spc == spc_{0} ? \"{0}\" :\n".format( name ) )
		output.write( "        \"?\"\n" )
		output.write( "    );\n" )
		output.write( "}\n\n" )

		output.write( "Instruction InstructionSet::findInstruction( const std::string & name ) const {\n" )
		for inst in instructionSet:
			iname = inst.getName();
			output.write( "    if ( name == \"{0}\" ) return vmc_{0};\n".format( iname, ) );
		output.write( "    throw Mishap( \"Bad instruction name\" );\n" )
		output.write( "}\n\n" ) 
		
		output.write(  "const char * InstructionSet::signature( Ref x ) const {\n" )
		for inst in instructionSet:
			if "i" != inst.getType():
				output.write( "    if ( x == spc_{} ) return \"{}\";\n".format( inst.getName(), inst.getType() ) )
		output.write( "    return \"i\";\n" )
		output.write( "}\n\n" );
	
		output.write( "Ref InstructionSet::lookup( Instruction instr ) const {\n" );
		output.write( "    switch ( instr ) {\n" );
		for inst in instructionSet:
			iname = inst.getName();
			output.write( "    case vmc_{0}: return this->spc_{0};\n".format( iname ) )
		output.write( "    }\n" )
		output.write( "    throw Mishap( \"Bad instruction\" );\n" )
		output.write( "}\n\n" )
	
	def generateImplementationCPP( self, output ):
		# Do nothing.
		return
	
	def generateInstructionSetNthCPP( self, output ):
		raise Exception( "Not overridden" )

	def getNumId( self ):
		"""Returns the implementation number"""
		raise Exception( "Not overridden" )

################################################################################
#    GenericImplementation
################################################################################

class GenericImplementation( Implementation ):
	
	def getNumId( self ):
		"""Returns the implementation number"""
		raise Exception( "GENERIC" )


################################################################################
#    Implementation1
################################################################################

class Implementation1( Implementation ):

	def getNumId( self ):
		return 1

	def generateInstructionSetNthCPP( self, output ):
		for inst in instructionSet:
			output.write( "static Ref *spcfn_{}( Ref *pc, Machine vm ) {{\n".format( inst.getName() ) )
			inst.generateBody( output, 1 )
			output.write( "}\n\n" )
		
		output.write( "InstructionSet{0}::InstructionSet{0}() {{\n".format( self.getNumId() ) )
		for inst in instructionSet:
			output.write( "    this->spc_{0} = (Ref)spcfn_{0};\n".format( inst.getName() ) )
		output.write( "}\n\n" )

################################################################################
#    Implementation2
################################################################################

class Implementation2( Implementation ):

	def getNumId( self ):
		return 2

	def generateInstructionSetNthCPP( self, output ):
		for inst in instructionSet:
			output.write( "static void spcfn_{}( void ) {{\n".format( inst.getName() ) )
			inst.generateBody( output, 1 )
			output.write( "}\n\n" )
		
		output.write( "InstructionSet{0}::InstructionSet{0}() {{\n".format( self.getNumId() ) )
		for inst in instructionSet:
			output.write( "    this->spc_{0} = (Ref)spcfn_{0};\n".format( inst.getName() ) )
		output.write( "}\n\n" )

################################################################################
#    Implementation3
################################################################################

class Implementation3( Implementation ):

	def getNumId( self ):
		return 3

	def generateInstructionSetNthCPP( self, output ):
		# Nothing.
		return


	def generateImplementationCPP( self, output ):
		output.write( "void Machine3::core( bool init_mode, Ref *pc ) {\n" )
		output.write( "    Ref *VMSP, *VMVP, *VMLINK;\n" )
		output.write( "    if ( init_mode ) goto Initialize;\n" )
		output.write( "    MELT;\n" )
		output.write( "    goto **pc;\n" )
		
		for inst in instructionSet:
			output.write(  "    L_{}: {{\n".format( inst.getName() ) )
			inst.generateBody( output, 2 )
			output.write( "    }\n" )
			
		output.write(  "    Initialize: {\n" )
		output.write(  "        InstructionSet & ins = vm->instruction_set;\n" )
		for inst in instructionSet:
			output.write(  "        ins.spc_{0} = &&L_{0};\n".format( inst.getName() ) )

		output.write( "        return;\n" )
		output.write( "    }\n" )
		output.write( "}\n" )	

################################################################################
#    Implementation4
################################################################################

class Implementation4( Implementation ):

	def getNumId( self ):
		return 4

	def generateImplementationCPP( self, output ):
		for inst in instructionSet:
			output.write( "case vmc_{}: {{\n".format( inst.getName() ) )
			inst.generateBody( output, 1 )
			output.write( "}; break;\n\n" )

	# Not 100% sure if we need this.
	def generateInstructionSetNthCPP( self, output ):
		for inst in instructionSet:
			name = inst.getName();
			output.write( "    this->spc_{0} = (Ref)vmc_{0};\n".format( name ) )


################################################################################
#    All Implementations
################################################################################

implementations = [
	Implementation1(),
	Implementation2(),
	Implementation3(),
	Implementation4()
]

################################################################################
#    Main
################################################################################

pw = open( "instruction_set.xdef.auto", 'w' )
for inst in instructionSet:
	pw.write( 'X( vmc_{0}, "{0}", "{1}" )\n'.format( inst.getName(), inst.getType() ) )
pw.close()

generic = GenericImplementation()
pw = open( "instruction_set.hpp.auto", 'w' )
generic.generateInstructionSetHPP( pw )
pw.close()

pw = open( "instruction_set.cpp.auto", 'w' )
generic.generateInstructionSetCPP( pw )
pw.close()

pw = open( "instruction.hpp.auto", 'w' )
generic.generateInstructionHPP( pw )
pw.close()

for impl in implementations:
	fname = "instruction_set{}.cpp.auto".format( impl.getNumId() )
	pw = open( fname, 'w' )
	impl.generateInstructionSetNthCPP( pw )
	pw.close()

for impl in implementations:
	fname = "machine{}.cpp.auto".format( impl.getNumId() )
	pw = open( fname, 'w' );
	impl.generateImplementationCPP( pw )
	pw.close()

