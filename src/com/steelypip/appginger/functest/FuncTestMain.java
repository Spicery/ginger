package com.steelypip.appginger.functest;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FilenameFilter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class FuncTestMain {
	
	public static void main( String[] args ) {
		new FuncTestMain().runMain( args );
	}

	private void runMain( String[] args ) {
		for ( String fname : args ) {
			this.runAppGingerFuncTests( fname );
		}
		//this.runCommon2GnxFuncTests();	
	}

	private static void tests( final File f, final List< FuncTest > test_list ) {
		if ( f.isDirectory() ) {
			for ( File sub : f.listFiles() ) {
				tests( sub, test_list );
			}
		} else if ( f.getName().endsWith( ".tests" ) ){
			try {
				FileFuncTestReader r =  new FileFuncTestReader( f );
				for (;;) {
					FuncTest t = r.readTest();
					if ( t == null ) break;
					test_list.add( t );
				}
			} catch ( IOException e ) {
				throw new RuntimeException( e );
			}			
		}
	}
	
	private List< FuncTest > tests( File home ) {
		List< FuncTest > test_list = new ArrayList< FuncTest >();
		tests( home, test_list );
		return test_list;
	}
	
	/*private void runCommon2GnxFuncTests() {
		List< FuncTest > tests = this.tests( new File( new File( new File( "c" ), "common2gnx" ), "functests" ) );
		//	To do
	}*/
	
	private boolean needNewLine = false;

	private void runAppGingerFuncTests( final String fname ) {
		//List< FuncTest > tests = this.tests( new File( new File( new File( "c" ), "appginger" ), "functests" ) );
		List< FuncTest > tests = this.tests( new File( fname ) );
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
		if ( needNewLine ) {
			System.out.println();
		} else {
			needNewLine = true;
		}
		System.out.format( "TEST SUMMARY for %s\n\t%s/%s, %s failures\n", fname, npasses, npasses+nfails, nfails );
		for ( FuncTest fail : failed ) {
			System.out.format( "Test '%s' failed\n", fail.name() );
			for ( String r : fail.reasons() ) {
				System.out.format( "\t%s\n", r );
			}
		}
	}

}
