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

#ifndef GINGER_CELL_HPP
#define GINGER_CELL_HPP

#include <algorithm>
#include <string>
#include <ostream>
#include <cstddef>

#include "shared.hpp"
#include "key.hpp"
#include "stringlayout.hpp"
#include "mishap.hpp"
#include "mapcrawl.hpp"
#include "external.hpp"
#include "bigint.hpp"
#include "rational.hpp"

namespace Ginger {

class MnxBuilder;
class Mnx;

class HeapObject;
class StringObject;
class VectorObject;
class DoubleObject;
class BigIntObject;
class RationalObject;
class PairObject;
class MapObject;
class MapletObject;
class ExternalObject;
class VirtualMachineObject;
class CharacterCell;

class Cell {
friend class HeapObject;
friend class MapObject;
protected:
	Ref ref;

public:
	Cell() : ref( SYS_ABSENT ) {}
	Cell( Ref _r ) : ref( _r ) {}
	Cell( HeapObject _h );

public:

	HeapObject asHeapObject() const;
	Ref asRef() const { return this->ref; }
	long getLong() const { return SmallToLong( this->ref ); }
	bool isSimple() const { return IsSimple( this->ref ); }
	bool isPositive() const { return ToLong( this->ref ) > 0; }
	bool isNegative() const { return ToLong( this->ref ) < 0; }
	bool isZero() const { return ToLong( this->ref ) == 0; }
	DoubleObject asDoubleObject() const;
	BigIntObject asBigIntObject() const;
	RationalObject asRationalObject() const;
	ExternalObject asExternalObject() const;
	StringObject asStringObject() const;


	bool isTermin() const { return this->ref == SYS_TERMIN; }
	bool isAbsent() const { return this->ref == SYS_ABSENT; }
	bool isSmall() const { return IsSmall( this->ref ); }
	bool isCharacter() const { return IsCharacter( this->ref ); }
	CharacterCell asCharacterCell() const;
	bool isStringObject() const { return IsString( this->ref ); }
	bool isHeapObject() const { return IsObj( this->ref ); }
	bool isPairObject() const { return IsPair( this->ref ); }
	bool isVectorObject() const { return IsVector( this->ref ); }
	bool isDoubleObject() const { return IsDouble( this->ref ); }
	bool isBigIntObject() const { return IsBigInt( this->ref ); }
	bool isRationalObject() const { return IsRational( this->ref ); }

	std::string toPrintString() const;
	std::string toShowString() const;
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
	HeapObject( Cell _c ) : obj_K( RefToPtr4( _c.ref ) ) {}

public:	//	Generic methods (works on any value)
	std::string toPrintString() const;
	shared< Mnx > toMnx() const;
	Cell deref() const;
	Cell upCast() const { return Cell( Ptr4ToRef( this->obj_K ) ); }

public:	//	Checked downcasts.
	StringObject asStringObject() const;
	VectorObject asVectorObject() const;
	MapObject asMapObject() const;	
	MapletObject asMapletObject() const;	
	DoubleObject asDoubleObject() const;
	BigIntObject asBigIntObject() const;
	RationalObject asRationalObject() const;
	ExternalObject asExternalObject() const;
	VirtualMachineObject asVirtualMachineObject() const;

public:	//	Others
	bool isFunctionObject() const { return IsFunctionKey( *this->obj_K ); }
	bool isCoreFunctionObject() const { return IsFunctionKey( *this->obj_K ) && IsCoreFunctionKey( *this->obj_K ); }
	bool isMethodFunctionObject() const { return IsFunctionKey( *this->obj_K ) && IsMethodKey( *this->obj_K ); }
	bool isStringObject() const { return IsSimple( *this->obj_K ) && KindOfSimpleKey( *this->obj_K ) == STRING_KIND; }
	bool isVectorObject() const { return IsSimple( *this->obj_K ) && KindOfSimpleKey( *this->obj_K ) == VECTOR_KIND; }
	bool isInstanceObject() const { return IsObj( *this->obj_K ); }
	bool isMixedObject() const { return IsSimple( *this->obj_K ) && KindOfSimpleKey( *this->obj_K ) == MIXED_KIND; }
	bool isRecordObject() const { return IsSimple( *this->obj_K ) && KindOfSimpleKey( *this->obj_K ) == RECORD_KIND; }
	bool isPairObject() const { return IsSimple( *this->obj_K ) && KindOfSimpleKey( *this->obj_K ) == PAIR_KIND; }
	bool isMapObject() const { return IsSimple( *this->obj_K ) && KindOfSimpleKey( *this->obj_K ) == MAP_KIND; }
	bool isMapletObject() const { return IsMaplet( this->obj_K ); }
	bool isWRecordObject() const { return IsSimple( *this->obj_K ) && KindOfSimpleKey( *this->obj_K ) == WRECORD_KIND; }
	bool isDoubleObject() const { return *this->obj_K == sysDoubleKey; }
	bool isBigIntObject() const { return *this->obj_K == sysBigIntKey; }
	bool isRationalObject() const { return *this->obj_K == sysRationalKey; }
	bool isAtomicWRecordObject() const { return IsSimple( *this->obj_K ) && KindOfSimpleKey( *this->obj_K ) == ATOMIC_WRECORD_KIND; }
	bool isExternalObject() const { return IsSimple( *this->obj_K ) && KindOfSimpleKey( *this->obj_K ) == EXTERNAL_KIND; }
	bool isVirtualMachineObject() const { return *this->obj_K == sysVirtualMachineKey; }

private:
	void dump( MnxBuilder & b, const bool deep = false ) const;
};

class PairObject : public HeapObject {
public:
	PairObject( Ref * _p ) : HeapObject( _p ) {}
	PairObject( HeapObject _h ) : HeapObject( _h ) {}
	PairObject( Cell _c ) : HeapObject( _c ) {}

public:
	Cell front() const;
	Cell back() const;
};

class VectorObject : public HeapObject {
public:
	typedef class VectorObjectGenerator generator;
public:
	VectorObject( Ref * _p ) : HeapObject( _p ) {}
	VectorObject( HeapObject _h ) : HeapObject( _h ) {}
	VectorObject( Cell _c ) : HeapObject( _c ) {}

public:
	Cell index1( ptrdiff_t n ) const;
	long length() const;
};

class VectorObjectGenerator {
private:
	int idx;
	class VectorObject vector_object;
public:
	VectorObjectGenerator( VectorObject v ) : idx( 1 ), vector_object( v ) {}
	bool operator !() const;
	Cell operator *() const;
	VectorObjectGenerator & operator ++();
};


class StringObject : public HeapObject {
public:
	StringObject( Ref * _p ) : HeapObject( _p ) {}
	StringObject( HeapObject _h ) : HeapObject( _h ) {}
	StringObject( Cell _c ) : HeapObject( _c ) {}

public:
	char index0( ptrdiff_t n ) const;
	long length() const;
	std::string getString() const;
	const char * getCharPtr() const;
};

class DoubleObject : public HeapObject {
public:
	DoubleObject( Ref * _p ) : HeapObject( _p ) {}
	DoubleObject( HeapObject _h ) : HeapObject( _h ) {}
	DoubleObject( Cell _c ) : HeapObject( _c ) {}

public:
	std::string toString() const;
	gngdouble_t getDouble() const;
};

class BigIntObject : public HeapObject {
public:
	BigIntObject( Ref * _p ) : HeapObject( _p ) {}
	BigIntObject( HeapObject _h ) : HeapObject( _h ) {}
	BigIntObject( Cell _c ) : HeapObject( _c ) {}

public:
	std::string toString() const;
	BigIntExternal * getBigIntExternal() const;
};

class RationalObject : public HeapObject {
public:
	RationalObject( Ref * _p ) : HeapObject( _p ) {}
	RationalObject( HeapObject _h ) : HeapObject( _h ) {}
	RationalObject( Cell _c ) : HeapObject( _c ) {}

public:
	std::string toString() const;
	RationalExternal * getRationalExternal() const;
};

class MapObject : public HeapObject {
public:
	class Generator {
	private:
		Ref * current_bucket;
		MapCrawl map_crawl;
	public:
		Generator( MapObject m );
		bool operator!();
		Generator & operator ++();
		std::pair< Cell, Cell > operator *() const;
	};
private:
	long numBuckets() const;
	
public:
	MapObject( Ref * _p ) : HeapObject( _p ) {}
	MapObject( HeapObject _h ) : HeapObject( _h ) {}
	MapObject( Cell _c ) : HeapObject( _c ) {}

public:
	Cell index( Cell c ) const;
	Generator & entries();
};

class MapletObject : public HeapObject {
public:
	MapletObject( Ref * _p ) : HeapObject( _p ) {}
	MapletObject( HeapObject _h ) : HeapObject( _h ) {}
	MapletObject( Cell _c ) : HeapObject( _c ) {}
public:
	Cell key() const;
	Cell value() const;
};

class ExternalObject : public HeapObject {
public:
	ExternalObject( Ref * _p ) : HeapObject( _p ) {}
	ExternalObject( HeapObject _h ) : HeapObject( _h ) {}
	ExternalObject( Cell _c ) : HeapObject( _c ) {}

public:
	External * getExternal() const;
};

class VirtualMachineObject : public ExternalObject {
public:
	VirtualMachineObject( Ref * _p ) : ExternalObject( _p ) {}
	VirtualMachineObject( HeapObject _h ) : ExternalObject( _h ) {}
	VirtualMachineObject( Cell _c ) : ExternalObject( _c ) {}

public:
	class MachineClass * getExternal() const;
};

class CharacterCell : public Cell {
public:
	CharacterCell() : Cell() {}
	CharacterCell( Ref _r ) : Cell( _r ) {}
	CharacterCell( HeapObject _h ) : Cell( _h ) {}

public:
	char getChar() const { return CharacterToChar( this->ref ); }
};



} // namespace Ginger

#endif 
