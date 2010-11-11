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
