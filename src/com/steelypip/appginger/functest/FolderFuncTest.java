package com.steelypip.appginger.functest;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.List;



public class FolderFuncTest implements FuncTest {
	
	private final File home;
	private boolean pass = false;
	private List< String > reason_list = new ArrayList< String >();

	public FolderFuncTest( final File home ) {
		super();
		this.home = home;
	}

	public String name() {
		final String fname = this.home.getName();
		final String tname = fname.substring( 0, fname.indexOf( '.' ) );
		return tname;
	}

	public String getKnownDefect() {
		return null;
	}

	public boolean hasKnownDefect() {
		return false;
	}

	public boolean passed() {
		return this.pass;
	}

	public void run() {
		try {
			Process p = Runtime.getRuntime().exec( "c/appginger/cpp/appginger -B" );
			
			OutputStream stdin = p.getOutputStream();
			InputStream stdout = p.getInputStream();
			InputStream stderr = p.getErrorStream();
		
			
			CaptureStream outcap = new CaptureStream( stdout );
			outcap.start();
			
			CaptureStream errcap = new CaptureStream( stderr );
			errcap.start();
			
			File input = new File( this.home, "input.txt" );
			File expected = new File( this.home, "expected.txt" );
						
			PrintWriter pw = new PrintWriter( new OutputStreamWriter( stdin ) );
			BufferedReader b = new BufferedReader( new InputStreamReader( new FileInputStream( input ) ) );
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
				this.reason_list.add( "Non-zero exit value " + exit_value );
				might_pass = false;
			}
			
			
			outcap.join();
			errcap.join();
			
			
			if ( !outcap.passed() ) {
				this.reason_list.add( "stdout: " + outcap.reason() );
				might_pass = false;
			}
		
			if ( !errcap.passed() ) {
				this.reason_list.add( "stderr: " + outcap.reason() );
				might_pass = false;
			}
		
			final String outstring = outcap.readBuffer();
			final String errstring = errcap.readBuffer();
			
			if ( errstring.length() != 0 ) {
				this.reason_list.add( "error buff: " + errstring );
				might_pass = false;
			}
			
			final FileInputStream e = new FileInputStream( expected );
			final StringBuilder expbuffer = new StringBuilder();
			for (;;) {
				int ich = e.read();
				if ( ich < 0 ) break;
				expbuffer.append( (char)ich );
			}
			
			final String expstring = expbuffer.toString();
			if ( ! outstring.trim().equals( expstring.trim() ) ) {
				this.reason_list.add( "Unexpected output" );
				this.reason_list.add( "  Actual:   " + outstring );
				this.reason_list.add( "  Expected: " + expstring );
				might_pass = false;
			}
			
			this.pass = might_pass;
			
		} catch ( Exception e ) {
			this.reason_list.add( "exception: " + e );
		}
	}

	public List< String > reasons() {
		return this.reason_list;
	}
	
	
	
}
