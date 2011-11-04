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

#ifndef CAGE_HPP
#define CAGE_HPP

#include <vector>
#include <cstddef>
#include "common.hpp"

class MachineClass;
class HeapClass;
class CageClass;

class XfrClass {
private:
    Ref *			tmptop;
	Ref	*			origin;
	CageClass * 	cage;

public:
	bool checkRoom( int );
	void xfrRef( Ref x );
	void xfrDup( Ref x, int n );
	void xfrVector( std::vector< Ref > & chain );
	void xfrSubstring( const char *s, int a, int b );
	void xfrSubstringStep( int offset, const char *s, int a, int b );
	void xfrSubstringFinish( int offset );
	void xfrCopy( Ref *p, int n );
	void xfrCopy( Ref * obj_A, Ref * obj_Z1 );

	void setOrigin();      						//  set the object origin
	Ref makeRef();              				//  return valid ptr
	Ref * makeRefRef();
	

public:
	XfrClass( CageClass * c );
	XfrClass( Ref * & pc, MachineClass & machine, int size );
	XfrClass( Ref * & pc, HeapClass & machine, int size );
};

class CageClass {
friend class CageCrawl;
friend class ToSpaceCopier;
private:
	long				cage_id;
	Ref				*	start;
	Ref				*	queue_base;
    Ref         	*	top;
    Ref         	*	end;
    
public:
	bool checkRoom( int );
	
public:	
	ptrdiff_t nboxesInUse() { return this->top - this->start; }
	ptrdiff_t capacity() { return this->end - this->start; }
	void reset();
	void resetQueue();
	bool isEmpty();
	bool hasEmptyQueue();
	bool hasntEmptyQueue();
	
public:
	long serialNumber() { return this->cage_id; }
	
public:
	friend class XfrClass;
	CageClass( int capacity );
	CageClass();
	~CageClass();
};

typedef class MachineClass * Machine;
typedef class CageClass *Cage;

#endif

