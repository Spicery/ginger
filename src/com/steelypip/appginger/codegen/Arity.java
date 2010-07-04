package com.steelypip.appginger.codegen;

public class Arity {
	
	public static final Arity MANY = new Arity( 0, true );
	final int count;
	final boolean more;
	
	public Arity( int count, boolean more ) {
		super();
		this.count = count;
		this.more = more;
	}
	
	public Arity( int count ) {
		this( count, false );
	}
	
	public String toString() {
		return String.format( "Arity(%s,%s)", this.count, this.more );
	}

}
