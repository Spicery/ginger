#include <iostream>
using namespace std;

#include "common.hpp"
#include "key.hpp"
#include "heap.hpp"
#include "garbagecollect.hpp"

#include <cstring>

//#define DBG_HEAP

#define ARBITRARY_SIZE 1048576


CageClass * HeapClass::preflight( int size ) {
	#ifdef DBG_HEAP 
		cerr << "Preflight " << size << endl;
	#endif
	if ( this->current->checkRoom( size ) ) {
		return this->current;
	} else {
		return static_cast< CageClass * >( 0 );
	}
}

CageClass * HeapClass::preflight( Ref * & pc, int size ) {
	#ifdef DBG_HEAP 
		cerr << "Preflight " << size << endl;
	#endif
	if ( this->current->checkRoom( size ) ) {
		return this->current;
	} else {
		pc = sysGarbageCollect( pc, this->machine_ptr );
		if ( this->current->checkRoom( size ) ) {
			return this->current;
		} else {
			CageClass * new_cage = new CageClass( max( ARBITRARY_SIZE, size ) );
			this->zoo.push_back( this->current );
			this->current = new_cage;
			return this->current;
		}
	}
}

/*
 * Copy string into a cage of the heap, possibly incurring a GC.
 * We add a null-termination byte too.
 */
Ref HeapClass::copyString( const char *s ) {
	//	Strings have a two Ref overhead & must be ref aligned and have
	//	an additional 1-byte overhead for null-byte.
	size_t n = strlen( s );
	Ref * fake_pc = static_cast< Ref * >( 0 );
	XfrClass xfr( fake_pc, *this, 2 + ( n + 1 + sizeof( Ref ) - 1 ) / sizeof( Ref ) );
	
	//	We will actually copy n+1 bytes to include the null-termination.
	//	So the calculation of the size is a little bit trickier.
	xfr.xfrRef( LongToRef( n ) );
	xfr.setOrigin();
	xfr.xfrRef( sysStringKey );
	//	printf( "Set origin to '%x'\n", ToUInt( sysStringKey ) );
	xfr.xfrSubstring( s, 0, n );
	return xfr.make();

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
	for ( vector< CageClass * >::iterator it = this->zoo.begin(); it != zoo.end(); ++it ) {
		if ( (*it)->isEmpty() ) {
			this->current = *it;
			return;
		}
	}	
	this->current = this->newCageClass();
}