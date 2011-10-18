
#ifndef EXAMPLE_HPP
#define EXAMPLE_HPP

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

class SimpleKeyTest : public CppUnit::TestFixture {
	
	CPPUNIT_TEST_SUITE( SimpleKeyTest );
	CPPUNIT_TEST( testOK );
	CPPUNIT_TEST_SUITE_END();
	
protected:
	void testOK();

};


#endif
