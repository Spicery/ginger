
#ifndef EXAMPLE_HPP
#define EXAMPLE_HPP

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

class ExampleTest : public CppUnit::TestFixture {
	
	CPPUNIT_TEST_SUITE( ExampleTest );
	CPPUNIT_TEST( testOK );
	CPPUNIT_TEST_SUITE_END();
	
protected:
	void testOK();

};


#endif