#ifndef ITEM_HPP
#define ITEM_HPP

#include <string>

#include "functor.hpp"
#include "role.hpp"

enum {
	prec_tight	= 	100,
	prec_not	=  1600,
    prec_arrow  =  10000,
    prec_comma  =  20000,
    prec_semi   =  30000,
    prec_max    = 100000
};


class ItemClass {
private:
    std::string 			name;			//	mandatory
    
public:
    Functor 				functor;		//	mandatory
    Role 					role;			//	mandatory
    int 					precedence;
    
public:
	std::string & 	nameString() { return this->name; }
	
public:
	ItemClass( const char *n, Functor f, Role r, int p ) :
		name( n ),
		functor( f ),
		role( r ),
		precedence( p )
	{
	}
	
	ItemClass() :
		name( "" ),
		functor( fnc___fnc_default ),
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
