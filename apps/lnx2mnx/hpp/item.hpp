#ifndef ITEM_HPP
#define ITEM_HPP

#include <iostream>
#include <string>

#include "toktype.hpp"

namespace XSON2GNX {


class ItemClass {
friend class ItemFactoryClass;
friend class RoleMatch;
private:
    std::string 			name;
    TokenType				tok_type;

public:
	ItemClass( const char *n, TokenType f ) :
		name( n ),
		tok_type( f )
	{}
	
	ItemClass() :
		name( "" ),
		tok_type( tokty_start )
	{}
	
	ItemClass( ItemClass & that ) :
		name( that.name ), 
		tok_type( that.tok_type )
	{}
	
	~ItemClass() {
		//std::cout << "Deleting Item" << std::endl;
	}
	
public:
	bool isName();
	bool isSign();
	bool isStringValue();
	bool isIntegerValue();
	bool isLiteralConstant();
	bool isAtEnd();
	bool hasRole( unsigned int role );
	const std::string asValue();
	const std::string asType();
	const float asPrecedence();
	const std::string asFeature( const std::string & feature );
};

typedef ItemClass * Item;

const char *name_to_tok_type( enum TokenType ty );

class RoleMatch {
private:
	TokenTypeMask toktymask;
public:
	void addNamedRole( const char * role );
	void addNamedRole( const std::string & role );
	void addRole( enum TokenType role );
	bool contains( const Item & item );
};


} // namespace.


#endif

