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

#include <ostream>
#include <sstream>

#include "functionlayout.hpp"
#include "vectorlayout.hpp"
#include "listlayout.hpp"
#include "sysdouble.hpp"
#include "mnx.hpp"
#include "shared.hpp"
#include "cell.hpp"
#include "refprint.hpp"
#include "sysprint.hpp"
#include "gnxconstants.hpp"

namespace Ginger {
using namespace std;

Cell PairObject::front() const {
	return Cell( this->obj_K[ PAIR_HEAD_OFFSET ] );
}

Cell PairObject::back() const {
	return Cell( this->obj_K[ PAIR_HEAD_OFFSET ] );
}

std::string DoubleObject::toString() const {
	stringstream s;
	s << this->getDouble();
	return s.str();
}
	
double DoubleObject::getDouble() const {
	return gngFastDoubleValue( this->obj_K );
}


Cell VectorObject::index1( ptrdiff_t n ) const {
	return Cell( indexVectorLayout( this->obj_K, n ) );
}

long VectorObject::length() const {
	return lengthOfVectorLayout( this->obj_K );	
}

VectorObject HeapObject::asVectorObject() const {
	if ( not this->isVectorObject() ) {
		throw Ginger::Mishap( "Vector needed" ).culprit( "Argument", this->toPrintString() );
	}
	return VectorObject( this->obj_K );
}

char StringObject::index0( ptrdiff_t n ) const {
	char * s = ToChars( &this->obj_K[ 1 ] );
	return s[ n ];
}

long StringObject::length() const {
	return RefToLong( this->obj_K[ STRING_LAYOUT_OFFSET_LENGTH ] );
}

std::string StringObject::getString() const {
	char * s = ToChars( &this->obj_K[ 1 ] );
	return std::string( s, s + this->length() );
}

StringObject HeapObject::asStringObject() const {
	if ( not this->isStringObject() ) {
		throw Ginger::Mishap( "String needed" ).culprit( "Argument", this->toPrintString() );
	}
	return StringObject( this->obj_K );
}

StringObject Cell::asStringObject() const {
	if ( not IsString( this->ref ) ) {
		throw Ginger::Mishap( "String needed" ).culprit( "Argument", this->toPrintString() );		
	}
	return StringObject( RefToPtr4( this->ref ) );
}

/*std::string HeapObject::asString() const {
	if ( not this->isStringObject() ) {
		throw Ginger::Mishap( "String needed" ).culprit( "Argument", this->toPrintString() );
	}
	char * s = ToChars( &this->obj_K[ 1 ] );
	long b = RefToLong( this->obj_K[ STRING_LAYOUT_OFFSET_LENGTH ] );
	return std::string( s, s + b );
}*/

/*Cell HeapObject::indexVectorObject( ptrdiff_t n ) const {
	if ( not this->isVectorObject() ) {
		throw Ginger::Mishap( "Vector needed" ).culprit( "Argument", this->toPrintString() );
	}
	return Cell( indexVectorLayout( this->obj_K, n ) );
}*/

/*long HeapObject::lengthVectorObject() const {
	if ( not this->isVectorObject() ) {
		throw Ginger::Mishap( "Vector needed" ).culprit( "Argument", this->toPrintString() );
	}
	return lengthOfVectorLayout( this->obj_K );
}*/

HeapObject Cell::asHeapObject() const {
	if ( not this->isHeapObject() ) {
		throw Ginger::Mishap( "Object needed" ).culprit( "Argument", this->toPrintString() );
	}
	return HeapObject( RefToPtr4( this->ref ) );
}

void Cell::print( ostream & out, bool showing ) const {
	RefPrint printer( out );
	printer.setShowing( showing );
	printer.refPrint( this->ref );
}

void Cell::println( ostream & out, bool showing ) const {
	RefPrint printer( out );
	printer.setShowing( showing );
	printer.refPrint( this->ref );
	out << endl;		
}

std::string Cell::toPrintString() const {
	stringstream str;
	RefPrint printer( str );
	printer.refPrint( this->ref );
	return str.str();
}

std::string HeapObject::toPrintString() const {
	if ( this->obj_K != NULL ) {
		return this->deref().toPrintString();
	} else {
		return "<NULL>";
	}
}

Cell HeapObject::deref() const {
	return Cell( Ptr4ToRef( this->obj_K ) );
}


shared< Mnx > HeapObject::toMnx() const {
	MnxBuilder b;
	this->dump( b );
	return b.build();
}

void Cell::dump( MnxBuilder & b, const bool deep ) const {
	if ( IsSmall( this->ref ) ) {
		b.start( "constant" );
		b.put( "type", "int" );
		b.put( "value", SmallToULong( this->ref ) );
		b.end();
	} else if ( IsObj( this->ref ) ) {
		HeapObject( RefToPtr4( this->ref ) ).dump( b, deep );
	} else {
		b.start( "simple.value.to.be.done" );
		b.end();
	}
}

void HeapObject::dump( MnxBuilder & b, const bool deep ) const {

	/*	Comparison
	bool isMixedObject() const { return IsSimple( *this->obj_K ) && KindOfSimpleKey( *this->obj_K ) == MIXED_KIND; }
	*/

	Ref k = *this->obj_K;
	if ( IsFunctionKey( k ) ) {
		b.start( "function" );
		b.put( "name", nameOfFn( this->obj_K ) );
		b.end();
	} else if ( IsSimpleKey( k ) ) {
		switch ( KindOfSimpleKey( k ) ) {
			case STRING_KIND: {
				StringObject s( this->obj_K );
				b.start( "constant" );
				b.put( "type", "string" );
				b.put( "value", s.getString() );
				b.end();
				break;
			}
			case VECTOR_KIND: {
				VectorObject v( this->obj_K );
				b.start( "vector" );
				if ( deep ) {
					for ( ptrdiff_t i = 1; i <= v.length(); i++ ) {
						v.index1( i ).dump( b, deep );
					}
				}
				b.end();
				break;
			}
			case RECORD_KIND: {
				b.start( "record" );
				if ( deep ) {
					/// @todo iterate over the children.
				}
				b.end();
				break;
			}
			case PAIR_KIND: {
				b.start( "list" );
				if ( deep ) {
					/// @todo iterate over the children.
					Cell c( Ptr4ToRef( this->obj_K ) );
					while ( c.isPairObject() ) {
						PairObject p( RefToPtr4( c.ref ) );
						p.front().dump( b, deep );
						c = p.back();
					}
				}
				b.end();
				break;
			}
			case MAP_KIND: {
				b.start( "map" );
				if ( deep ) {
					/// @todo iterate over the children.
				}
				b.end();
				break;
			}
			default: {
				if ( *this->obj_K == sysDoubleKey ) {
					DoubleObject d( this->obj_K );
					b.start( CONSTANT );
					b.put( "type", "double" );
					b.put( "value", d.toString() );
					b.end();
				} else {
					b.start( "to.be.done" );
					b.end();
				}
			}
		}
	} else if ( IsObj( k ) ) {
		b.start( "instance" );
		if ( deep ) {
			///	@todo Iterate over the children.
		}
		b.end();	
	} else {
		throw Ginger::Mishap( "Invalid object" );
	}
}



} // namespace Ginger