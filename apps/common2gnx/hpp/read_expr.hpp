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
	bool 		is_postfix_allowed;

private:
	Node prefixProcessing();
	//Node postfixProcessing();
	Node postfixProcessing( Node lhs, Item item, int prec );
	Node readStmntsCheck( TokType fnc );
	Node readStmnts();
	//Node readId();
	Item readIdItem();
	Node readExprCheck( TokType fnc );
	Node readIf( TokType sense, TokType closer );
	Node readSyscall();
	//Node readBindings();
	//Node readConditions();
	Node readFor();
	Node readAtomicExpr();
	Node readLambda();
	Node readDefinition();
	std::string readPkgName();
	void readTryCatch( NodeFactory & ftry );
	Node readTry( const bool try_vs_transaction );

	
public:
	void setPatternMode() { this->pattern_mode = true; }
	void clearPatternMode() { this->pattern_mode = false; }
	void setCStyleMode( const bool flag ) { this->cstyle_mode = flag; }
	bool isPostfixAllowed() { return this->is_postfix_allowed; }

public:
	Node read_expr();
	Node read_query();
	Node read_expr_prec( int prec );
	Node read_opt_expr();
	Node read_opt_expr_prec( int prec );
	void checkSemi();
	void checkToken( TokType fnc );
	void checkPeekToken( TokType fnc );
	bool tryToken( TokType fnc );
	bool tryPeekToken( TokType fnc );
	bool tryName( const char * name );
	
public:
	ReadStateClass( ItemFactory ifact ) :
		item_factory( ifact ),
		pattern_mode( false ),
		cstyle_mode( false ),
		is_postfix_allowed( true )
	{
	}
	
};

typedef ReadStateClass *ReadState;

#endif

