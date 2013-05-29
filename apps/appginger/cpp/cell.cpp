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

#include "externalkind.hpp"
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
#include "sysmap.hpp"
#include "sysequals.hpp"
#include "syskey.hpp"

namespace Ginger {
using namespace std;

// -- Cells --------------------------------------------------------------------

Cell::Cell( HeapObject h ) :
	ref( Ptr4ToRef( h.obj_K ) )
{
}


HeapObject Cell::asHeapObject() const {
	if ( not this->isHeapObject() ) {
		throw Ginger::Mishap( "Object needed" ).culprit( "Argument", this->toPrintString() );
	}
	return HeapObject( RefToPtr4( this->ref ) );
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

void Cell::print( ostream & out, bool showing ) const {
	int column = 0;
	RefPrint printer( out, column, RefPrint::SHOW );
	printer.refPrint( this->ref );
}

void Cell::println( ostream & out, bool showing ) const {
	int column = 0;
	RefPrint printer( out, column, RefPrint::SHOW );
	printer.refPrint( this->ref );
	out << endl;		
}

std::string Cell::toPrintString() const {
	stringstream str;
	int column = 0;
	RefPrint printer( str, column );
	printer.refPrint( this->ref );
	return str.str();
}

std::string Cell::toShowString() const {
	stringstream str;
	int column = 0;
	RefPrint printer( str, column, RefPrint::SHOW );
	printer.refPrint( this->ref );
	return str.str();
}

CharacterCell Cell::asCharacterCell() const {
	if ( IsCharacter( this->ref ) ) {
		return CharacterCell( this->ref );
	} else {
		throw Ginger::Mishap( "Character needed" ).culprit( "Argument", this->toPrintString() );
	}
}



// -- HeapObjects --------------------------------------------------------------

VectorObject HeapObject::asVectorObject() const {
	if ( not this->isVectorObject() ) {
		throw Ginger::Mishap( "Vector needed" ).culprit( "Argument", this->toPrintString() );
	}
	return VectorObject( this->obj_K );
}

MapObject HeapObject::asMapObject() const {
	if ( not this->isMapObject() ) {
		throw Ginger::Mishap( "Map needed" ).culprit( "Argument", this->toPrintString() );
	}
	return MapObject( this->obj_K );
}

MapletObject HeapObject::asMapletObject() const {
	if ( not this->isMapletObject() ) {
		throw Ginger::Mishap( "Maplet needed" ).culprit( "Argument", this->toPrintString() );
	}
	return MapletObject( this->obj_K );
}

StringObject HeapObject::asStringObject() const {
	if ( not this->isStringObject() ) {
		throw Ginger::Mishap( "String needed" ).culprit( "Argument", this->toPrintString() );
	}
	return StringObject( this->obj_K );
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

void HeapObject::dump( MnxBuilder & b, const bool deep ) const {
	Ref k = *this->obj_K;
	if ( IsFunctionKey( k ) ) {
		b.start( "function" );
		b.put( "name", nameOfFn( this->obj_K ) );
		b.end();
	} else if ( IsSimpleKey( k ) ) {
		switch ( KindOfSimpleKey( k ) ) {
			case STRING_KIND: {
				StringObject s( *this );
				b.start( "constant" );
				b.put( "type", "string" );
				b.put( "value", s.getString() );
				b.end();
				break;
			}
			case VECTOR_KIND: {
				VectorObject v( *this );
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
					Cell c = this->upCast();
					while ( c.isPairObject() ) {
						PairObject p( c );
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
					MapObject m( this->obj_K );
					for ( MapObject::Generator g( m ); !!g; ++g ) {
						pair< Cell, Cell > p = *g;
						b.start( "maplet" );
						p.first.dump( b, deep );
						p.second.dump( b, deep );
						b.end();
					}
				}
				b.end();
				break;
			}
			case EXTERNAL_KIND: {
				b.start( "external" );
				b.put( "class", keyName( this->obj_K ) );
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

// -- PairObject ---------------------------------------------------------------

Cell PairObject::front() const {
	return Cell( this->obj_K[ PAIR_HEAD_OFFSET ] );
}

Cell PairObject::back() const {
	return Cell( this->obj_K[ PAIR_HEAD_OFFSET ] );
}

// -- DoubleObject -------------------------------------------------------------

std::string DoubleObject::toString() const {
	stringstream s;
	s << this->getDouble();
	return s.str();
}
	
double DoubleObject::getDouble() const {
	return gngFastDoubleValue( this->obj_K );
}

// -- VectorObject -------------------------------------------------------------

Cell VectorObject::index1( ptrdiff_t n ) const {
	return Cell( indexVectorLayout( this->obj_K, n ) );
}

long VectorObject::length() const {
	return lengthOfVectorLayout( this->obj_K );	
}

bool VectorObjectGenerator::operator !() const {
	return this->idx > this->vector_object.length();
}

Cell VectorObjectGenerator::operator *() const {
	return this->vector_object.index1( this->idx );
}

VectorObjectGenerator & VectorObjectGenerator::operator ++() {
	this->idx += 1;
	return *this;
}



// -- StringObject -------------------------------------------------------------

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

const char * StringObject::getCharPtr() const {
	return ToChars( &this->obj_K[ 1 ] );
}

// -- ExternalObject -----------------------------------------------------------

External * ExternalObject::getExternal() const {
	return reinterpret_cast< External * >( this->obj_K[ EXTERNAL_KIND_OFFSET_VALUE ] );
}

// -- MapletObject -------------------------------------------------------------

Cell MapletObject::key() const {
	return this->obj_K[ MAPLET_KEY_OFFSET ];
}

Cell MapletObject::value() const {
	return this->obj_K[ MAPLET_VALUE_OFFSET ];
}

// -- MapObject ----------------------------------------------------------------


MapObject::Generator::Generator( MapObject m ) :
	current_bucket( NULL ),
	map_crawl( m.obj_K )
{
}

bool MapObject::Generator::operator!() {
	//std::cerr << "calling !: " << ( this->current_bucket == NULL ) << ", " << this->map_crawl.hasBeenCalled() << std::endl;
	if ( this->current_bucket != NULL ) return false;
	if ( not this->map_crawl.hasBeenCalled() ) {
		//std::cerr << "calling (should update next bucket)" << std::endl;
		++*this;
	} 
	return this->current_bucket == NULL;
}

MapObject::Generator & MapObject::Generator::operator ++() {
	this->current_bucket = this->map_crawl.nextBucket();
	return *this;
}

std::pair< Cell, Cell > MapObject::Generator::operator *() const {
	return(
		std::pair< Cell, Cell >( 
			this->current_bucket[ ASSOC_OFFSET_KEY ],
			this->current_bucket[ ASSOC_OFFSET_VALUE ]
		)
	);
}

Cell MapObject::index( Cell c ) const {
	const Ref idx = c.ref;
	Ref * map_K = this->obj_K;
	const Ref map_key = *( this->obj_K );
	
	const long width = fastMapPtrWidth( map_K );
	const bool eq = MapKeyEq( map_key );

	const unsigned long hk = ( eq ? gngEqHash( idx ) : gngIdHash( idx ) ) & ( ( 1 << width ) - 1 );
	const Ref data = this->obj_K[ MAP_OFFSET_DATA ];
	Ref bucket = RefToPtr4( data )[ hk + 1 ];
	
	while ( bucket != SYS_ABSENT ) {
		Ref k1 = fastAssocKey( bucket );
		if ( eq ? refEquals( idx, k1 ) : idx == k1 ) {
			//	Found it.
			return Cell( fastAssocValue( bucket ) );
		} else {
			bucket = fastAssocNext( bucket );
		}
	}

	return Cell( SYS_ABSENT );
}

long MapObject::numBuckets() const {
	return 1 << fastMapPtrWidth( this->obj_K );
}


} // namespace Ginger