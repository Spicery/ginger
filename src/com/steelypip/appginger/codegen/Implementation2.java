package com.steelypip.appginger.codegen;

import java.io.PrintWriter;

public class Implementation2 extends Implementation {


	@Override
	public int getNumId() {
		return 2;
	}

	@Override
	public void generateInstructionSetNthCPP( PrintWriter output ) {
		for ( Instruction inst : this.instructions() ) {
			output.format( "static void spcfn_%s( void ) {\n", inst.getName() );
			inst.generateBody( output, 1 );
			output.format( "}\n\n", inst.getName() );
			//output.format( "Special spc_%s = spcfn_%s;\n\n", inst.getName(), inst.getName() );
		}
		
		final int num = this.getNumId();
		output.format( "InstructionSet%s::InstructionSet%s() {\n", num, num );
		for ( Instruction inst : this.instructions() ) {
			output.format( "    this->spc_%s = (Ref)spcfn_%s;\n", inst.getName(), inst.getName() );
		}
		output.format( "}\n\n" );		

	}

	
}
