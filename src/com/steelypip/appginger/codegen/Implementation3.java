package com.steelypip.appginger.codegen;

import java.io.PrintWriter;

public class Implementation3 extends Implementation {


	@Override
	public int getNumId() {
		return 3;
	}

	@Override
	public void generateInstructionSetNthCPP( PrintWriter output ) {
		//	Nothing.
	}

	@Override
	void generateImplementationCPP( PrintWriter output ) {
		output.print( "void Machine3::core( bool init_mode, Ref *pc ) {\n" );
		output.print( "    Ref *VMSP, *VMVP, *VMLINK;\n" );
		output.print( "    if ( init_mode ) goto Initialize;\n" );
		output.print( "    MELT;\n" );
		output.print( "    goto **pc;\n" );
		
		for ( Instruction inst : this.instructions() ) {
			output.format(  "    L_%s: {\n", inst.getName() );
			inst.generateBody( output, 2 );
			output.format( "    }\n" );
		}
			
		output.print(  "    Initialize: {\n" );
		output.print(  "        InstructionSet & ins = vm->instruction_set;\n" );
		for ( Instruction inst : this.instructions() ) {
			output.format(  "        ins.spc_%s = &&L_%s;\n", inst.getName(), inst.getName() );
		}

		output.print(  "        return;\n" );
		output.print( "    }\n" );
		output.print( "}\n" );		
	}

	
	
}
