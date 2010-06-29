#include "dict.hpp"

Ident DictClass::lookup( const std::string & s ) {
	std::map< std::string, Ident >::iterator it = this->table.find( s );
	return it == this->table.end() ? shared< IdentClass >() : it->second;
}

Ident DictClass::add( const std::string & s ) {
	Ident id = ident_new_global( s );
    return this->table[ s ] = id;
}

Ident DictClass::lookup_or_add( const std::string & c ) {
    Ident id = this->lookup( c );
	if ( not id ) {
    	return this->add( c );
    } else {
    	return id;
    }
}