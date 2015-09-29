#include "item.hpp"
#include <sstream>

bool ItemClass::item_is_prefix() {
    return this->role.IsPrefix();
}

bool ItemClass::item_is_postfix() {
    return this->role.IsPostfix();
}

bool ItemClass::item_is_anyfix() {
    return this->role.IsAnyfix();
}

bool ItemClass::item_is_binary() {
    return this->role.IsBinary();
}

bool ItemClass::item_is_neg_num() {
	std::stringstream ss( this->nameString() );
	int i;
	bool b = ( ss >> i ).fail();
	return not( b ) && i < 0;
}

bool ItemClass::item_is_name() {
	return this->is_name;
}

bool ItemClass::item_is_signed_num() {
	if ( not ( this->tok_type == tokty_int or this->tok_type == tokty_double ) ) return false;
	if ( this->nameString().empty() ) return false;
	const char ch = this->nameString()[ 0 ];
	return ch == '+' or ch == '-';
}
