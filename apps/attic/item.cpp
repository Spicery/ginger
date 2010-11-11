#include "item.hpp"
#include "key.hpp"

bool ItemClass::item_is_neg_num() {
    return(
        this->functor == fnc_int &&
        SmallToInt( this->extra ) < 0
    );
}

int ItemClass::item_int() {
    return ToInt( this->extra );
}

bool ItemClass::item_is_prefix() {
    return IsPrefix( this->role );
}

bool ItemClass::item_is_postfix() {
    return IsPostfix( this->role );
}

bool ItemClass::item_is_anyfix() {
    return IsAnyfix( this->role );
}

bool ItemClass::item_is_binary() {
    return IsBinary( this->role );
}

// -- DELETE THE ONES BELOW HERE --
/*
bool item_is_neg_num( Item it ) {
    return(
        it->functor == fnc_int &&
        SmallToInt( it->extra ) < 0
    );
}

int item_int( Item it ) {
    return ToInt( it->extra );
}

bool item_is_prefix( Item it ) {
    return IsPrefix( it->role );
}

bool item_is_postfix( Item it ) {
    return IsPostfix( it->role );
}

bool item_is_anyfix( Item it ) {
    return IsAnyfix( it->role );
}

bool item_is_binary( Item it ) {
    return IsBinary( it->role );
}
*/
