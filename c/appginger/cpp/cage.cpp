#include <string.h>
#include <stdio.h>
#include <vector>
#include <stdexcept>

#include <cstdio>
#include <cstdlib>

#include "heap.hpp"
#include "gc.hpp"
#include "key.hpp"
#include "mishap.hpp"

CageClass::CageClass( int capacity ) {
	size_t n = sizeof( Ref ) * capacity;
	Ref *data = (Ref *)malloc( n );
	this->start = data;
	this->top = this->start;
	this->end = this->start + n;
}

XfrClass::XfrClass( CageClass & c ) :
	tmptop( c.top ),
	origin( NULL ),
	cage( c )
{
}

XfrClass::XfrClass( HeapClass & h, int preflight ) :
	origin( NULL ),
	cage( h.preflight( preflight ) )
{
	this->tmptop = this->cage.top;
}

XfrClass::XfrClass( MachineClass & m, int preflight ) :
	origin( NULL ),
	cage( m.heap().preflight( preflight ) )
{
	this->tmptop = this->cage.top;
}

CageClass::~CageClass() {
	free( this->start );
}

bool CageClass::checkRoom( int k ) {
	return this->end - this->top >= k;
}

bool XfrClass::checkRoom( int k ) {
	return this->cage.end - this->tmptop >= k;
}

//  copy x to tmp-end-of-heap
void XfrClass::xfrRef( Ref x ) {
	*( this->tmptop++ ) = x;
}

//	Copy from elements a to b of chain.
void XfrClass::xfrVector( std::vector< Ref > & v ) {
	for ( std::vector< Ref >::iterator it = v.begin(); it != v.end(); ++it ) {
		*( this->tmptop++ ) = *it;
	}
}

//  Copy characters from index a to b of s.
void XfrClass::xfrSubstring( const char *s, int a, int b ) {
	/*
	printf(
		"Copying from %d to %d = %d bytes = %d words\n",
		a, b,
		b - a + 1,
		( sizeof( Ref ) + b - a ) / sizeof( Ref )
	);
	*/
	memcpy( (void *)(this->tmptop), s + a, b - a + 1 );
	this->tmptop += ( sizeof( Ref ) + b - a ) / sizeof( Ref );
}

//  set the object origin
void XfrClass::setOrigin() {
	this->origin = this->tmptop;
}

//  return valid ptr
Ref XfrClass::make() {
	if ( this->origin == NULL ) {
		throw std::runtime_error( "Origin was not set during copy" );
	}
	this->cage.top = this->tmptop;
	//	printf( "Origin = %x\n", ToUInt( *RefToPtr4( origin ) ) );
	return Ptr4ToRef( this->origin );
}