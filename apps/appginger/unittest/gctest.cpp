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
#include "sysmap.hpp"

static int lengthOfAssocChain( Ref achain ) {
	int count = 0;
	for ( ; achain != sys_absent ; count++ ) {
		CPPUNIT_ASSERT( IsObj( achain ) && *RefToPtr4( achain ) == sysAssocKey );
		achain = RefToPtr4( achain )[ ASSOC_OFFSET_NEXT ];
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

static int verifyAssocChain( Ref chain ) {
	int count = 0;
	for (;;) {
		CPPUNIT_ASSERT( chain == sys_absent || IsAssoc( chain ) );
		if ( chain == sys_absent ) break;
		count += 1;
		chain = fastAssocNext( chain );
	}
	return count;
}

#define IsHashMapData( x ) 	( IsObj( x ) && ( RefToPtr4( x )[ 0 ] == sysHashMapDataKey ) )

static void verifyMapIntegrity( Ref map, Ref expected_map_key ) {
	CPPUNIT_ASSERT( IsMap( map ) );
	Ref * map_K = RefToPtr4( map );
	CPPUNIT_ASSERT_EQUAL( expected_map_key, map_K[ 0 ] );
	Ref data = map_K[ MAP_OFFSET_DATA ];
	CPPUNIT_ASSERT( IsHashMapData( data ) );
	Ref * data_K = RefToPtr4( data );
	int bit_width = fastMapPtrWidth( map_K );
	int length = SmallToLong( data_K[ VECTOR_OFFSET_LENGTH ] );
	
	//cout << "length = " << length << endl;
	//cout << "bit_width = " << bit_width << endl;

	CPPUNIT_ASSERT_EQUAL( length, 1 << bit_width );
	
	int sofar = 0;
	for ( int n = 1; n <= length; n++ ) {
		Ref chain = data_K[ n ];
		sofar += verifyAssocChain( chain );
	}
	
	long count = SmallToLong( map_K[ MAP_OFFSET_COUNT ] );
	CPPUNIT_ASSERT_EQUAL( count, (long)sofar );
}

/*
	{
		cout << "HardId: " << MapKeyEq( sysHardIdMapKey ) << endl;
		cout << "WeakId: " << MapKeyEq( sysWeakIdMapKey ) << endl;
		cout << "CacheEq: " << MapKeyEq( sysCacheEqMapKey ) << endl;
		cout << "HardEq: " << MapKeyEq( sysHardEqMapKey ) << endl;
		cout << endl;
		cout << "HardId: " << ( SimpleKeyID( sysHardIdMapKey ) & 0x3 ) << endl;
		cout << "WeakId: " << ( SimpleKeyID( sysWeakIdMapKey ) & 0x3 ) << endl;
		cout << "CacheEq: " << ( SimpleKeyID( sysCacheEqMapKey ) & 0x3 ) << endl;
		cout << "HardEq: " << ( SimpleKeyID( sysHardEqMapKey ) & 0x3 ) << endl;
	}
*/

void GCTest::testWeakIdRehashing() {
	{
		std::stringstream program;
		
		//	Create some garbage.
		program << "<sysapp name=\"newVector\"><sysapp name=\"explode\"><seq><constant type=\"string\" value=\"abcdefghijk\"/></seq></sysapp></sysapp>";
		
		//	val a := "alpha";
		//	val b := "beta";
		//	val c := "gamma";
		program << "<bind><var name=\"a\" protected=\"true\" tag=\"public\"/><constant type=\"string\" value=\"alpha\"/></bind>";
		program << "<bind><var name=\"b\" protected=\"true\" tag=\"public\"/><constant type=\"string\" value=\"beta\"/></bind>";
		program << "<bind><var name=\"c\" protected=\"true\" tag=\"public\"/><constant type=\"string\" value=\"gamma\"/></bind>";
		//	val map := newHardIdMap( a :- 11, b :- 22, "gamma" :- 33 );	# not c
		program << "<bind><var name=\"map\" protected=\"true\" tag=\"public\"/><app><id name=\"newWeakIdMap\"/><seq><seq><sysapp name=\"newMaplet\"><id name=\"a\"/><constant type=\"int\" value=\"11\"/></sysapp><sysapp name=\"newMaplet\"><id name=\"b\"/><constant type=\"int\" value=\"22\"/></sysapp></seq><sysapp name=\"newMaplet\"><constant type=\"string\" value=\"gamma\"/><constant type=\"int\" value=\"33\"/></sysapp></seq></app></bind>";
		
		//	val ma := map.index( a );
		//	val mb := map.index( b );
		//	val mc := map.index( c );
		program << "<bind><var name=\"ma\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><id name=\"a\"/></seq></app></bind>";
		program << "<bind><var name=\"mb\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><id name=\"b\"/></seq></app></bind>";
		program << "<bind><var name=\"mc\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><id name=\"c\"/></seq></app></bind>";
	
		//	val mav := map.index( "alpha" );
		program << "<bind><var name=\"mav\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><constant type=\"string\"  value=\"alpha\"/></seq></app></bind>";
		program << "<bind><var name=\"mbv\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><constant type=\"string\"  value=\"beta\"/></seq></app></bind>";
	
	
		std::ostringstream output;
		while ( rcep->unsafe_read_comp_exec_print( program, output ) ) {};
		
		verifyMapIntegrity( this->safeValof( "map" )->valof, sysWeakIdMapKey );
		{
			Ref map = this->safeValof( "map" )->valof;
			CPPUNIT_ASSERT_EQUAL( 3L, SmallToLong( RefToPtr4( map )[ MAP_OFFSET_COUNT ] ) );
		}
	
		{
			Valof * ma = this->safeValof( "ma" );
			//cout << endl << "pre GC: " << ( gngIdHash( this->safeValof( "a" )->valof ) & 0x3 ) << endl;
			//cout << ma->valof << endl;
			CPPUNIT_ASSERT_EQUAL( LongToSmall( 11 ), ma->valof );
		}
		{
			Valof * mb = this->safeValof( "mb" );
			CPPUNIT_ASSERT_EQUAL( LongToSmall( 22 ), mb->valof );
		}
		{
			Valof * mc = this->safeValof( "mc" );
			CPPUNIT_ASSERT_EQUAL( sys_absent, mc->valof );
		}
		{
			Valof * mav = this->safeValof( "mav" );
			CPPUNIT_ASSERT_EQUAL( sys_absent, mav->valof );
		}	
		{
			Valof * mbv = this->safeValof( "mbv" );
			CPPUNIT_ASSERT_EQUAL( sys_absent, mbv->valof );
		}	
	}

	vm->getPressure().setUnderPressure();
	sysQuiescentGarbageCollect( vm, NULL );
	
	verifyMapIntegrity( this->safeValof( "map" )->valof, sysWeakIdMapKey );
	{
		Ref map = this->safeValof( "map" )->valof;
		CPPUNIT_ASSERT_EQUAL( 2L, SmallToLong( RefToPtr4( map )[ MAP_OFFSET_COUNT ] ) );
	}

	{
		std::stringstream program;
		
		
		//	% val ma1 := map.index( a );
		//	% val mb1 := map.index( b );
		program << "<bind><var name=\"ma1\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><id name=\"a\"/></seq></app></bind>";
		program << "<bind><var name=\"mb1\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><id name=\"b\"/></seq></app></bind>";
		
		//	% val mav1 := map.index( "alpha" );
		//	% val mbv1 := map.index( "beta" );
		//	% val mcv1 := map.index( "gamma" );
		program << "<bind><var name=\"mav1\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><constant type=\"string\" value=\"alpha\"/></seq></app></bind>";
		program << "<bind><var name=\"mbv1\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><constant type=\"string\" value=\"beta\"/></seq></app></bind>";
		
		std::ostringstream output;
		while ( rcep->unsafe_read_comp_exec_print( program, output ) ) {
		}
		
		{
			Valof * ma1 = this->safeValof( "ma1" );
			//cout << endl << "post GC: " << ( gngIdHash( this->safeValof( "a" )->valof ) & 0x3 ) << endl;
			//cout << endl << "post GC: " << this->safeValof( "a" )->valof << "->" << ( gngIdHash( this->safeValof( "a" )->valof ) & 0x3 ) << endl;
			CPPUNIT_ASSERT_EQUAL( LongToSmall( 11 ), ma1->valof );
		}
		{
			Valof * mb1 = this->safeValof( "mb1" );
			CPPUNIT_ASSERT_EQUAL( LongToSmall( 22 ), mb1->valof );
		}
		{
			Valof * mav1 = this->safeValof( "mav1" );
			CPPUNIT_ASSERT_EQUAL( sys_absent, mav1->valof );
		}	
		{
			Valof * mbv1 = this->safeValof( "mbv1" );
			CPPUNIT_ASSERT_EQUAL( sys_absent, mbv1->valof );
		}	
	}

}

void GCTest::testHardIdRehashing() {
	{
		std::stringstream program;
		
		//	Create some garbage.
		program << "<sysapp name=\"newVector\"><sysapp name=\"explode\"><seq><constant type=\"string\" value=\"abcdefghijk\"/></seq></sysapp></sysapp>";
		
		//	val a := "alpha";
		//	val b := "beta";
		//	val c := "gamma";
		program << "<bind><var name=\"a\" protected=\"true\" tag=\"public\"/><constant type=\"string\" value=\"alpha\"/></bind>";
		program << "<bind><var name=\"b\" protected=\"true\" tag=\"public\"/><constant type=\"string\" value=\"beta\"/></bind>";
		program << "<bind><var name=\"c\" protected=\"true\" tag=\"public\"/><constant type=\"string\" value=\"gamma\"/></bind>";
		//	val map := newHardIdMap( a :- 11, b :- 22, c :- 33 );
		program << "<bind><var name=\"map\" protected=\"true\" tag=\"public\"/><app><id name=\"newHardIdMap\"/><seq><seq><sysapp name=\"newMaplet\"><id name=\"a\"/><constant type=\"int\" value=\"11\"/></sysapp><sysapp name=\"newMaplet\"><id name=\"b\"/><constant type=\"int\" value=\"22\"/></sysapp></seq><sysapp name=\"newMaplet\"><id name=\"c\"/><constant type=\"int\" value=\"33\"/></sysapp></seq></app></bind>";
		
		//	val ma := map.index( a );
		//	val mb := map.index( b );
		//	val mc := map.index( c );
		program << "<bind><var name=\"ma\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><id name=\"a\"/></seq></app></bind>";
		program << "<bind><var name=\"mb\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><id name=\"b\"/></seq></app></bind>";
		program << "<bind><var name=\"mc\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><id name=\"c\"/></seq></app></bind>";
	
		//	val mav := map.index( "alpha" );
		program << "<bind><var name=\"mav\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><constant type=\"string\" value=\"alpha\"/></seq></app></bind>";
		program << "<bind><var name=\"mbv\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><constant type=\"string\" value=\"beta\"/></seq></app></bind>";
		program << "<bind><var name=\"mcv\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><constant type=\"string\" value=\"gamma\"/></seq></app></bind>";
	
	
		std::ostringstream output;
		while ( rcep->unsafe_read_comp_exec_print( program, output ) ) {};
		
		verifyMapIntegrity( this->safeValof( "map" )->valof, sysHardIdMapKey );
	
		{
			Valof * ma = this->safeValof( "ma" );
			//cout << endl << "pre GC: " << ( gngIdHash( this->safeValof( "a" )->valof ) & 0x3 ) << endl;
			//cout << ma->valof << endl;
			CPPUNIT_ASSERT_EQUAL( LongToSmall( 11 ), ma->valof );
		}
		{
			Valof * mb = this->safeValof( "mb" );
			CPPUNIT_ASSERT_EQUAL( LongToSmall( 22 ), mb->valof );
		}
		{
			Valof * mc = this->safeValof( "mc" );
			CPPUNIT_ASSERT_EQUAL( LongToSmall( 33 ), mc->valof );
		}
		{
			Valof * mav = this->safeValof( "mav" );
			CPPUNIT_ASSERT_EQUAL( sys_absent, mav->valof );
		}	
		{
			Valof * mbv = this->safeValof( "mbv" );
			CPPUNIT_ASSERT_EQUAL( sys_absent, mbv->valof );
		}	
		{
			Valof * mcv = this->safeValof( "mcv" );
			CPPUNIT_ASSERT_EQUAL( sys_absent, mcv->valof );
		}
	}

	vm->getPressure().setUnderPressure();
	sysQuiescentGarbageCollect( vm, NULL );
	
	verifyMapIntegrity( this->safeValof( "map" )->valof, sysHardIdMapKey );

	{
		std::stringstream program;
		
		
		//	% val ma1 := map.index( a );
		//	% val mb1 := map.index( b );
		//	% val mc1 := map.index( c );
		program << "<bind><var name=\"ma1\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><id name=\"a\"/></seq></app></bind>";
		program << "<bind><var name=\"mb1\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><id name=\"b\"/></seq></app></bind>";
		program << "<bind><var name=\"mc1\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><id name=\"c\"/></seq></app></bind>";
	
		//	% val mav1 := map.index( "alpha" );
		//	% val mbv1 := map.index( "beta" );
		//	% val mcv1 := map.index( "gamma" );
		program << "<bind><var name=\"mav1\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><constant type=\"string\" value=\"alpha\"/></seq></app></bind>";
		program << "<bind><var name=\"mbv1\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><constant type=\"string\" value=\"beta\"/></seq></app></bind>";
		program << "<bind><var name=\"mcv1\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><constant type=\"string\" value=\"gamma\"/></seq></app></bind>";
	
		std::ostringstream output;
		while ( rcep->unsafe_read_comp_exec_print( program, output ) ) {
		}
		
		{
			Valof * ma1 = this->safeValof( "ma1" );
			//cout << endl << "post GC: " << ( gngIdHash( this->safeValof( "a" )->valof ) & 0x3 ) << endl;
			//cout << endl << "post GC: " << this->safeValof( "a" )->valof << "->" << ( gngIdHash( this->safeValof( "a" )->valof ) & 0x3 ) << endl;
			CPPUNIT_ASSERT_EQUAL( LongToSmall( 11 ), ma1->valof );
		}
		{
			Valof * mb1 = this->safeValof( "mb1" );
			CPPUNIT_ASSERT_EQUAL( LongToSmall( 22 ), mb1->valof );
		}
		{
			Valof * mc1 = this->safeValof( "mc1" );
			CPPUNIT_ASSERT_EQUAL( LongToSmall( 33 ), mc1->valof );
		}
		{
			Valof * mav1 = this->safeValof( "mav1" );
			CPPUNIT_ASSERT_EQUAL( sys_absent, mav1->valof );
		}	
		{
			Valof * mbv1 = this->safeValof( "mbv1" );
			CPPUNIT_ASSERT_EQUAL( sys_absent, mbv1->valof );
		}	
		{
			Valof * mcv1 = this->safeValof( "mcv1" );
			CPPUNIT_ASSERT_EQUAL( sys_absent, mcv1->valof );
		}
	}

}

void GCTest::testHardEqMap() {
	{
		std::stringstream program;
		
		//	Create some garbage.
		program << "<sysapp name=\"newVector\"><sysapp name=\"explode\"><seq><constant type=\"string\" value=\"abcdefghijk\"/></seq></sysapp></sysapp>";
		
		//	val a := "alpha";
		//	val b := "beta";
		//	val c := "gamma";
		program << "<bind><var name=\"a\" protected=\"true\" tag=\"public\"/><constant type=\"string\" value=\"alpha\"/></bind>";
		program << "<bind><var name=\"b\" protected=\"true\" tag=\"public\"/><constant type=\"string\" value=\"beta\"/></bind>";
		program << "<bind><var name=\"c\" protected=\"true\" tag=\"public\"/><constant type=\"string\" value=\"gamma\"/></bind>";
		
		
		
		//	val map := newHardIdMap( a :- 11, b :- 22, c :- 33 );
		program << "<bind><var name=\"map\" protected=\"true\" tag=\"public\"/><app><id name=\"newMap\"/><seq><seq><sysapp name=\"newMaplet\"><id name=\"a\"/><constant type=\"int\" value=\"11\"/></sysapp><sysapp name=\"newMaplet\"><id name=\"b\"/><constant type=\"int\" value=\"22\"/></sysapp></seq><sysapp name=\"newMaplet\"><id name=\"c\"/><constant type=\"int\" value=\"33\"/></sysapp></seq></app></bind>";
		
		
		//	val ma := map.index( a );
		//	val mb := map.index( b );
		//	val mc := map.index( c );
		program << "<bind><var name=\"ma\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><id name=\"a\"/></seq></app></bind>";
		program << "<bind><var name=\"mb\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><id name=\"b\"/></seq></app></bind>";
		program << "<bind><var name=\"mc\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><id name=\"c\"/></seq></app></bind>";
	
		//	val mav := map.index( "alpha" );
		//	val mbv := map.index( "beta" );
		//	val mcv := map.index( "gamma" );
		program << "<bind><var name=\"mav\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><constant type=\"string\" value=\"alpha\"/></seq></app></bind>";
		program << "<bind><var name=\"mbv\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><constant type=\"string\" value=\"beta\"/></seq></app></bind>";
		program << "<bind><var name=\"mcv\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><constant type=\"string\" value=\"gamma\"/></seq></app></bind>";
	
		std::ostringstream output;
		while ( rcep->unsafe_read_comp_exec_print( program, output ) ) {};

		verifyMapIntegrity( this->safeValof( "map" )->valof, sysHardEqMapKey );
		{
			Ref map = this->safeValof( "map" )->valof;
			CPPUNIT_ASSERT_EQUAL( 3L, SmallToLong( RefToPtr4( map )[ MAP_OFFSET_COUNT ] ) );
		}

		{
			Valof * ma = this->safeValof( "ma" );
			//cout << endl << "pre GC: " << ( gngIdHash( this->safeValof( "a" )->valof ) & 0x3 ) << endl;
			//cout << ma->valof << endl;
			CPPUNIT_ASSERT_EQUAL( LongToSmall( 11 ), ma->valof );
		}
		{
			Valof * mb = this->safeValof( "mb" );
			CPPUNIT_ASSERT_EQUAL( LongToSmall( 22 ), mb->valof );
		}
		{
			Valof * mc = this->safeValof( "mc" );
			CPPUNIT_ASSERT_EQUAL( LongToSmall( 33 ), mc->valof );
		}
		{
			Valof * mav = this->safeValof( "mav" );
			CPPUNIT_ASSERT_EQUAL( LongToSmall( 11 ), mav->valof );
		}	
		{
			Valof * mbv = this->safeValof( "mbv" );
			CPPUNIT_ASSERT_EQUAL( LongToSmall( 22 ), mbv->valof );
		}	
		{
			Valof * mcv = this->safeValof( "mcv" );
			CPPUNIT_ASSERT_EQUAL( LongToSmall( 33 ), mcv->valof );
		}
	}

	vm->getPressure().setUnderPressure();
	sysQuiescentGarbageCollect( vm, NULL );
	
	verifyMapIntegrity( this->safeValof( "map" )->valof, sysHardEqMapKey );
	{
		Ref map = this->safeValof( "map" )->valof;
		CPPUNIT_ASSERT_EQUAL( 3L, SmallToLong( RefToPtr4( map )[ MAP_OFFSET_COUNT ] ) );
	}

	{
		std::stringstream program;
		
		
		//	% val ma1 := map.index( a );
		//	% val mb1 := map.index( b );
		//	% val mc1 := map.index( c );
		program << "<bind><var name=\"ma1\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><id name=\"a\"/></seq></app></bind>";
		program << "<bind><var name=\"mb1\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><id name=\"b\"/></seq></app></bind>";
		program << "<bind><var name=\"mc1\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><id name=\"c\"/></seq></app></bind>";
	
		//	% val mav1 := map.index( "alpha" );
		//	% val mbv1 := map.index( "beta" );
		//	% val mcv1 := map.index( "gamma" );
		program << "<bind><var name=\"mav1\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><constant type=\"string\" value=\"alpha\"/></seq></app></bind>";
		program << "<bind><var name=\"mbv1\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><constant type=\"string\" value=\"beta\"/></seq></app></bind>";
		program << "<bind><var name=\"mcv1\" protected=\"true\" tag=\"public\"/><app><id name=\"index\"/><seq><id name=\"map\"/><constant type=\"string\" value=\"gamma\"/></seq></app></bind>";
	
		std::ostringstream output;
		while ( rcep->unsafe_read_comp_exec_print( program, output ) ) {
		}
		
		{
			Valof * ma1 = this->safeValof( "ma1" );
			//cout << endl << "post GC: " << ( gngIdHash( this->safeValof( "a" )->valof ) & 0x3 ) << endl;
			//cout << endl << "post GC: " << this->safeValof( "a" )->valof << "->" << ( gngIdHash( this->safeValof( "a" )->valof ) & 0x3 ) << endl;
			CPPUNIT_ASSERT_EQUAL( LongToSmall( 11 ), ma1->valof );
		}
		{
			Valof * mb1 = this->safeValof( "mb1" );
			CPPUNIT_ASSERT_EQUAL( LongToSmall( 22 ), mb1->valof );
		}
		{
			Valof * mc1 = this->safeValof( "mc1" );
			CPPUNIT_ASSERT_EQUAL( LongToSmall( 33 ), mc1->valof );
		}
		{
			Valof * mav1 = this->safeValof( "mav1" );
			CPPUNIT_ASSERT_EQUAL( LongToSmall( 11 ), mav1->valof );
		}	
		{
			Valof * mbv1 = this->safeValof( "mbv1" );
			CPPUNIT_ASSERT_EQUAL( LongToSmall( 22 ), mbv1->valof );
		}	
		{
			Valof * mcv1 = this->safeValof( "mcv1" );
			CPPUNIT_ASSERT_EQUAL( LongToSmall( 33 ), mcv1->valof );
		}
	}

}



void GCTest::testCacheEqMap() {
	std::stringstream program;
	
	//	val cmap := newCacheMap( "a" :- 1, "b" :- 2 );
	program << "<bind><var name=\"cmap\" protected=\"true\" tag=\"public\"/><app><id name=\"newCacheMap\"/><seq><sysapp name=\"newMaplet\"><constant type=\"string\" value=\"a\"/><constant type=\"int\" value=\"1\"/></sysapp><sysapp name=\"newMaplet\"><constant type=\"string\" value=\"b\"/><constant type=\"int\" value=\"2\"/></sysapp></seq></app></bind>";
	
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
	
	verifyMapIntegrity( this->safeValof( "cmap" )->valof, sysCacheEqMapKey );
	
	//	The first garbage collect should not affect the cache map.
	
	{
		Valof * cmap = this->safeValof( "cmap" );
		CPPUNIT_ASSERT( IsMap( cmap->valof ) );
		CPPUNIT_ASSERT_EQUAL( 0L, SmallToLong( RefToPtr4( cmap->valof )[ MAP_OFFSET_COUNT ] ) );
		CPPUNIT_ASSERT( IsHashMapData( RefToPtr4( cmap->valof )[ MAP_OFFSET_DATA ] ) );
		Ref * data_K = RefToPtr4( RefToPtr4( cmap->valof )[ MAP_OFFSET_DATA ] );
		const long N = SmallToLong( data_K[ VECTOR_OFFSET_LENGTH ]	);
		CPPUNIT_ASSERT( N > 1 );
		for ( long i = 1; i < N; i++ ) {
			CPPUNIT_ASSERT_EQUAL( sys_absent, data_K[ i ] );
		}
	}

}

void GCTest::testAllRef() {
	stringstream program;
	
	//	val h := newHardRef( [ "unique ref" ] );
	//	val w := newWeakRef( [ "unique ref" ] );
	//	val s := newSoftRef( [ "unique ref" ] );
	
	program << "<bind><var name=\"h\" protected=\"true\" tag=\"public\"/><app><id name=\"newHardRef\"/><sysapp name=\"newList\"><constant type=\"string\" value=\"unique ref\"/></sysapp></app></bind>";
	program << "<bind><var name=\"w\" protected=\"true\" tag=\"public\"/><app><id name=\"newWeakRef\"/><sysapp name=\"newList\"><constant type=\"string\" value=\"unique ref\"/></sysapp></app></bind>";
	program << "<bind><var name=\"s\" protected=\"true\" tag=\"public\"/><app><id name=\"newSoftRef\"/><sysapp name=\"newList\"><constant type=\"string\" value=\"unique ref\"/></sysapp></app></bind>";
	
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

void GCTest::testMethodWeakness() {	
	std::stringstream program;

	//	val alpha := newMethod( "alpha", 1, 1 ); 
	program << "<bind><var name=\"alpha\" protected=\"true\" tag=\"public\"/><app><id name=\"newMethod\"/><seq><seq><constant type=\"string\" value=\"alpha\"/><constant type=\"int\" value=\"1\"/></seq><constant type=\"int\" value=\"1\"/></seq></app></bind>";
	
	//	val A := newClass( "C", { alpha }, {}, {} );
	program << "<bind><var name=\"A\" protected=\"true\" tag=\"public\"/><app><id name=\"newClass\"/><seq><seq><seq><constant type=\"string\" value=\"C\"/><sysapp name=\"newVector\"><id name=\"alpha\"/></sysapp></seq><sysapp name=\"newVector\"><seq/></sysapp></seq><sysapp name=\"newVector\"><seq/></sysapp></seq></app></bind>";
	
	//	val B := newClass( "B", { alpha }, {}, {} );
	program << "<bind><var name=\"B\" protected=\"true\" tag=\"public\"/><app><id name=\"newClass\"/><seq><seq><seq><constant type=\"string\" value=\"B\"/><sysapp name=\"newVector\"><id name=\"alpha\"/></sysapp></seq><sysapp name=\"newVector\"><seq/></sysapp></seq><sysapp name=\"newVector\"><seq/></sysapp></seq></app></bind>";
	
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

