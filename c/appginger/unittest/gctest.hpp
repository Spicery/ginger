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

#ifndef METHOD_TEST_HPP
#define METHOD_TEST_HPP

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

class GCTest : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( GCTest );
	CPPUNIT_TEST( testMethod );
	CPPUNIT_TEST( testCacheMap );
	CPPUNIT_TEST_SUITE_END();
	
protected:
	void testMethod();
	void testCacheMap();
};

#endif
