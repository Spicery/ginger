package com.steelypip.appginger.functest;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FilenameFilter;
import java.io.IOException;
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
		List< FuncTest > test_list = new ArrayList< FuncTest >();
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
			if ( test.isDirectory() ) {
				test_list.add( new FolderFuncTest( test ) );
			} else if ( test.isFile() ) {
				try {
					FileFuncTestReader r =  new FileFuncTestReader( test );
					for (;;) {
						FuncTest t = r.readTest();
						if ( t == null ) break;
						test_list.add( t );
					}
				} catch ( IOException e ) {
					throw new RuntimeException( e );
				}
			} else {
				throw new RuntimeException();
			}
		}
		return test_list;
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
