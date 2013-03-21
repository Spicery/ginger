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

#include "key.hpp"
#include "stringlayout.hpp"
#include "mishap.hpp"

namespace Ginger {

class Cell;

class HeapObject {
private:
	Ref * obj_K;

public:
	HeapObject( Ref * _p ) : obj_K( _p ) {}

public:
	bool isString() const {
		return this->obj_K[ 0 ] == sysStringKey;
	}

	std::string asString() const;

	std::string toPrintString() const;

	Cell asCell() const;

	bool isFunctionObject() const { return IsFunctionKey( *this->obj_K ); }
	bool isCoreFunctionObject() const { return IsFunctionKey( *this->obj_K ) && IsCoreFunctionKey( *this->obj_K ); }
	bool isMethodFunctionObject() const { return IsFunctionKey( *this->obj_K ) && IsMethodKey( *this->obj_K ); }
	bool isInstanceObject() const { return IsObj( *this->obj_K ); }
	bool isVectorObject() const { return IsSimple( *this->obj_K ) && KindOfSimpleKey( *this->obj_K ) == VECTOR_KIND; }
	bool isMixedObject() const { return IsSimple( *this->obj_K ) && KindOfSimpleKey( *this->obj_K ) == MIXED_KIND; }
	bool isRecordObject() const { return IsSimple( *this->obj_K ) && KindOfSimpleKey( *this->obj_K ) == RECORD_KIND; }
	bool isPairObject() const { return IsSimple( *this->obj_K ) && KindOfSimpleKey( *this->obj_K ) == PAIR_KIND; }
	bool isMapObject() const { return IsSimple( *this->obj_K ) && KindOfSimpleKey( *this->obj_K ) == MAP_KIND; }
	bool isWRecordObject() const { return IsSimple( *this->obj_K ) && KindOfSimpleKey( *this->obj_K ) == WRECORD_KIND; }
	bool isAtomicWRecordObject() const { return IsSimple( *this->obj_K ) && KindOfSimpleKey( *this->obj_K ) == ATOMIC_WRECORD_KIND; }
	bool isStringObject() const { return IsSimple( *this->obj_K ) && KindOfSimpleKey( *this->obj_K ) == STRING_KIND; }

};

class Cell {
private:
	Ref ref;

public:
	Cell() : ref( SYS_ABSENT ) {}
	Cell( Ref _r ) : ref( _r ) {}

public:

	std::string asString() const {
		return this->asHeapObject().asString();
	}

	HeapObject asHeapObject() const;

	bool isString() const {
		return IsString( this->ref );
	}

	bool isHeapObject() const {
		return IsObj( this->ref );
	}

	std::string toPrintString() const;

	void print( std::ostream & out, bool showing = false ) const;

	void println( std::ostream & out, bool showing = false ) const;
};

} // namspace Ginger

#endif 