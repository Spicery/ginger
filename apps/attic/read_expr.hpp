#ifndef READ_EXPR_HPP
#define READ_EXPR_HPP

#include "item_factory.hpp"
#include "term.hpp"

class ReadStateClass {

public:
	ItemFactory item_factory;
	
private:
	Term prefix_processing();
	Term postfix_processing();
	Term postfix_processing( Term lhs, Item item, int prec );
	Term read_stmnts_check( Functor fnc );
	bool try_token( Functor fnc );
	Term read_stmnts();
	Term read_id();
	Term read_opt_expr_prec( int prec );
	void check_token( Functor fnc );
	Term read_expr_check( Functor fnc );
	Term read_if( Functor sense, Functor closer );
	Term read_syscall();
	Term read_bindings();
	Term read_conditions();
	Term read_for();

public:
	Term read_expr();
	Term read_expr_prec(int prec );
	
public:
	ReadStateClass( ItemFactory ifact ) :
		item_factory( ifact )
	{
	}
	
};

typedef ReadStateClass *ReadState;

//typedef struct ReadState *ReadState;

//Term read_expr( ReadState );
//Term read_expr_prec( ReadState, int prec );

#endif
