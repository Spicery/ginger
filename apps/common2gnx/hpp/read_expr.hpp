#ifndef READ_EXPR_H
#define READ_EXPR_H

#include "toktype.hpp"
#include "item_factory.hpp"
#include "node.hpp"

class ReadStateClass {

public:
	ItemFactory item_factory;
	bool		pattern_mode;
	
private:
	Node prefix_processing();
	Node postfix_processing();
	Node postfix_processing( Node lhs, Item item, int prec );
	Node read_stmnts_check( TokType fnc );
	Node read_stmnts();
	Node read_id();
	Item read_id_item();
	Node read_expr_check( TokType fnc );
	Node read_if( TokType sense, TokType closer );
	Node read_syscall();
	Node read_bindings();
	Node read_conditions();
	Node read_for();
	Node read_atomic_expr();
	std::string read_pkg_name();
	
public:
	void setPatternMode() { this->pattern_mode = true; }
	void clearPatternMode() { this->pattern_mode = false; }

public:
	Node read_expr();
	Node read_query();
	Node read_expr_prec( int prec );
	Node read_opt_expr();
	Node read_opt_expr_prec( int prec );
	void checkSemi();
	void check_token( TokType fnc );
	bool try_token( TokType fnc );
	bool try_name( const char * name );
	
public:
	ReadStateClass( ItemFactory ifact ) :
		item_factory( ifact ),
		pattern_mode( false )
	{
	}
	
};

typedef ReadStateClass *ReadState;

#endif

