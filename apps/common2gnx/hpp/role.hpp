#ifndef ROLE_H
#define ROLE_H

#include "dontknow.hpp"

enum ArityCode {
	DontKnowCode,
	ZeroCode,
	OneCode,
	TwoCode
};

//	This code assumes that DONTKNOW == -1.  I just do not trust the
//	compiler to optimize the code without this assumption.
//
#define ArityToCode( x ) 	( (x) + 1 )
#define CodeToArity( x )    ( (x) - 1 )
#define RoleToInArity( x ) 	CodeToArity( (x) & 3 )
#define RoleToOutArity( x ) CodeToArity( ( (x) >> 2 ) & 3 )

enum FixityCode {
	NofixCode       = 0,
	PrefixCode		= 1,
	PostfixCode     = 2,
	BothfixCode     = 3
};

enum CompileCode {
	NoSpecialCode 		= 0,
	EofCode				= 1,
	LiteralCode			= 2,
	UnusedCompileCode3	= 3,
	FormCode			= 4,	
	SysCode    			= 5,
	UnusedCompileCode6	= 6,
	UnusedCompileCode7	= 7
};

class Role {
public:
	int 				in_arity 		: 4;
	enum FixityCode 	fixity_code		: 4;
	enum CompileCode	compile_code   	: 4;
	
public:
	bool IsCloser()		{ return this->fixity_code == NofixCode && this->compile_code == NoSpecialCode; }

	bool IsPrefix() 	{ return ( this->fixity_code & PrefixCode ) != 0; }
	bool IsPostfix() 	{ return ( this->fixity_code & PostfixCode ) != 0; }
	bool IsBothfix()	{ return this->fixity_code == ( PrefixCode | PostfixCode ); }
	bool IsAnyfix()		{ return this->fixity_code != NofixCode; }

	bool IsUnary() 		{ return this->in_arity == 1; }
	bool IsBinary()		{ return this->in_arity == 2; }
	
	bool IsLiteral() 	{ return this->compile_code == LiteralCode; }
	bool IsForm() 		{ return this->compile_code == FormCode; }
	bool IsSys() 		{ return this->compile_code == SysCode; }
	bool IsEof()		{ return this->compile_code == EofCode; }
};


extern Role IncrByRole;
extern Role DecrByRole;
extern Role AnonRole;
extern Role BinaryFormRole;
extern Role BinarySysRole;
extern Role BothfixRole;
extern Role CharRole;
extern Role CharSeqRole;
extern Role CloserRole;
extern Role EofRole;
extern Role LiteralRole;
extern Role ConstantRole;
extern Role NoRole;
extern Role PostfixRole;
extern Role PrefixOpRole;
extern Role PrefixRole;
extern Role SkipRole;
extern Role StringRole;
extern Role SymbolRole;
extern Role SuffixSysRole;
extern Role UnarySysRole;
extern Role UnaryFormRole;

#endif

