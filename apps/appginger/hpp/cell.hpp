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

#ifndef CELL_HPP
#define CELL_HPP

#include <string>
#include <ostream>
#include <cstddef>

#include "key.hpp"
#include "stringlayout.hpp"
#include "mishap.hpp"

namespace Ginger {

class HeapObject;
class StringObject;
class VectorObject;
class DoubleObject;
class PairObject;

class Cell {
friend class HeapObject;
private:
	Ref ref;

public:
	Cell() : ref( SYS_ABSENT ) {}
	Cell( Ref _r ) : ref( _r ) {}

public:

	StringObject asStringObject() const;
	HeapObject asHeapObject() const;

	bool isString() const { return IsString( this->ref ); }
	bool isHeapObject() const { return IsObj( this->ref ); }
	bool isPairObject() const { return IsPair( this->ref ); }
	bool isVectorObject() const { return IsVector( this->ref ); }

	std::string toPrintString() const;

	void print( std::ostream & out, bool showing = false ) const;

	void println( std::ostream & out, bool showing = false ) const;

private:
	void dump( MnxBuilder & b, const bool deep ) const;
};


class HeapObject {
friend class Cell;
protected:
	Ref * obj_K;

public:
	HeapObject( Ref * _p ) : obj_K( _p ) {}

public:	//	Generic methods (works on any value)
	std::string toPrintString() const;
	shared< Mnx > toMnx() const;
	Cell deref() const;


public:	//	String methods (only works on strings or is especially relevant to)
	bool isStringObject() const { return IsSimple( *this->obj_K ) && KindOfSimpleKey( *this->obj_K ) == STRING_KIND; }
	StringObject asStringObject() const;
	
public:	//	Vector methods (only works on vectors or is especially relevant to)
	VectorObject asVectorObject() const;
	bool isVectorObject() const { return IsSimple( *this->obj_K ) && KindOfSimpleKey( *this->obj_K ) == VECTOR_KIND; }
	

public:	//	Others
	bool isFunctionObject() const { return IsFunctionKey( *this->obj_K ); }
	bool isCoreFunctionObject() const { return IsFunctionKey( *this->obj_K ) && IsCoreFunctionKey( *this->obj_K ); }
	bool isMethodFunctionObject() const { return IsFunctionKey( *this->obj_K ) && IsMethodKey( *this->obj_K ); }
	bool isInstanceObject() const { return IsObj( *this->obj_K ); }
	bool isMixedObject() const { return IsSimple( *this->obj_K ) && KindOfSimpleKey( *this->obj_K ) == MIXED_KIND; }
	bool isRecordObject() const { return IsSimple( *this->obj_K ) && KindOfSimpleKey( *this->obj_K ) == RECORD_KIND; }
	bool isPairObject() const { return IsSimple( *this->obj_K ) && KindOfSimpleKey( *this->obj_K ) == PAIR_KIND; }
	bool isMapObject() const { return IsSimple( *this->obj_K ) && KindOfSimpleKey( *this->obj_K ) == MAP_KIND; }
	bool isWRecordObject() const { return IsSimple( *this->obj_K ) && KindOfSimpleKey( *this->obj_K ) == WRECORD_KIND; }
	bool isAtomicWRecordObject() const { return IsSimple( *this->obj_K ) && KindOfSimpleKey( *this->obj_K ) == ATOMIC_WRECORD_KIND; }

private:
	void dump( MnxBuilder & b, const bool deep = false ) const;
};

class PairObject : public HeapObject {
public:
	PairObject( Ref * _p ) : HeapObject( _p ) {}

public:
	Cell front() const;
	Cell back() const;
};

class VectorObject : public HeapObject {
public:
	VectorObject( Ref * _p ) : HeapObject( _p ) {}

public:
	Cell index1( ptrdiff_t n ) const;
	long length() const;
};

class StringObject : public HeapObject {
public:
	StringObject( Ref * _p ) : HeapObject( _p ) {}

public:
	char index0( ptrdiff_t n ) const;
	long length() const;
	std::string getString() const;
};

class DoubleObject : public HeapObject {
public:
	DoubleObject( Ref * _p ) : HeapObject( _p ) {}

public:
	std::string toString() const;
	double getDouble() const;
};





} // namspace Ginger

#endif 