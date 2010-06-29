package com.steelypip.appginger.codegen;

import java.io.PrintWriter;

public class Implementation1 extends Implementation {
	
	@Override
	public int getNumId() {
		return 1;
	}

	@Override
	public void generateInstructionSetNthCPP( PrintWriter output ) {
		for ( Instruction inst : this.instructions() ) {
			output.format( "static Ref *spcfn_%s( Ref *pc, Machine vm ) {\n", inst.getName() );
			inst.generateBody( output, 1 );
			output.format( "}\n\n" );
		}
		
		final int num = this.getNumId();
		output.format( "InstructionSet%s::InstructionSet%s() {\n", num, num );
		for ( Instruction inst : this.instructions() ) {
			output.format( "    this->spc_%s = (Ref)spcfn_%s;\n", inst.getName(), inst.getName() );
		}
		output.format( "}\n\n" );		
	}


	

}
