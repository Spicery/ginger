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

#include "gctest.hpp"
#include "appcontext.hpp"
#include "rcep.hpp"
#include "key.hpp"
#include "garbagecollect.hpp"
#include "gcstats.hpp"
#include "vectorlayout.hpp"
#include "listlayout.hpp"
#include "reflayout.hpp"

static int lengthOfAssocChain( Ref achain ) {
	int count = 0;
	for ( ; achain != sys_absent ; count++ ) {
		CPPUNIT_ASSERT( IsObj( achain ) && *RefToPtr4( achain ) == sysAssocKey );
		achain = RefToPtr4( achain )[ ASSOC_NEXT_OFFSET ];
	}
	return count;
}

Valof * GCTest::safeValof( const char * name ) {
	Valof * v = this->interactive->valof( name );
	CPPUNIT_ASSERT( v != NULL );
	return v;
}

void GCTest::checkAlpha( int N ) {
	Valof * v = this->safeValof( "alpha" );
	CPPUNIT_ASSERT( IsMethod( v->valof ) );
	
	Ref achain = RefToPtr4( v->valof )[ METHOD_OFFSET_DISPATCH_TABLE ];
	
	CPPUNIT_ASSERT_EQUAL( N, lengthOfAssocChain( achain ) );
}

void GCTest::testRehashing() {
	std::stringstream program;
	
	/*
	% val a := "alpha";
	% val b := "beta";
	% val c := "gamma";
	*/
	program << "<bind><var name=\"a\" protected=\"true\" tag=\"public\"/><string value=\"alpha\"/></bind>";
	program << "<bind><var name=\"b\" protected=\"true\" tag=\"public\"/><string value=\"beta\"/></bind>";
	program << "<bind><var name=\"c\" protected=\"true\" tag=\"public\"/><string value=\"gamma\"/></bind>";
	//	val map := newHardIdMap( a :- 11, b :- 22, c :- 33 );
	program << "<bind><var name=\"map\" protected=\"true\" tag=\"public\"/><app><id name=\"newHardIdMap\"/><seq><seq><sysapp name=\"newMaplet\"><id name=\"a\"/><int value=\"11\"/></sysapp><sysapp name=\"newMaplet\"><id name=\"b\"/><int value=\"22\"/></sysapp></seq><sysapp name=\"newMaplet\"><id name=\"c\"/><int value=\"33\"/></sysapp></seq></app></bind>";

	
	//	% val ma := map.index( a );
	//	% val mb := map.index( b );
	//	% val mc := map.index( c );
	program << "<bind><var name=\"ma\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><id name=\"a\"/></seq></app></bind>";
	program << "<bind><var name=\"mb\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><id name=\"b\"/></seq></app></bind>";
	program << "<bind><var name=\"mc\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><id name=\"c\"/></seq></app></bind>";

	//	% val mav := map.index( "alpha" );
	//	% val mbv := map.index( "beta" );
	//	% val mcv := map.index( "gamma" );
	program << "<bind><var name=\"mav\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><string value=\"alpha\"/></seq></app></bind>";
	program << "<bind><var name=\"mbv\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><string value=\"beta\"/></seq></app></bind>";
	program << "<bind><var name=\"mcv\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><string value=\"gamma\"/></seq></app></bind>";


	std::ostringstream output;
	while ( rcep->unsafe_read_comp_exec_print( program, output ) ) {};

	{
		Valof * ma = this->safeValof( "ma" );
		CPPUNIT_ASSERT_EQUAL( LongToSmall( 11 ), ma->valof );

	}	

	//vm->getPressure().setUnderPressure();
	//sysQuiescentGarbageCollect( vm, NULL );
	

}



void GCTest::testRef() {
	stringstream program;
	
	//	val h := newHardRef( [ "unique ref" ] );
	//	val w := newWeakRef( [ "unique ref" ] );
	//	val s := newSoftRef( [ "unique ref" ] );
	
	program << "<bind><var name=\"h\" protected=\"true\" tag=\"public\"/><app><id name=\"newHardRef\"/><sysapp name=\"newList\"><string value=\"unique ref\"/></sysapp></app></bind>";
	program << "<bind><var name=\"w\" protected=\"true\" tag=\"public\"/><app><id name=\"newWeakRef\"/><sysapp name=\"newList\"><string value=\"unique ref\"/></sysapp></app></bind>";
	program << "<bind><var name=\"s\" protected=\"true\" tag=\"public\"/><app><id name=\"newSoftRef\"/><sysapp name=\"newList\"><string value=\"unique ref\"/></sysapp></app></bind>";
	
	std::ostringstream output;
	while ( rcep->unsafe_read_comp_exec_print( program, output ) ) {};

	//	Force an unpressured GC.

	vm->getPressure().clearUnderPressure();
	sysQuiescentGarbageCollect( vm, NULL );
	
	//	HardRef not garbage-collected.
	{	
		Ref h = this->safeValof( "h" )->valof;
		CPPUNIT_ASSERT( IsRef( h ) );
		CPPUNIT_ASSERT( isList( RefToPtr4( h )[ REF_OFFSET_CONT ] ) );
	}	
	//	WeakRef is garbage-collected.
	{
		Ref w =  this->safeValof( "w" )->valof;
		CPPUNIT_ASSERT( IsRef( w ) );
		CPPUNIT_ASSERT_EQUAL( sys_absent, RefToPtr4( w )[ REF_OFFSET_CONT ] );
	}
	//	SoftRef is not garbage-collected (no pressure).
	{	
		Ref s = this->safeValof( "s" )->valof;
		CPPUNIT_ASSERT( IsRef( s ) );
		CPPUNIT_ASSERT( isList( RefToPtr4( s )[ REF_OFFSET_CONT ] ) );
	}	

	//	Force pressurised garbage collection.
	vm->getPressure().setUnderPressure();
	sysQuiescentGarbageCollect( vm, NULL );

	//	HardRef survives pressure.
	{	
		Ref h = this->safeValof( "h" )->valof;
		CPPUNIT_ASSERT( IsRef( h ) );
		CPPUNIT_ASSERT( isList( RefToPtr4( h )[ REF_OFFSET_CONT ] ) );
	}	
	//	WeakRef was already collected.
	{
		Ref w =  this->safeValof( "w" )->valof;
		CPPUNIT_ASSERT( IsRef( w ) );
		CPPUNIT_ASSERT_EQUAL( sys_absent, RefToPtr4( w )[ REF_OFFSET_CONT ] );
	}
	//	SoftRef is forced to be weak & therefore collected.
	{
		Ref s =  this->safeValof( "s" )->valof;
		CPPUNIT_ASSERT( IsRef( s ) );
		CPPUNIT_ASSERT_EQUAL( sys_absent, RefToPtr4( s )[ REF_OFFSET_CONT ] );
	}	
}

void GCTest::testCacheMap() {
	std::stringstream program;
	
	//	val cmap := newCacheMap( "a" :- 1, "b" :- 2 );
	program << "<bind><var name=\"cmap\" protected=\"true\" tag=\"public\"/><app><id name=\"newCacheMap\"/><seq><sysapp name=\"newMaplet\"><string value=\"a\"/><int value=\"1\"/></sysapp><sysapp name=\"newMaplet\"><string value=\"b\"/><int value=\"2\"/></sysapp></seq></app></bind>";
	
	std::ostringstream output;
	while ( rcep->unsafe_read_comp_exec_print( program, output ) ) {};

	{
		Valof * cmap = this->safeValof( "cmap" );
		CPPUNIT_ASSERT( IsObj( cmap->valof ) );
		CPPUNIT_ASSERT( IsMap( cmap->valof ) );
		CPPUNIT_ASSERT_EQUAL( 2L, SmallToLong( RefToPtr4( cmap->valof )[ MAP_OFFSET_COUNT ] ) );

	}	

	vm->getPressure().setUnderPressure();
	sysQuiescentGarbageCollect( vm, NULL );
	
	//	The first garbage collect should not affect the cache map.
	
	{
		Valof * cmap = this->safeValof( "cmap" );
		CPPUNIT_ASSERT( IsMap( cmap->valof ) );
		CPPUNIT_ASSERT_EQUAL( 0L, SmallToLong( RefToPtr4( cmap->valof )[ MAP_OFFSET_COUNT ] ) );
		CPPUNIT_ASSERT( IsVector( RefToPtr4( cmap->valof )[ MAP_OFFSET_DATA ] ) );
		Ref * data_K = RefToPtr4( RefToPtr4( cmap->valof )[ MAP_OFFSET_DATA ] );
		const long N = SmallToLong( data_K[ VECTOR_OFFSET_LENGTH ]	);
		CPPUNIT_ASSERT( N > 1 );
		for ( long i = 1; i < N; i++ ) {
			CPPUNIT_ASSERT_EQUAL( sys_absent, data_K[ i ] );
		}
	}

}

void GCTest::testMethod() {	
	std::stringstream program;

	//	val alpha := newMethod( "alpha", 1, 1 ); 
	program << "<bind><var name=\"alpha\" protected=\"true\" tag=\"public\"/><app><id name=\"newMethod\"/><seq><seq><string value=\"alpha\"/><int value=\"1\"/></seq><int value=\"1\"/></seq></app></bind>";
	
	//	val A := newClass( "C", { alpha }, {}, {} );
	program << "<bind><var name=\"A\" protected=\"true\" tag=\"public\"/><app><id name=\"newClass\"/><seq><seq><seq><string value=\"C\"/><sysapp name=\"newVector\"><id name=\"alpha\"/></sysapp></seq><sysapp name=\"newVector\"><seq/></sysapp></seq><sysapp name=\"newVector\"><seq/></sysapp></seq></app></bind>";
	
	//	val B := newClass( "B", { alpha }, {}, {} );
	program << "<bind><var name=\"B\" protected=\"true\" tag=\"public\"/><app><id name=\"newClass\"/><seq><seq><seq><string value=\"B\"/><sysapp name=\"newVector\"><id name=\"alpha\"/></sysapp></seq><sysapp name=\"newVector\"><seq/></sysapp></seq><sysapp name=\"newVector\"><seq/></sysapp></seq></app></bind>";
	
	std::ostringstream output;
	while ( rcep->unsafe_read_comp_exec_print( program, output ) ) {};
	
	this->checkAlpha( 2 );
	
	//cerr << "GC[1]" << endl;
	GCStats stats1;
	sysQuiescentGarbageCollect( vm, &stats1 );		
	
	this->checkAlpha( 2 );
	
	Valof * b = this->safeValof( "B" );
	b->valof = sys_absent;
	
	//cerr << "GC[2]" << endl;
	GCStats stats2;
	sysQuiescentGarbageCollect( vm, &stats2 );
	
	this->checkAlpha( 1 );
	
}

CPPUNIT_TEST_SUITE_REGISTRATION( GCTest );

