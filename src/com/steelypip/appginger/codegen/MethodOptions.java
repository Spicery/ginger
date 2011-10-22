package com.steelypip.appginger.codegen;

public class MethodOptions {

	private boolean gen_constructor = false;
	private boolean gen_deconstructor = false;
	private boolean gen_recogniser = false;
	private boolean gen_fields = false;
	private boolean gen_indexer = false;
	
	public MethodOptions() {
		this( "CDRIF" );
	}
	
	/**
	 * C = constructor
	 * D = deconstructor
	 * R = recogniser
	 * F = record fields
	 * I = vector indexer
	 */
	public MethodOptions( String opts ) {
		gen_constructor = ( opts.indexOf( 'C' ) != -1 );
		gen_deconstructor = ( opts.indexOf( 'D' ) != -1 );
		gen_recogniser = ( opts.indexOf( 'R' ) != -1 );
		gen_fields = ( opts.indexOf( 'F' ) != -1 );
		gen_indexer = ( opts.indexOf( 'I' ) != -1 );
		if ( opts.indexOf( 'A' ) != -1 ) throw new RuntimeException( "Bad option: " + opts );
	}

	public boolean isGenConstructor() {
		return gen_constructor;
	}

	public boolean isGenDeconstructor() {
		return gen_deconstructor;
	}

	public boolean isGenRecogniser() {
		return gen_recogniser;
	}

	public boolean isGenFields() {
		return gen_fields;
	}

	public boolean isGenIndexer() {
		return gen_indexer;
	}
	
	
	
}
