#!/usr/bin/python3

from minxml import *

def testMakeSimple():
    builder = Builder()
    builder.startTagOpen( "xxx" )
    builder.startTagClose( "xxx" )
    builder.endTag( "xxx" )
    x = builder.build()
    assert "xxx" == x.getName()
    assert 0 == len( x )
    assert 0 == len( x.getAttributes() )
    
def testNested():
    builder = Builder()
    builder.startTagOpen( "xxx" )
    builder.put( "beta", "002" )
    builder.put( "alpha", "001" )
    builder.startTagClose( "xxx" )
    builder.startTagOpen( "yyy" )
    builder.startTagClose( "yyy" )
    builder.endTag( "yyy" )
    builder.startTagOpen( "zzz" )
    builder.startTagClose( "zzz" )
    builder.endTag( "zzz" )
    builder.endTag( "xxx" )
    x = builder.build()
    assert "<xxx alpha=\"001\" beta=\"002\"><yyy/><zzz/></xxx>" == str( x )
