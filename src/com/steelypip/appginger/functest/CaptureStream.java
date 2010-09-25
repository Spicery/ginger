package com.steelypip.appginger.functest;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

class CaptureStream extends Thread {
    private InputStream is;
    private StringBuilder out = new StringBuilder();
    private String reason = null;
    
    CaptureStream( InputStream is ) {
        this.is = is;
    }
    
    public void run() {
        InputStreamReader isr = new InputStreamReader(is);
        BufferedReader br = new BufferedReader( isr );
        
        for (;;) {
        	String line;
			try {
				line = br.readLine();
			} catch ( IOException e ) {
				this.reason = e.getMessage();
				return;
			}
        	if ( line == null ) break;
        	out.append( line );
        	out.append( '\n' );
        }
    }

	String readBuffer() {
		return this.out.toString();
	}
	
	boolean passed() {
		return this.reason == null;
	}
	
	String reason() {
		return this.reason;
	}
}