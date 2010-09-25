package com.steelypip.appginger.functest;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.List;

public class SimpleFuncTest implements FuncTest {
	
	private final String title;
	private final String known_defect;
	private final String command;
	private final String input;
	private final String expected;
	private boolean passed = false;
	private final List< String > reasons = new ArrayList< String >(); 
	
	public SimpleFuncTest( String title, String known_defect, String command, String input, String expected ) {
		super();
		this.title = title;
		this.known_defect = known_defect;
		this.command = command;
		this.input = input;
		this.expected = expected.trim();
	}

	public String name() {
		return title;
	}
	
	public boolean hasKnownDefect() {
		return this.known_defect != null;
	}
	
	public String getKnownDefect() {
		return this.known_defect;
	}

	public boolean passed() {
		return this.passed;
	}

	public List< String > reasons() {
		return this.reasons;
	}

	public void run() {
		try {
			Process p = Runtime.getRuntime().exec( this.command ); //"../cpp/appginger -B" );
			
			OutputStream stdin = p.getOutputStream();
			InputStream stdout = p.getInputStream();
			InputStream stderr = p.getErrorStream();
		
			
			CaptureStream outcap = new CaptureStream( stdout );
			outcap.start();
			
			CaptureStream errcap = new CaptureStream( stderr );
			errcap.start();
									
			PrintWriter pw = new PrintWriter( new OutputStreamWriter( stdin ) );
			BufferedReader b = new BufferedReader( new StringReader( input ) );
			for (;;) {
				String line = b.readLine();
				if ( line == null ) break;
				pw.println( line );
			}
			pw.flush();
			pw.close();
			
			
			boolean might_pass = true;
			
			final int exit_value = p.waitFor();
			if ( exit_value != 0 ) {
				this.reasons.add( "Non-zero exit value " + exit_value );
				might_pass = false;
			}
			
			
			outcap.join();
			errcap.join();
			
			
			if ( !outcap.passed() ) {
				this.reasons.add( "stdout: " + outcap.reason() );
				might_pass = false;
			}
		
			if ( !errcap.passed() ) {
				this.reasons.add( "stderr: " + outcap.reason() );
				might_pass = false;
			}
		
			final String outstring = outcap.readBuffer();
			final String errstring = errcap.readBuffer();
			
			if ( errstring.length() != 0 ) {
				this.reasons.add( "error buff: " + errstring );
				might_pass = false;
			}
			
			{
				String actual = outstring.trim();
				if ( ! actual.equals( expected ) ) {
					this.reasons.add( "Unexpected output" );
					this.reasons.add( "  Actual:   <" + actual + ">" );
					this.reasons.add( "  Expected: <" + expected + ">" );
					might_pass = false;
				}
			}
			
			this.passed = might_pass;
			
		} catch ( Exception e ) {
			this.reasons.add( "exception: " + e );
		}	
	}

}
