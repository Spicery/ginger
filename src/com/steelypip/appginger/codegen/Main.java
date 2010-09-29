package com.steelypip.appginger.codegen;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.List;

public class Main {

	public static void main( String[] args ) throws IOException {
		Main main = new Main();
		main.automachine();
		main.autosys();
	}

	public void autosys() throws IOException {
		final File aFolder = new File( new File( "c" ), "automatic" );
		final File outputFolder = new File( aFolder, "sys" );
		final LogCreated sysconsts = new LogCreated();
				
		new FullRecordClassGenerator(
			sysconsts, "Pair", "head", "tail"
		).generate( outputFolder, "pair" );
		
		new FullRecordClassGenerator(
			sysconsts, "Maplet", "mapletKey", "mapletValue"
		).generate( outputFolder, "maplet" );
				
		new FullVectorClassGenerator(
			sysconsts, "Vector"
		).generate( outputFolder, "vector" );
		
		//	Generate the unified CPP and unified HPP include files.
		{
			
		}
		
		//	Create the insert for the sysMap table.
		{
			final PrintWriter inc = new PrintWriter( new FileWriter( new File( outputFolder, "sysmap.inc.auto" ) ) );
			sysconsts.generateSysMapInclude( inc );
			inc.close();
		}
		
		//	Generate the outputs for the common2gnx translator.
		{
			final PrintWriter inc = new PrintWriter( new FileWriter( new File( outputFolder, "sysconsts.inc.auto" ) ) );
			sysconsts.generateSysConstTableInclude( inc );
			inc.close();
		}
		
		//	Generate the unified include files.
		{
			final PrintWriter inc = new PrintWriter( new FileWriter( new File( outputFolder, "datatypes.cpp.auto" ) ) );
			sysconsts.generateDatatypesCPP( inc );
			inc.close();
		}
		
		//	Generate the unified include files.
		{
			final PrintWriter inc = new PrintWriter( new FileWriter( new File( outputFolder, "datatypes.hpp.auto" ) ) );
			sysconsts.generateDatatypesHPP( inc );
			inc.close();
		}
		
	}
	
	/**
	 * When this runs, it must build the C++ class files for
	 * the different implementations of the virtual
	 * machine.
	 * @throws IOException 
	 */
	public void automachine() throws IOException {
		final File aFolder = new File( new File( "c" ), "automatic" );
		final File outputFolder = new File( aFolder, "machine" );
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
