package com.steelypip.appginger.codegen;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;

public abstract class DataClassGenerator {

	protected final String dataKeyRoot;
	private final LogCreated log;
	
	public DataClassGenerator( final LogCreated sysconsts, final String className ) {
		this.dataKeyRoot = className;
		this.log = sysconsts;
	}
	
	public String keyName() {
		return "sys" + this.dataKeyRoot + "Key";
	}
	
	public String consName() {
		return "sysNew" + this.dataKeyRoot;
	}
	
	public String indexName() {
		return "sysIndex" + this.dataKeyRoot;
	}
	
	public abstract void generate( final PrintWriter cpp, final PrintWriter hpp );
	
	public void generate( final File outputFolder, final String root ) throws IOException {
		final File cppfile = new File( outputFolder, root + ".cpp.auto" );
		final File hppfile = new File( outputFolder, root +".hpp.auto" );
		final PrintWriter cpp = new PrintWriter( new FileWriter( cppfile ) );
		final PrintWriter hpp = new PrintWriter( new FileWriter( hppfile ) );
		this.generate( cpp, hpp );
		cpp.close();
		hpp.close();
		this.log.addCppFile( cppfile );
		this.log.addHppFile( hppfile );
	}
	
	public void addSysConst( final String gname, final Arity a, final String cname ) {
		this.log.addSysConst( gname, a, cname );
	}
	
	public void addSysConst( final String gname, final int a, final String cname ) {
		this.log.addSysConst( gname, new Arity( a ), cname );
	}
	
	protected void generateRecogniser( final PrintWriter cpp, final PrintWriter hpp ) {
		final String recogniserName = "sysIs" + this.dataKeyRoot;
		cpp.format( "Ref * %s( Ref * pc, class MachineClass * vm ) {\n", recogniserName );
		cpp.format( "    if ( vm->count == 1 ) {\n" );
		cpp.format( "        Ref r = vm->fastPeek();\n" );
		cpp.format( "        vm->fastPeek() = IsObj( r ) && ( *RefToPtr4( r ) == %s ) ? sys_true : sys_false;\n", this.keyName() );
		cpp.format( "        return pc;\n" );
		cpp.format( "    } else {\n" );
		cpp.format( "        throw Mishap( \"Wrong number of arguments for head\" );\n" );
		cpp.format( "    }\n" );
		cpp.format( "}\n\n" );
		
		hpp.format( "extern Ref * %s( Ref * pc, MachineClass * vm );\n", recogniserName );
		this.addSysConst( "is" + this.dataKeyRoot, 1, recogniserName );
	}

	public static String capitalise( final String field ) { 
		return field.substring( 0, 1 ).toUpperCase() + field.substring( 1 );
	}


}
