#ifndef ITEM_HPP
#define ITEM_HPP

#include <string>

#include "common.hpp"
#include "functor.hpp"
#include "role.hpp"
#include "instruction.hpp"

enum {
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
    Ref						extra;
    
public:
	std::string & 	nameString() { return this->name; }
	
public:
	ItemClass( const char *n, Functor f, Role r, int p, Ref e ) :
		name( n ),
		functor( f ),
		role( r ),
		precedence( p ),
		extra( e )
	{
	}
	
	ItemClass() :
		name( "" ),
		functor( fnc___fnc_default ),
		role( PrefixRole ),
		precedence( 0 ),
		extra( (Ref)0 )
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

// REPLACE ALL THESE
//bool item_is_neg_num( Item it );
//int item_int( Item it );
//bool item_is_prefix( Item it );
//bool item_is_postfix( Item it );
//bool item_is_anyfix( Item it );
//bool item_is_binary( Item it );

//Item in_word_set( const char *str, unsigned int len );

#endif
