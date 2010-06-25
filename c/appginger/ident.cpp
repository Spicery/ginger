#include "ident.hpp"

IdentClass::IdentClass( const std::string & nm ) :
	name( nm ),
	is_local( false ),
	slot( -1 ),
	valof( NULL ),
	level( -1 ),
	next( NULL )
{
}

const std::string & IdentClass::getNameString() {
	return this->name;
}

bool IdentClass::isSame( IdentClass * other ) {
	return this->name == other->name;
}

Ident ident_new_local( const std::string & nm ) {
	IdentClass * id = new IdentClass( nm );
	id->is_local = true;
	return shared< IdentClass >( id );
}

Ident ident_new_global( const std::string & nm ) {
	IdentClass * id = new IdentClass( nm );
	id->is_local = false;
	return shared< IdentClass >( id );
}