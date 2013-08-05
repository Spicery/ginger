/******************************************************************************\
	Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
	of the GNU General Public License. This file is part of AppGinger.

    AppGinger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AppGinger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AppGinger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

#include <iostream>
#include <sstream>
#include <cstdio>

#include "CppUnitLite2.h"

#include "sax.hpp"
//#include "saxtest.hpp"
#include "mishap.hpp"

using namespace std;
using namespace Ginger;



//------------------------------------------------------------------------------

class MockHandler : public SaxHandler {
public:
	virtual void check() = 0;	
	virtual const char * testString() const = 0;

public:
	void runStreamTest() {
		stringstream ss( this->testString() );
		SaxParser parser( ss, *this );
		parser.readElement();
		this->check();
	}
	
	void runFileDescriptorTest() {
		FILE * fd = fopen( "test.tmp", "w" );
		fprintf( fd, "%s\n", this->testString() );
		fclose( fd );
		
		fd = fopen( "test.tmp", "r" );
		SaxParser parser( fd, *this );
		parser.readElement();
		this->check();
		fclose( fd );
	}
	
public:
	~MockHandler() {}
};

//------------------------------------------------------------------------------

class AssertEqualFailure : public std::runtime_error {
public:
	AssertEqualFailure( const char * x, const char * y, long line, const char * file ) 
		: std::runtime_error( "assert equal failure" )
	{}
};

class AssertTrueFailure : public std::runtime_error
{
public:
	AssertTrueFailure( const char * x, long line, const char * file )
		: std::runtime_error( "assert true failure" )
	{}
};

class AssertFail : public std::runtime_error
{
public:
	AssertFail( const char * x, long line, const char * file )
		: std::runtime_error( "assert fail" )
	{}
};

#define ASSERT_EQUAL( x, y ) \
	if ( !( (x) == (y) ) ) \
		throw AssertEqualFailure( #x, #y, __LINE__, __FILE__ );

#define ASSERT_TRUE( x ) \
	if ( !( x ) ) \
		throw AssertTrueFailure( #x, __LINE__, __FILE__ );

#define ASSERT_FAIL( x ) \
	throw AssertFail( x, __LINE__, __FILE__ );

class StreamParseMockHandler : public MockHandler {
private:
	int startCount;
	int endCount;

public:
	virtual void startTag( std::string & name, std::map< std::string, std::string > & attrs ) {
		ASSERT_EQUAL( 0, startCount++ );
		ASSERT_EQUAL( string( "alpha" ), name );
		ASSERT_EQUAL( attrs[ "red" ], string( "blue" ) );
		ASSERT_EQUAL( attrs[ "yellow" ], string( "" ) );
	}
	
	virtual void endTag( std::string & name ) {
		ASSERT_EQUAL( 0, endCount++ );
		ASSERT_EQUAL( "alpha", name );
	}
	
	void check() {
		//cout << "startCount = " << startCount << " & endCount = " << endCount << endl;
		ASSERT_TRUE( startCount == endCount && startCount == 1 );
	}
	
	const char * testString() const {
		return "<alpha red=\"blue\"/>";
	}
	
public:
	StreamParseMockHandler() :
		startCount( 0 ),
		endCount( 0 )
	{}
	virtual ~StreamParseMockHandler() {}
};


TEST( StreamParse__Sax ) {
	StreamParseMockHandler m;
	//m.runStreamTest();
	m.runFileDescriptorTest();
}

//------------------------------------------------------------------------------


class StreamParseNestedMockHandler : public MockHandler {
private:
	int startCount;
	int endCount;

public:
	virtual void startTag( std::string & name, std::map< std::string, std::string > & attrs ) {
		switch ( startCount++ ) {
			case 0: ASSERT_EQUAL( name, "alpha" ); break;
			case 1: ASSERT_EQUAL( name, "beta" ); break;
			default: ASSERT_FAIL( "invalid name" );
		}
	}
	
	virtual void endTag( std::string & name ) {
		switch ( endCount++ ) {
			case 1: ASSERT_EQUAL( name, "alpha" ); break;
			case 0: ASSERT_EQUAL( name, "beta" ); break;
			default: ASSERT_FAIL( "invalid name" );
		}
	}
	
	void check() {
		ASSERT_TRUE( startCount == endCount && startCount == 2 );
	}
	
	const char * testString() const {
		return "<alpha><beta/></alpha>";
	}
	
public:
	StreamParseNestedMockHandler() :
		startCount( 0 ),
		endCount( 0 )
	{}
	virtual ~StreamParseNestedMockHandler() {}
};

TEST( StreamParseNested__Sax ) {
	StreamParseNestedMockHandler m;
	//m.runStreamTest();
	m.runFileDescriptorTest();
}

//------------------------------------------------------------------------------


class TestMultipleChildren : public MockHandler {
private:
	int startCount;
	int endCount;

public:
	const char * testString() const {
		return "<alpha><beta/><gamma><delta foo  = 'bar' /></gamma></alpha>";
	}
	
	virtual void startTag( std::string & name, std::map< std::string, std::string > & attrs ) {
		switch ( startCount++ ) {
			case 0: ASSERT_EQUAL( string( "alpha" ), name ); break;
			case 1: ASSERT_EQUAL( string( "beta" ), name ); break;
			case 2: ASSERT_EQUAL( string( "gamma" ), name ); break;
			case 3: ASSERT_EQUAL( string( "delta" ), name ); {
				ASSERT_EQUAL( attrs[ "foo" ], string( "bar" ) );
				break;
			}
			default: ASSERT_FAIL( "invalid name" );
		}
	}
	
	virtual void endTag( std::string & name ) {
		switch ( endCount++ ) {
			case 3: ASSERT_EQUAL( string( "alpha" ), name ); break;
			case 0: ASSERT_EQUAL( string( "beta" ), name ); break;
			case 2: ASSERT_EQUAL( string( "gamma" ), name ); break;
			case 1: ASSERT_EQUAL( string( "delta" ), name  ); break;
			default: ASSERT_FAIL( "invalid name" );
		}
	}
	
	void check() {
		ASSERT_TRUE( startCount == endCount && startCount == 4 );
	}
	
public:
	TestMultipleChildren() :
		startCount( 0 ),
		endCount( 0 )
	{}
	virtual ~TestMultipleChildren() {}
};

TEST( MultipleChildren__Sax ) {
	try {
		TestMultipleChildren m;
		//m.runStreamTest();
		m.runFileDescriptorTest();
	} catch ( Ginger::Mishap e ) {
		e.report();
	}
}

//------------------------------------------------------------------------------
