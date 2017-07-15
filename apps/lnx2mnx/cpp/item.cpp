/******************************************************************************\
	Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
	of the GNU General Public License. This file is part of Ginger.

    Ginger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Ginger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Ginger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

//	C++ STL
#include <iostream>
#include <string>
#include <limits>
#include <sstream>

//	Ginger lib
#include "mishap.hpp"

//	This apps modules.
#include "item.hpp"

namespace LNX2MNX_NS {
using namespace std;
using namespace Ginger;

/*bool ItemClass::item_is_neg_num() {
	std::stringstream ss( this->name );
	int i;
	bool b = ( ss >> i ).fail();
	return not( b ) && i < 0;
}*/

/*bool ItemClass::isLiteralConstant() {
	switch ( this->tok_type ) {
		case tokty_number : 
		case tokty_string : 
			return true;
		default:
			return false;
	}
}

bool ItemClass::isName() {
	return this->tok_type == tokty_name;
}

bool ItemClass::isSign() {
	return this->tok_type == tokty_sign;
}

bool ItemClass::isStringValue() {
	return this->tok_type == tokty_string;
}

bool ItemClass::isIntegerValue() {
	return this->tok_type == tokty_number;
}

bool ItemClass::isAtEnd() {
	return this->tok_type == tokty_end;
}

const std::string ItemClass::asValue() {
	return this->name;
}

const std::string ItemClass::asType() {
	switch ( this->tok_type ) {
		case tokty_number : return "int";
		case tokty_string : return "string";
		default: throw Mishap( "Unreachable" );
    }
}

static float FLOAT_MIN = -numeric_limits<float>::max();

const float ItemClass::asPrecedence() {
	float p = this->name == "+" ? 150.0 : this->name == "*" ? 250.0 : FLOAT_MIN;
	cerr << " calc precedence of item " << name << " as " << p << endl;
	return p;
}

const std::string ItemClass::asFeature( const std::string & feature ) {
	if ( feature == "type" ) {
		return this->asType();
	} else if ( feature == "value" ) {
		return this->asValue();
	} else {
		throw Mishap( "Unrecognised feature name" ).culprit( "Feature", feature );
	}
}*/

void RoleMatch::addNamedRole( const char * role ) {
	this->toktymask.add( role );
}

void RoleMatch::addNamedRole( const std::string & role ) {
	this->toktymask.add( role.c_str() );
}

void RoleMatch::addRole( enum TokenType role ) {
	this->toktymask.add( role );
}

static enum TokenType  asTokType( LnxItem * item ) {
	//	To be done
	// 	return 0;
	return tokty_start;
}

bool RoleMatch::contains( LnxItem * item ) {
	return this->toktymask.contains( asTokType( item ) );
}

} // namespace
