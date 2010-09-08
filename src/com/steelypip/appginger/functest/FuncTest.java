package com.steelypip.appginger.functest;

import java.util.List;

public interface FuncTest {
	
	String name();
	boolean hasKnownDefect();
	String getKnownDefect();
	
	void run();
	
	boolean passed();

	List< String > reasons();

}
