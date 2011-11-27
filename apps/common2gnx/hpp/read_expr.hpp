#ifndef READ_EXPR_H
#define READ_EXPR_H

#include "mnx.hpp"
#include "shared.hpp"
typedef shared< Ginger::Mnx > Node;

#include "toktype.hpp"
#include "item_factory.hpp"

typedef Ginger::MnxBuilder NodeFactory;

class ReadStateClass {

public:
	ItemFactory item_factory;
	bool		pattern_mode;
	bool		cstyle_mode;
	
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
	Node read_lambda();
	Node read_definition();
	std::string read_pkg_name();
	void read_try_catch( NodeFactory & ftry );
	Node read_try( const bool try_vs_transaction );


	
public:
	void setPatternMode() { this->pattern_mode = true; }
	void clearPatternMode() { this->pattern_mode = false; }
	void setCStyleMode( const bool flag ) { this->cstyle_mode = flag; }

public:
	Node read_expr();
	Node read_query();
	Node read_expr_prec( int prec );
	Node read_opt_expr();
	Node read_opt_expr_prec( int prec );
	void checkSemi();
	void check_token( TokType fnc );
	void check_peek_token( TokType fnc );
	bool try_token( TokType fnc );
	bool try_peek_token( TokType fnc );
	bool try_name( const char * name );
	
public:
	ReadStateClass( ItemFactory ifact ) :
		item_factory( ifact ),
		pattern_mode( false ),
		cstyle_mode( false )
	{
	}
	
};

typedef ReadStateClass *ReadState;

#endif

