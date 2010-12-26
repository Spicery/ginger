package com.steelypip.appginger.std;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.URLDecoder;
import java.net.URLEncoder;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpression;
import javax.xml.xpath.XPathExpressionException;
import javax.xml.xpath.XPathFactory;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;


public class StdMain {


	static File appginger() {
		File dir = new File( System.getProperty( "user.dir" ) );
		dir = new File( dir, "apps" );
		dir = new File( dir, "appginger" );
		dir = new File( dir, "cpp" );
		File e = new File( dir, "appginger" );
		return e;	
	}
	
	static File ginger_std() {
		File dir = new File( System.getProperty( "user.dir" ) );
		dir = new File( dir, "projects" );
		dir = new File( dir, "standard_library" );
		dir = new File( dir, "ginger.common" );
		return dir;
		
	}
	
	/**
	 * Uses URL encoding but does so quite aggressively as it will be used as a file name.
	 * File names get substituted in Unix command line scripts, so can be used for injection
	 * attacks. This is a form of neutralisation.
	 * @param s String to encode
	 * @return URL encoded String
	 */
	static String encode( final String name ) {
		final StringBuilder b = new StringBuilder();
		for ( int i = 0; i < name.length(); i++ ) {
			char ch = name.charAt( i );
			if ( ch < 256 && ( Character.isDigit( ch ) || Character.isLetter( ch ) || "_-".indexOf( ch ) >= 0 ) ) {
				b.append( ch );
			} else {
				b.append( '%' );
				b.append( Integer.toHexString( ch ) );
			}
		}
		return b.toString();
	}
	
	public static void main( final String[] args ) throws ParserConfigurationException, IOException, SAXException, XPathExpressionException {		
		 
		System.out.println( appginger() );
		Process p = Runtime.getRuntime().exec( new String[] { appginger().toString(), "-M" } );
		InputStream stdout = p.getInputStream();
		
		DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
		dbf.setValidating( false );
		dbf.setFeature( "http://xml.org/sax/features/namespaces", false );
		dbf.setFeature( "http://xml.org/sax/features/validation", false );
		dbf.setFeature( "http://apache.org/xml/features/nonvalidating/load-dtd-grammar", false );
		dbf.setFeature( "http://apache.org/xml/features/nonvalidating/load-external-dtd", false );
		DocumentBuilder db = dbf.newDocumentBuilder();
		Document doc = db.parse( stdout );

        //doc.getDocumentElement().normalize();
        //System.out.println( "Root element " + doc.getDocumentElement().getNodeName() );
        
        XPathFactory factory = XPathFactory.newInstance();
        XPath xpath = factory.newXPath();
        XPathExpression expr = xpath.compile( "//appginger/std/sysfn" );
        NodeList nodes = (NodeList)expr.evaluate(doc, XPathConstants.NODESET);
        
        //System.out.println( "#Sysfns = " + nodes.getLength() );
        
        final File dir = ginger_std();
        dir.mkdirs();
        
        for ( int i = 0; i < nodes.getLength(); i++ ) {
        	final Element e = (Element)nodes.item( i );
        	final String name = e.getAttribute( "name" );
        	final String ename = encode( name );
        	
        	
        	//System.out.format( "%s\t%s\t%s\t%s\n", name, enc_name, encode( name ), d );
        	
        	final File f = new File( dir, ename + ".sysfn" );
        	f.createNewFile();
            
        	//System.out.println( enc_name + ", " + name );
            //System.out.println( URLEncoder.encode( "... ", "UTF-8" ) );
        }
	}

}
