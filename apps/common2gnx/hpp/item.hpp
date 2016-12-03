#ifndef ITEM_HPP
#define ITEM_HPP

#include <string>

#include "toktype.hpp"
#include "role.hpp"
#include "precedences.hpp"



class ItemClass {
private:
    std::string 			name;			//	mandatory
    
public:
    TokType 				tok_type;		//	mandatory
    Role 					role;			//	mandatory
    int 					precedence;
  	bool					is_name;
    
public:
	std::string & 	nameString() { return this->name; }
	
public:
	ItemClass( const char *n, TokType f, Role r, int p ) :
		name( n ),
		tok_type( f ),
		role( r ),
		precedence( p ),
		is_name( false )
	{
	}
	
	ItemClass() :
		name( "" ),
		tok_type( tokty___default ),
		role( PrefixRole ),
		precedence( 0 )
	{
	}

public:
	bool item_is_neg_num();
	bool item_is_signed_num();
	int item_int();
	bool item_is_prefix();
	bool item_is_postfix();
	bool item_is_anyfix();
	bool item_is_binary();
	bool item_is_name();
	bool item_is_eof() { return this->tok_type == tokty_eof; }
};

typedef ItemClass * Item;

extern Item item_default;
extern Item item_unary_op;
extern Item item_binary_op;



#endif

