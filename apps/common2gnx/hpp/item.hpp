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
    
public:
	std::string & 	nameString() { return this->name; }
	
public:
	ItemClass( const char *n, TokType f, Role r, int p ) :
		name( n ),
		tok_type( f ),
		role( r ),
		precedence( p )
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
	int item_int();
	bool item_is_prefix();
	bool item_is_postfix();
	bool item_is_anyfix();
	bool item_is_binary();

};

typedef ItemClass * Item;

extern Item item_default;
extern Item item_unary_op;
extern Item item_binary_op;



#endif

