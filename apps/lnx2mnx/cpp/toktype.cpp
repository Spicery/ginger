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

//	C++ STL
#include <string>
//#include <iostream>

//	AppGinger lib
#include "mishap.hpp"


#include "toktype.hpp"

namespace LNX2MNX_NS {
using namespace std;
using namespace Ginger;

const char *tok_type_name( TokenType fnc ) {
    switch ( fnc ) {
		case tokty_end: return "end";
		case tokty_name: return "name";
		case tokty_sign: return "sign";
		case tokty_number : return "number";
		case tokty_string : return "string";
		default: return "?";
    }
    throw "Unreachable";
}

enum TokenType name_to_tok_type( const char * nm ) {
	string name( nm );
	if ( name == "end" ) return tokty_end;
	if ( name == "name" ) return tokty_name;
	if ( name == "sign" ) return tokty_sign;
	if ( name == "number" ) return tokty_number;
	if ( name == "string" ) return tokty_string;
	throw Mishap( "name_to_tok_type: Unrecognised name" ).culprit( "Name", nm );
}

void TokenTypeMask::add( enum TokenType ty ) {
	this->mask |= ( 1 << ty );
}

void TokenTypeMask::add( const char * name ) {
	this->add( string( name ) );
}

void TokenTypeMask::add( const std::string & name ) {
	if ( name == "literal" ) {
		this->add( tokty_number );
		this->add( tokty_string );
	} else {
		this->add( name_to_tok_type( name.c_str() ) );
	}
}

bool TokenTypeMask::contains( enum TokenType ty ) {
	//std::cout << "Comparing mask " << this->mask << " with " << ( 1 << ty ) << " = " << (this->mask & ( 1 << ty )) << std::endl;
	return ( this->mask & ( 1 << ty ) ) != 0;
}

} // namespace
