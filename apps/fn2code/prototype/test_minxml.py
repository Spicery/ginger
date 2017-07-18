#!/usr/bin/python3

from minxml import *

def testBasic():
    x = MinXML( "xxx" )
    assert "xxx" == x.getName()
    assert 0 == len( x.getAttributes() )
    x.put( "alpha", "001" )
    assert 1 == len( x.getAttributes() )
    assert "001" == x.get( "alpha" )
    assert not( x )
    assert 0 == len( x ) 
    x.add( MinXML( "yyy" ) )
    x.add( MinXML( "zzz" ) )
    assert 2 == len( x )
    assert "yyy" == x[0].getName()
    
def testPrintingEmpty():
    x = MinXML( "xxx" );
    assert "<xxx/>" == str( x )

def testPrintingAttributes():
    x = MinXML( "xxx" )
    x.put( "alpha", "001" )
    x.put( "beta", "002" )
    assert "<xxx alpha=\"001\" beta=\"002\"/>" == str( x )

def testPrintingNonAscii():
    x = MinXML( "xxx" )
    x.put( "nonascii", "\u00FF" )
    assert "<xxx nonascii=\"&#255;\"/>" == str( x )

def testPrintingSpecialCharacters():
    x = MinXML( "xxx" )
    x.put( "special", "<>&'\"" )
    assert "<xxx special=\"&lt;&gt;&amp;&apos;&quot;\"/>" == str( x )

def testPrintingChildren():
    x = MinXML( "xxx" )
    x.add( MinXML( "yyy" ) )
    x.add( MinXML( "zzz" ) )
    assert "<xxx><yyy/><zzz/></xxx>" == str( x )
