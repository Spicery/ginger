package com.steelypip.appginger.functest;

import java.io.File;
import java.io.FilenameFilter;
import java.util.ArrayList;
import java.util.List;

public class FuncTestMain {
	
	public static void main( String[] args ) {
		new FuncTestMain().run( args );
	}

	private void run( String[] args ) {
		this.runAppGingerFuncTests();
		this.runCommon2GnxFuncTests();	
	}

	private List< FuncTest > tests( File home ) {
		List< FuncTest > t = new ArrayList< FuncTest >();
		for ( 
			File test :
			home.listFiles(
				new FilenameFilter() {
					public boolean accept( File dir, String name ) {
						return name.endsWith( ".test" );
					}
				}
			)
		) {
			t.add( new FolderFuncTest( test ) );
		}
		return t;
	}
	
	private void runCommon2GnxFuncTests() {
		List< FuncTest > tests = this.tests( new File( new File( new File( "c" ), "common2gnx" ), "functests" ) );
		//	To do
	}

	private void runAppGingerFuncTests() {
		List< FuncTest > tests = this.tests( new File( new File( new File( "c" ), "appginger" ), "functests" ) );
		List< FuncTest > failed = new ArrayList< FuncTest >();
		int npasses = 0;
		int nfails = 0;
		for ( FuncTest ft : tests ) {
			ft.run();
			if ( ft.passed() ) {
				npasses += 1;
			} else {
				nfails += 1;
				failed.add(  ft );
			}
		}
		System.out.format( "%s/%s, %s failures\n", npasses, npasses+nfails, nfails );
		for ( FuncTest fail : failed ) {
			System.out.format( "Test '%s' failed\n", fail.name() );
			for ( String r : fail.reasons() ) {
				System.out.format( "\t%s\n", r );
			}
		}
	}

}
