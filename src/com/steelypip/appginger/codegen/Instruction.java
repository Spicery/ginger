package com.steelypip.appginger.codegen;

import java.io.PrintWriter;

public abstract class Instruction {
	
	private String name;
	private String type;
	
	public String getName() {
		if ( name == null ) throw new RuntimeException( "name not set" );
		return name;
	}
	
	public void setName( String name ) {
		if ( this.name != null ) throw new RuntimeException( "name already set" );
		this.name = name;
	}
	
	public String getType() {
		if ( this.type == null ) throw new RuntimeException( "type not set" );
		return type;
	}
	
	public void setType( String type ) {
		if ( this.type != null ) throw new RuntimeException( "type already set" );
		this.type = type;
	}
	
	public Instruction init( final String name, final String type ) {
		this.setName( name );
		this.setType( type );
		return this;
	}
	
	public abstract void generateCodeInstAction( final PrintWriter output );
	public abstract void generateCodeInstLabel( final PrintWriter output );

	public abstract void generateBody( final PrintWriter output, final int indents );
}
