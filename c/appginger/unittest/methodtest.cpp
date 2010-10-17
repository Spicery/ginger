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

#include <sstream>
using namespace std;

#include <cppunit/TestAssert.h>

#include "methodtest.hpp"
#include "appcontext.hpp"
#include "rcep.hpp"
#include "key.hpp"
#include "garbagecollect.hpp"
#include "gcstats.hpp"

static int lengthOfAssocChain( Ref achain ) {
	int count = 0;
	for ( ; achain != sys_absent ; count++ ) {
		CPPUNIT_ASSERT( IsObj( achain ) && *RefToPtr4( achain ) == sysAssocKey );
		achain = RefToPtr4( achain )[ ASSOC_NEXT_OFFSET ];
	}
	return count;
}

static Valof * safeValof( Package * interactive, const char * name ) {
	Valof * v = interactive->valof( name );
	CPPUNIT_ASSERT( v != NULL );
	return v;
}

static void checkAlpha( Package * interactive, int N ) {
	Valof * v = safeValof( interactive, "alpha" );
	CPPUNIT_ASSERT( IsMethod( v->valof ) );
	
	Ref achain = RefToPtr4( v->valof )[ METHOD_OFFSET_DISPATCH_TABLE ];
	
	CPPUNIT_ASSERT_EQUAL( N, lengthOfAssocChain( achain ) );
}

void MethodTest::testMethod() {
	AppContext context;
	MachineClass * vm = context.newMachine();
	Package * interactive = context.initInteractivePackage( vm );
	RCEP rcep( interactive );
	
	std::stringstream program;

	//	val alpha := newMethod( "alpha", 1, 1 ); 
	program << "<bind><var name=\"alpha\" protected=\"true\" tag=\"public\"/><app><id name=\"newMethod\"/><seq><seq><string value=\"alpha\"/><int value=\"1\"/></seq><int value=\"1\"/></seq></app></bind>";
	
	//	val A := newClass( "C", { alpha }, {}, {} );
	program << "<bind><var name=\"A\" protected=\"true\" tag=\"public\"/><app><id name=\"newClass\"/><seq><seq><seq><string value=\"C\"/><sysapp name=\"newVector\"><id name=\"alpha\"/></sysapp></seq><sysapp name=\"newVector\"><seq/></sysapp></seq><sysapp name=\"newVector\"><seq/></sysapp></seq></app></bind>";
	
	//	val B := newClass( "B", { alpha }, {}, {} );
	program << "<bind><var name=\"B\" protected=\"true\" tag=\"public\"/><app><id name=\"newClass\"/><seq><seq><seq><string value=\"B\"/><sysapp name=\"newVector\"><id name=\"alpha\"/></sysapp></seq><sysapp name=\"newVector\"><seq/></sysapp></seq><sysapp name=\"newVector\"><seq/></sysapp></seq></app></bind>";
	
	std::ostringstream output;
	while ( rcep.unsafe_read_comp_exec_print( program, output ) ) {};
	
	checkAlpha( interactive, 2 );
	
	//cerr << "GC[1]" << endl;
	GCStats stats1;
	sysQuiescentGarbageCollect( vm, &stats1 );		
	
	checkAlpha( interactive, 2 );
	
	Valof * b = safeValof( interactive, "B" );
	b->valof = sys_absent;
	
	//cerr << "GC[2]" << endl;
	GCStats stats2;
	sysQuiescentGarbageCollect( vm, &stats2 );
	
	checkAlpha( interactive, 1 );
	
}

CPPUNIT_TEST_SUITE_REGISTRATION( MethodTest );
