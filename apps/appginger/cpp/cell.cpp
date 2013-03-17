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

#include "cell.hpp"
#include "refprint.hpp"

namespace Ginger {
using namespace std;

std::string HeapObject::asString() const {
	if ( not this->isString() ) {
		throw Ginger::Mishap( "String needed" ).culprit( "Argument", this->toPrintString() );
	}
	char * s = ToChars( &this->obj_K[ 1 ] );
	long b = RefToLong( this->obj_K[ STRING_LAYOUT_OFFSET_LENGTH ] );
	return std::string( s, s + b );
}

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
	return this->asCell().toPrintString();
}

Cell HeapObject::asCell() const {
	return Cell( Ptr4ToRef( this->obj_K ) );
}

} // namespace Ginger