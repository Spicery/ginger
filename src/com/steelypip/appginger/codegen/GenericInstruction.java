package com.steelypip.appginger.codegen;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.PrintWriter;

public class GenericInstruction extends Instruction {

	@Override
	public void generateCodeInstAction( PrintWriter output ) {
		throw new RuntimeException();
	}

	@Override
	public void generateCodeInstLabel( PrintWriter output ) {
		throw new RuntimeException();
	}
	
	@Override
	public void generateBody( PrintWriter output, final int indents ) {
		final File instFolder = new File( "instruction_set" );
		final File instFile = new File( instFolder, this.getName() + "." + this.getType() );
		try {
			final BufferedReader r = new BufferedReader( new FileReader( instFile ) );
			for (;;) {
				final String line = r.readLine();
				if ( line == null ) break;
				for ( int i = 0; i < indents; i++ ) {
					output.print(  "    " );
				}
				output.println( line );
			}
		} catch ( IOException e ) {
			throw new RuntimeException();
		}
	}
	
	
	
}
