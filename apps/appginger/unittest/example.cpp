#include <iostream>

#include "example.hpp"
#include <cppunit/TestAssert.h>

void ExampleTest::testOK() {
	//std::cout << "testOK" << std::endl;
	//CPPUNIT_FAIL( "Testing a failure" );
}

CPPUNIT_TEST_SUITE_REGISTRATION( ExampleTest );
