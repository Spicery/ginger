#ifndef IDENT_HPP
#define IDENT_HPP

#include "shared.hpp"

#include <string>
#include <memory>

#include "common.hpp"


class IdentClass {
public:
	std::string		name;
	bool			is_local;
	int				slot; 		//	used for local idents
	Ref				valof;		//	used for global idents
	int 			level;		//	level of scope
	IdentClass		*next;      //	chain used for env linking

public:
	bool isSame( IdentClass * other );
	
public:
	IdentClass( const std::string & nm );
	const std::string & getNameString();
};

typedef shared< IdentClass > Ident;


Ident ident_new_local( const std::string & nm );
Ident ident_new_tmp( int n );
Ident ident_new_global( const std::string & nm );

#endif
