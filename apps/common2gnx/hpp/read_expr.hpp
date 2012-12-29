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
	bool		span_mode;
	
private:
	bool 		is_postfix_allowed;

private:
	Node prefixProcessing();
	Node prefixProcessingCore();
	Node postfixProcessing( Node lhs, Item item, int prec );
	Node readStmntsCheck( TokType fnc );
	Node readStmnts();
	Node readCompoundStmnts();
	Node readCompoundCore();
	Node readCompoundCoreCheck( TokType closer );
	const std::string readIdName();
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
	Node readListOrVector( const bool vector_vs_list, TokType closer );
	Node readMap( TokType closer );
	Node readSwitch();
	Node readSwitchStmnts();
	Node readThrow();
	Node readId( const std::string item );
	Node readEnvVar();
	Node readDHat();
	Node readPackage();
	Node readImport();
	Node readReturn();
	Node readRecordClass();

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
	bool tryPeekCloser();
	bool tryPeekToken( TokType fnc );
	bool tryName( const char * name );
	bool isAtEndOfInput();

	
public:
	ReadStateClass( ItemFactory ifact, bool span_mode ) :
		item_factory( ifact ),
		pattern_mode( false ),
		cstyle_mode( false ),
		span_mode( span_mode ),
		is_postfix_allowed( true )
	{
	}
	
};

typedef ReadStateClass *ReadState;

#endif

