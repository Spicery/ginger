package com.steelypip.appginger;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.List;

public class Main {

	public static void main( String[] args ) throws IOException {
		Main main = new Main();
//		main.run( args );
		main.automatic();
	}

	
	/**
	 * When this runs, it must build the C++ class files for
	 * the different implementations of the virtual
	 * machine.
	 * @throws IOException 
	 */
	public void automatic() throws IOException {
		final File cFolder = new File( "c" );
		final File outputFolder = new File( cFolder, "automatic" );
		if ( ! outputFolder.exists() ) {
			throw new RuntimeException( "Missing output folder" );
		}
		
		final Implementation generic = new GenericImplementation();
		{
			final PrintWriter pw = new PrintWriter( new FileWriter( new File( outputFolder, "instruction_set.hpp.auto" ) ) );
			generic.generateInstructionSetHPP( pw );
			pw.close();
		}
		{
			final PrintWriter pw = new PrintWriter( new FileWriter( new File( outputFolder, "instruction_set.cpp.auto" ) ) );
			generic.generateInstructionSetCPP( pw );
			pw.close();
		}
		{
			final PrintWriter pw = new PrintWriter( new FileWriter( new File( outputFolder, "instruction.hpp.auto" ) ) );		
			generic.generateInstructionHPP( pw );
			pw.close();
			
		}
		
		for ( Implementation impl : this.implementations() ) {
			final String fname = String.format( "instruction_set%s.cpp.auto", impl.getNumId() );
			final File out = new File( outputFolder, fname );
			final PrintWriter pw = new PrintWriter( new FileWriter( out ) );
			impl.generateInstructionSetNthCPP( pw );
			pw.close();
		}
		for ( Implementation impl : this.implementations() ) {
			final String fname = String.format( "machine%s.cpp.auto", impl.getNumId() );
			final File out = new File( outputFolder, fname );
			final PrintWriter pw = new PrintWriter( new FileWriter( out ) );
			impl.generateImplementationCPP( pw );
			pw.close();
		}
	}
	
	/**
	 * When this runs, it must build the C++ class files for
	 * the different implementations of the virtual
	 * machine.
	 * @throws IOException 
	 */
	private List< Implementation > implementations() {
		final List< Implementation > list = new ArrayList< Implementation >();
		list.add( new Implementation1() );
		list.add( new Implementation2() );
		list.add( new Implementation3() );
		return list;
	}

}
