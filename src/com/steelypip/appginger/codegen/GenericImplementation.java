package com.steelypip.appginger.codegen;

import java.io.PrintWriter;

public class GenericImplementation extends Implementation {

	@Override
	public int getNumId() {
		throw new RuntimeException();
	}

	@Override
	public void generateInstructionSetNthCPP( PrintWriter output ) {
		throw new RuntimeException();
	}

	
}
