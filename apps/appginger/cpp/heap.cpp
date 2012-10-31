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
using namespace std;

#include "common.hpp"
#include "key.hpp"
#include "heap.hpp"
#include "garbagecollect.hpp"
#include "doublelayout.hpp"

#include <cstring>

//#define DBG_HEAP

#define ARBITRARY_SIZE 1048576

bool HeapClass::wouldGC( int size ) {
	return not this->current->checkRoom( size );
}

CageClass * HeapClass::preflight( Ref * & pc, int size ) {
	#ifdef DBG_HEAP 
		cerr << "Preflight " << size << endl;
	#endif
	if ( this->current->checkRoom( size ) ) {
		return this->current;
	} else {
		if ( this->machine_ptr->gcMoveEnabled() ) {
			pc = sysGarbageCollect( pc, this->machine_ptr );
			if ( this->current->checkRoom( size ) ) {
				return this->current;
			}
		}
		return this->preflight( size );
	}
}

CageClass * HeapClass::preflight( int size ) {
	#ifdef DBG_HEAP 
		cerr << "Preflight " << size << endl;
	#endif
	if ( this->current->checkRoom( size ) ) {
		return this->current;
	} else {
		CageClass * new_cage = new CageClass( max( ARBITRARY_SIZE, size ) );
		this->zoo.push_back( this->current );
		this->current = new_cage;
		return this->current;		
	}
}

/*
 * Copy string into a cage of the heap, possibly incurring a GC.
 * We add a null-termination byte too.
 */
Ref HeapClass::copyString( Ref * & pc, const char * s ) {
	//	Strings have a two Ref overhead & must be ref aligned and have
	//	an additional 1-byte overhead for null-byte.
	size_t n = strlen( s );

	//	TODO: This looks like a defect to me! Check all uses!!
	//	Surely we should not be using fake_pc but real pc. 
	//	OK - I have commented out the offending line - leaving this 
	//	message as a reminder to review all uses in more detail later.
	//	Ref * fake_pc = static_cast< Ref * >( 0 );
	XfrClass xfr( pc, *this, 2 + ( n + 1 + sizeof( Ref ) - 1 ) / sizeof( Ref ) );
	
	//	We will actually copy n+1 bytes to include the null-termination.
	//	So the calculation of the size is a little bit trickier.
	xfr.xfrRef( LongToRef( n ) );
	xfr.setOrigin();
	xfr.xfrRef( sysStringKey );
	//	printf( "Set origin to '%x'\n", ToUInt( sysStringKey ) );
	xfr.xfrSubstring( s, 0, n );
	return xfr.makeRef();
}


/*
 * Copy string into a cage of the heap, possibly incurring a GC.
 * We add a null-termination byte too.
 */
Ref HeapClass::copyString( const char *s ) {
	Ref * fake_pc = static_cast< Ref * >( 0 );
	return this->copyString( fake_pc, s );
}

Ref HeapClass::copyDouble( Ref * & pc, gngdouble d ) {
	XfrClass xfr( pc, *this, DOUBLE_SIZE );
	xfr.setOrigin();
	xfr.xfrRef( sysDoubleKey );
	xfr.xfrCopy( reinterpret_cast< Ref * >( &d ), DOUBLE_VALUE_SIZE );
	return xfr.makeRef();
}

Ref HeapClass::copyDouble( gngdouble d ) {
	Ref * fake_pc = static_cast< Ref * >( 0 );
	return this->copyDouble( fake_pc, d );
}


HeapClass::HeapClass( MachineClass * machine ) :
	machine_ptr( machine )
{	
	this->current = this->newCageClass();
}

HeapClass::~HeapClass() {
	for ( vector< CageClass * >::iterator it = this->zoo.begin(); it != zoo.end(); ++it ) {
		delete *it;
	}
}

CageClass * HeapClass::newCageClass() {
	CageClass * c = new CageClass( ARBITRARY_SIZE );
	this->zoo.push_back( c );
	return c;
}

void HeapClass::selectCurrent() {
	this->current = 0;
	for ( vector< CageClass * >::iterator it = this->zoo.begin(); it != zoo.end(); ++it ) {
		this->current = *it;	// We prefer non-empty, so only stop if we get that.§
		if ( not (*it)->isEmpty() ) {
			return;
		}
	}	
	//	I do not believe we can ever get here. We should *always* have a 
	if ( not this->current ) this->current = this->newCageClass();
}
