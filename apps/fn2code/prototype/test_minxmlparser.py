#!/usr/bin/python3

from minxml import *
import io

def testEmpty():
    assert None == Parser( io.StringIO( "" ) ).readElement()

def testEmptyAsIterable():
    n = 0
    for m in Parser( io.StringIO( "" ) ):
        n += 1
    assert 0 == n

def testNonEmpty():
    m = Parser( io.StringIO( "<foo/>" ) ).readElement()
    assert m != None
    assert "foo" == m.getName()
    assert 0 == len( m.getAttributes() )
    assert m.isEmpty()
    
def testAttributesEitherQuote():
    m = Parser( io.StringIO( "<foo left='right' less=\"more\"/>" ) ).readElement()
    assert "right" == m.get( "left" )
    assert "more" == m.get( "less" )
    assert m.isEmpty()
    
def testNested():
    m = Parser( io.StringIO( "<outer><foo left='right' less=\"more\"></foo></outer>" ) ).readElement()
    assert None != m
    assert 1 == len( m )
    assert "foo" == m[ 0 ].getName()
    assert m[ 0 ].isEmpty()

def testComment():
    m = Parser( io.StringIO( "<outer><!-- this is a comment --><foo left='right' less=\"more\"></foo></outer>" ) ).readElement()
    assert m != None
    assert 1 == len( m )
    assert "foo" == m[ 0 ].getName()
    assert m[0].isEmpty()
    
def testCommentWithEmbeddedSigns():
  m = Parser( io.StringIO( "<outer><!-- <- this -> is -> a <- comment <! --><foo left='right' less=\"more\"></foo></outer>" ) ).readElement()
  assert None != m
  assert 1 == len( m )
  assert "foo" == m[0].getName() 
  assert m[0].isEmpty()
    
def testBadComment():
    try:
        Parser( io.StringIO( "<outer><!-- <- this --> is -> a <- bad comment <! --><foo left='right' less=\"more\"></foo></outer>" ) ).readElement()
        raise Exception
    except MinXMLError:
        pass

    
def testPrologElison():
  m = Parser( io.StringIO( "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\"><xxx/>" ) ).readElement()
  assert None != m
  assert "xxx" == m.getName()
  assert m.isEmpty()
    
def testForbiddenLessThan():
    try:
        Parser( io.StringIO( "<xxx a='<'/>" ) ).readElement()
        raise Exception
    except MinXMLError:
        pass

    
def testForbiddenAmpersand():
    try:
        Parser( io.StringIO( "<xxx a='&'/>" ) ).readElement()
        raise Exception
    except MinXMLError:
        pass
    
def testForbiddenDoubleInDouble():
    try:
        Parser( io.StringIO( "<xxx a=\"\"\"/>" ) ).readElement()
        raise Exception
    except MinXMLError:
        pass
   
def testForbiddenSingleInSingle():
    try:
        Parser( io.StringIO( "<xxx a='\''/>" ) ).readElement()
        raise Exception
    except MinXMLError:
        pass
    
def testAllowedGreaterThan():
    assert Parser( io.StringIO( "<xxx a='>'/>" ) ).readElement() != None
    
def testAllowedSingleInDouble():
    assert None != Parser( io.StringIO( "<xxx a=\"'\"/>" ) ).readElement()

def testAllowedDoubleInSingle():
    assert None != Parser( io.StringIO( "<xxx a='\"'/>" ) ).readElement()
    
def testAsIterable():
    n = 0;
    for m in Parser( io.StringIO( "<xxx/><yyy/><!-- woot --><zzz/>" ) ):
          n += 1
    assert 3 == n

    
def testEscape():
    m = Parser( io.StringIO( "<foo bar='&lt;&gt;&amp;&quot;&apos;'/>" ) ).readElement()
    assert "<>&\"'" == m.get( "bar" )
            
def testPrintNumericEntities():
    m = Parser( io.StringIO( "<foo bar='&#12;'/>" ) ).readElement()
    assert "<foo bar=\"&#12;\"/>" == str( m )
            
def testUnfinished():
    try:
        Parser( io.StringIO( "<foo>" ) ).readElement()
        raise Exception 
    except MinXMLError:
        pass
    
def testNestedDiscard():
    m = (
        Parser( io.StringIO(
              "<?xml version=\"1.0\" standalone=\"yes\" ?>\n"
              "<!DOCTYPE author [\n"
              "  <!ELEMENT author (#PCDATA)>\n"
              "  <!ENTITY js \"Jo Smith\">\n"
              "]>\n"
              "<author name='Jo Smith'></author>"
          ) ).readElement()
    )
    assert "author" == m.getName() 
