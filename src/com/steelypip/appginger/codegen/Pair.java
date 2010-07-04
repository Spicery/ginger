package com.steelypip.appginger.codegen;

public class Pair< T1, T2 > {

	final private T1 left;
	final private T2 right;
	
	public Pair( T1 left, T2 right ) {
		super();
		this.left = left;
		this.right = right;
	}
	
	public T1 getLeft() {
		return left;
	}
	public T2 getRight() {
		return right;
	}
	
	
	
}
