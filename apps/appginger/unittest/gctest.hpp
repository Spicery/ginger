/******************************************************************************\
	Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
	of the GNU General Public License. This file is part of Ginger.

    Ginger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Ginger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Ginger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

#ifndef METHOD_TEST_HPP
#define METHOD_TEST_HPP

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

#include "appcontext.hpp"
#include "rcep.hpp"

class GCTest : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( GCTest );
	CPPUNIT_TEST( testAllRef );
	CPPUNIT_TEST( testMethodWeakness );
	CPPUNIT_TEST( testCacheEqMap );
	CPPUNIT_TEST( testHardEqMap );
	CPPUNIT_TEST( testHardIdRehashing );
	CPPUNIT_TEST( testWeakIdRehashing );
	CPPUNIT_TEST_SUITE_END();
	
protected:
	void testAllRef();
	void testMethodWeakness();
	void testCacheEqMap();
	void testHardEqMap();
	void testHardIdRehashing();
	void testWeakIdRehashing();

protected:	
	AppContext * context;
	MachineClass * vm;
	Package * interactive;
	RCEP * rcep;

protected:
	void checkAlpha( int N );


public:
	Valof * safeValof( const char * name );

	void setUp(); 
	
	void tearDown();

};

#endif
