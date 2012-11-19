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
	Node prefixProcessingCore();
	Node postfixProcessing( Node lhs, Item item, int prec );
	Node readStmntsCheck( TokType fnc );
	Node readStmnts();
	Node readCompoundStmnts( bool obrace_read );
	Item readIdItem();
	Node readExprCheck( TokType fnc );
	Node readIf( TokType sense, TokType closer );
	Node readSyscall();
	Node readFor();
	Node readAtomicExpr();
	Node readLambda();
	Node readDefinition();
	Node readVarVal( TokType fnc );
	Node readElement();
	std::string readPkgName();
	void readTryCatch( NodeFactory & ftry );
	Node readTry( const bool try_vs_transaction );
	
public:
	void setPatternMode() { this->pattern_mode = true; }
	void clearPatternMode() { this->pattern_mode = false; }
	void setCStyleMode( const bool flag ) { this->cstyle_mode = flag; }
	bool isPostfixAllowed() { return this->is_postfix_allowed; }
	void reset() { this->is_postfix_allowed = true; }

public:
	Node readExpr();
	Node readQuery();
	Node readExprPrec( int prec );
	Node readOptExpr();
	Node readOptEmptyExpr();
	Node readOptEmptyExprPrec( int prec );
	Node readOptEmptyExprCheck( TokType fnc );
	Node readOptExprPrec( int prec );
	Node readSingleStmnt();
	void checkToken( TokType fnc );
	void checkPeekToken( TokType fnc );
	bool tryToken( TokType fnc );
	bool tryPeekToken( TokType fnc );
	bool tryName( const char * name );
	bool isAtEndOfInput();

	
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

