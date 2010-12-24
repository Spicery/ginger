package com.steelypip.appginger.std;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;

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

        doc.getDocumentElement().normalize();
        System.out.println( "Root element " + doc.getDocumentElement().getNodeName() );
        
        XPathFactory factory = XPathFactory.newInstance();
        XPath xpath = factory.newXPath();
        XPathExpression expr = xpath.compile( "//appginger/std/sysfn" );
        NodeList nodes = (NodeList)expr.evaluate(doc, XPathConstants.NODESET);
        
        System.out.println( "#Sysfns = " + nodes.getLength() );
        
        for ( int i = 0; i < nodes.getLength(); i++ ) {
        	Element e = (Element)nodes.item( i );
            System.out.println( e.getAttribute( "name" ) ); 
        }
	}

}
