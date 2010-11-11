#ifndef CORD_H
#define CORD_H

#include <string>

#include <string.h>
#include "common.hpp"


class CordClass {
public:
	const std::string 	text;
	unsigned int		hashcode;
	
public:
	CordClass( const std::string & s ) :
		text( s )
	{
	}

	CordClass( const char * t ) : 
		text( t )
	{
	}
	
	CordClass( CordClass *c ) :
		text( c->text )
	{
	}
	
public: 	
	size_t length() {
		return this->text.size();
	}
	
	const std::string & textString() {
		return this->text;
	}
	
	const char * cstring() {
		return this->text.c_str();
	}
	
	unsigned int hash() {
		return this->hashcode;
	}
	
	bool eq( CordClass * y ) {
		return(
			this->hashcode == y->hashcode ?
			this->text.compare( y->text ) :
			false
		);
	}
	
	bool eq( CordClass & y ) {
		return(
			this->hashcode == y.hashcode ?
			this->text.compare( y.text ) :
			false
		);
	}
};

typedef CordClass * Cord;

#endif