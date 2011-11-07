package com.steelypip.appginger.functest;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.LineNumberReader;
import java.util.Iterator;
import java.util.List;

public class FileFuncTestReader {
	
	private final String command;
	private final File source;
	private LineNumberReader reader;
	
	public FileFuncTestReader( File source, final String command ) throws FileNotFoundException {
		super();
		this.source = source;
		this.command = command;
		this.reader = new LineNumberReader( new BufferedReader( new FileReader( source ) ) );
	}
	
	private static boolean isSeparator( char ch, final String s ) {
		if ( 16 < s.length() ) {
			for ( int i = 0; i < s.length(); i++ ) {
				if ( s.charAt( i ) != ch ) return false;
			}
			return true;
		} else {
			return false;
		}
	}
	
	private static boolean isMajorSeparator( final String s ) {
		return isSeparator( '=', s );
	}

	private static boolean isMinorSeparator( final String s ) {
		return isSeparator( '-', s );
	}
	

	public FuncTest readTest() throws IOException {
		
		String title = null;
		String known_defect = null;
		String input_string = null;
		String expected_string = null;

		//	Read introduction.
		final int nlines = this.reader.getLineNumber();
		for ( String line = this.reader.readLine(); line != null; line = this.reader.readLine() ) {
			if ( isMinorSeparator( line ) ) break;
			if ( isMajorSeparator( line ) ) return this.readTest();
			int split = line.indexOf( ':' );
			if ( split >= 0 ) {
				final String key = line.substring( 0, split );
				final String value = line.substring( split + 1 );
				if ( key.compareToIgnoreCase( "title" ) == 0 ) {
					title = value;
				} else if ( key.compareToIgnoreCase( "known" ) == 0 || key.compareToIgnoreCase( "exit" ) == 0) {
					known_defect = value;
				}
			}
 		}
		if ( nlines == this.reader.getLineNumber() ) return null;
		
		boolean at_end = false;
		
		//	Read input.
		final StringBuilder input = new StringBuilder();
		for ( String line = this.reader.readLine(); ; line = this.reader.readLine() ) {
			if ( line == null ) throw new RuntimeException( "Invalid test format (" + this.reader.getLineNumber() + ")" );
			if ( isMajorSeparator( line ) ) {
				at_end = true;
				break;
			}
			if ( isMinorSeparator( line ) ) break;
			input.append( line );
			input.append( '\n' );
		}
		input_string = input.toString();

		if ( ! at_end ) {	
			//	Read expected.
			StringBuilder expected = new StringBuilder();
			for ( String line = this.reader.readLine(); line != null; line = this.reader.readLine() ) {
				if ( line == null || isMinorSeparator( line ) ) {
					throw new RuntimeException( "Invalid test format (" + this.reader.getLineNumber() + ")" );
				}
				if ( isMajorSeparator( line ) ) break;
				expected.append( line );
				expected.append( '\n' );
			}
			expected_string = expected.toString();
		}
		
		return new SimpleFuncTest( title, known_defect, this.command, input_string, expected_string );
	}
		
}
