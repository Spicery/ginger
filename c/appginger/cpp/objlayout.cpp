#include "objlayout.hpp"

#include "common.hpp"
#include "key.hpp"

unsigned long sizeAfterKeyOfRecord( Ref * key ) {
	return ( ToULong( *key ) & LENGTH_MASK ) >> KIND_WIDTH >> TAGG;
}

unsigned long sizeAfterKeyOfVector( Ref * key ) {
	return ToULong( *( key - 1 ) ) >> TAG;
}

//	Add 1 for null.
unsigned long sizeAfterKeyOfString( Ref * key ) {
	return ( sizeAfterKeyOfVector( key ) + sizeof( long ) - 1 + 1 ) / sizeof( long );
}

unsigned long sizeAfterKeyOfFnLength( Ref * key ) {
	return ToULong( *( key - OFFSET_FROM_FN_LENGTH_TO_KEY ) ) >> TAGGG;
}

void findObjectLimits( Ref * object, Ref * & start, Ref * & end ) {
	
}
