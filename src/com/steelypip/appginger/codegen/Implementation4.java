package com.steelypip.appginger.codegen;

import java.io.PrintWriter;

public class Implementation4 extends Implementation {
	
	@Override
	public int getNumId() {
		return 4;
	}

	@Override
	public void generateInstructionSetNthCPP( PrintWriter output ) {
		for ( Instruction inst : this.instructions() ) {
			final String name = inst.getName();
			output.format( "    this->spc_%s = (Ref)vmc_%s;\n", name, name );
		}
	}

	@Override
	void generateImplementationCPP( PrintWriter output ) {
		for ( Instruction inst : this.instructions() ) {
			output.format( "case vmc_%s: {\n", inst.getName() );
			inst.generateBody( output, 1 );
			output.format( "}; break;\n\n" );
		}
	}

}
