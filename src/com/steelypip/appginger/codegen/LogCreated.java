package com.steelypip.appginger.codegen;

import java.io.File;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class LogCreated {
	
	final Map< String, Pair< Arity, String > > mapping = new HashMap< String, Pair< Arity, String > >();
	
	public void addSysConst( final String ginger_name, final Arity arity, final String internal_name ) {
		final Pair< Arity, String > nm = this.mapping.get( ginger_name );
		if ( nm == null ) {
			this.mapping.put( ginger_name, new Pair< Arity, String >( arity, internal_name ) );
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
			Pair< Arity, String > p = this.mapping.get( ginger_name );
			inc.format( 
				"SysMap::value_type( \"%s\", SysInfo( fnc_syscall, %s, %s ) ),\n",
				ginger_name,
				p.getLeft(),
				p.getRight()
			);
		}
	}

	public void generateSysConstTableInclude( PrintWriter inc ) {
		for ( String ginger_name : mapping.keySet() ) {
			Pair< Arity, String > p = this.mapping.get( ginger_name );
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
