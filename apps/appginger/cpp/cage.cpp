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

#include <string.h>
#include <stdio.h>
#include <vector>
#include <stdexcept>

#include <cstdio>
#include <cstdlib>

#include <stddef.h>

#include "heap.hpp"
#include "key.hpp"
#include "mishap.hpp"
#include "machine.hpp"

#include <iostream>

static long cage_id_seq = 0;

CageClass::CageClass( int capacity ) {
	//size_t n = sizeof( Ref ) * capacity;
	//std::cerr << "Allocated a cage of size: " << n << std::endl;
	Ref *data = new Ref[ capacity ];
	this->start = data;
	this->queue_base = this->start;
	this->top = this->start;
	this->end = data + capacity;
	this->cage_id = cage_id_seq++;
}

#define ARBITRARY_SIZE 1048576

CageClass::CageClass() {
	//size_t n = sizeof( Ref ) * ARBITRARY_SIZE;
	Ref *data = new Ref[ ARBITRARY_SIZE ];
	this->start = data;
	this->queue_base = this->start;
	this->top = this->start;
	this->end = data + ARBITRARY_SIZE;
	this->cage_id = cage_id_seq++;
}

bool CageClass::hasEmptyQueue() { 
	return this->queue_base >= this->top; 
}

bool CageClass::hasntEmptyQueue() { 
	return this->queue_base < this->top; 
}


void CageClass::reset() {
	this->top = this->start;
}

void CageClass::resetQueue() {
	this->queue_base = this->start;
}

bool CageClass::isEmpty() {
	return this->top == this->start;
}

XfrClass::XfrClass( CageClass * c ) :
	tmptop( c->top ),
	origin( c->top ),
	cage( c )
{
}

XfrClass::XfrClass( Ref * & pc, HeapClass & h, int preflight ) :
	origin( NULL ),
	cage( h.preflight( pc, preflight ) )
{
	this->tmptop = this->cage->top;
}

XfrClass::XfrClass( Ref * & pc, MachineClass & m, int preflight ) :
	origin( NULL ),
	cage( m.heap().preflight( pc, preflight ) )
{
	this->tmptop = this->cage->top;
}

CageClass::~CageClass() {
	delete this->start;
}

bool CageClass::checkRoom( int k ) {
	return this->end - this->top >= k;
}

bool XfrClass::checkRoom( int k ) {
	return this->cage->end - this->tmptop >= k;
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

//  Copy characters from index a to b of s to an offset from tmptop
void XfrClass::xfrSubstringStep( int offset, const char *s, int a, int b ) {
	memcpy( reinterpret_cast< char * >( this->tmptop ) + offset, s + a, b - a + 1 );
}

void XfrClass::xfrSubstringFinish( int offset ) {
	this->tmptop += ( offset + sizeof( Ref ) - 1 ) / sizeof( Ref );
}

void XfrClass::xfrDup( Ref x, int n ) {
	for ( int i = 0; i < n; i++ ) {
		*( this->tmptop++ ) = x;		
	}
}

void XfrClass::xfrCopy( Ref * words, int n ) {
	memcpy( this->tmptop, words, n * sizeof( Ref ) );
	this->tmptop += n;
}

void XfrClass::xfrCopy( Ref * obj_A, Ref * obj_Z1 ) {
	ptrdiff_t d = obj_Z1 - obj_A;
	if ( d < 0 ) throw "Assertion violation";
	memcpy( this->tmptop, obj_A, d * sizeof( Ref ) );
	this->tmptop += d;
}

//  set the object origin
void XfrClass::setOrigin() {
	this->origin = this->tmptop;
}

Ref * XfrClass::makeRefRef() {
	if ( this->origin == NULL ) {
		throw std::runtime_error( "Origin was not set during copy" );
	}
	this->cage->top = this->tmptop;
	//	printf( "Origin = %x\n", ToUInt( *RefToPtr4( origin ) ) );
	return this->origin;
}

//  return valid ptr
Ref XfrClass::makeRef() {
	return Ptr4ToRef( this->makeRefRef() );
}
