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

#include "item.hpp"
#include <sstream>
#include "mishap.hpp"

using namespace Ginger;

/*bool ItemClass::item_is_neg_num() {
	std::stringstream ss( this->name );
	int i;
	bool b = ( ss >> i ).fail();
	return not( b ) && i < 0;
}*/

bool ItemClass::isLiteralConstant() {
	switch ( this->tok_type ) {
		case tokty_symbol :
		case tokty_charseq :
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

bool ItemClass::isStringValue() {
	return this->tok_type == tokty_string;
}

bool ItemClass::isCharSeqValue() {
	return this->tok_type == tokty_charseq;
}

bool ItemClass::isIntegerValue() {
	return this->tok_type == tokty_number;
}

bool ItemClass::isOpenList() {
	return this->isSign( '[' );
}

bool ItemClass::isCloseList() {
	return this->isSign( ']' );
}

bool ItemClass::isComma() {
	return this->isSign( ',' );
}
	
bool ItemClass::isSign( char ch ) {
	return this->tok_type == tokty_sign && this->name[0] == ch;
}


const std::string ItemClass::asValue() {
	return this->name;
}

const std::string ItemClass::asType() {
	switch ( this->tok_type ) {
		case tokty_symbol : return "symbol";
		case tokty_number : return "int";
		case tokty_string : return "string";
		default: throw Mishap( "Unreachable" );
    }
}

