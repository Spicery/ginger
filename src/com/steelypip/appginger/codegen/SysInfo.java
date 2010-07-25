package com.steelypip.appginger.codegen;

public class SysInfo {
	
	private String name;
	private Arity in_arity;
	private Arity out_arity;

	public SysInfo( String name, Arity in_arity, Arity out_arity ) {
		super();
		this.name = name;
		this.in_arity = in_arity;
		this.out_arity = out_arity;
	}

	public String getName() {
		return name;
	}

	public Arity getInArity() {
		return in_arity;
	}

	public Arity getOutArity() {
		return out_arity;
	}

	
	
}
