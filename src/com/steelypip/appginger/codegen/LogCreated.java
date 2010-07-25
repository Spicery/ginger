package com.steelypip.appginger.codegen;

import java.io.File;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class LogCreated {
	
	final Map< String, SysInfo > mapping = new HashMap< String, SysInfo >();
	
	public void addSysConst( final String ginger_name, final Arity in_arity, final Arity out_arity, final String internal_name ) {
		final SysInfo nm = this.mapping.get( ginger_name );
		if ( nm == null ) {
			this.mapping.put( ginger_name, new SysInfo( internal_name, in_arity, out_arity ) );
		} else { 
			throw new RuntimeException( "Repeated name" );
		}
	}

	
	private final List< File > cpp_file_list = new ArrayList< File >();
	
	public void addCppFile( final File name ) {
		this.cpp_file_list.add(  name  );
	}
	
	private final List< File > hpp_file_list = new ArrayList< File >();
	
	public void addHppFile( final File name ) {
		this.hpp_file_list.add( name );
	}

	public void generateSysMapInclude( final PrintWriter inc ) {
		for ( String ginger_name : mapping.keySet() ) {
			SysInfo p = this.mapping.get( ginger_name );
			inc.format( 
				"SysMap::value_type( \"%s\", SysInfo( fnc_syscall, %s, %s, %s ) ),\n",
				ginger_name,
				p.getInArity(),
				p.getOutArity(),
				p.getName()
			);
		}
	}

	public void generateSysConstTableInclude( PrintWriter inc ) {
		for ( String ginger_name : mapping.keySet() ) {
			SysInfo p = this.mapping.get( ginger_name );
			inc.format( 
				"this->table[ \"%s\" ] = new SysConst( \"sysfn\", \"%s\" );\n",
				ginger_name,
				ginger_name
			);
		}
	}

	private void includes( PrintWriter inc, List< File > files ) {
		for ( File f : files ) {
			inc.format( "#include \"%s\"\n", f.getName() );
		}
	}

	public void generateDatatypesCPP( PrintWriter inc ) {
		this.includes( inc, this.cpp_file_list );
	}

	public void generateDatatypesHPP( PrintWriter inc ) {
		this.includes( inc, this.hpp_file_list );
	}

	
}
