package com.steelypip.appginger.functest;

import java.util.List;

public interface FuncTest {
	
	String name();
	
	void run();
	
	boolean passed();

	List< String > reasons();

}
