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

