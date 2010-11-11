#ifndef ROLE_HPP
#define ROLE_HPP

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

enum SyntaxCode {
	NofixCode       = 0,
	PrefixCode		= 1,
	PostfixCode     = 2,
	BothfixCode     = 3
};

enum SemanticCode {
	NoSpecialCode 	= 0,
	EofCode			= 1,
	ImmediateCode	= 2,
	UnusedSemCode	= 3,
	InLineCode		= 4,	
	UnusedSemCode5	= 5,
	UnusedSemCode6	= 6,
	UnusedSemCode7	= 7
};

#define MakeRole( in, out, syn, sem ) \
	( ( (in)+1) | ( ((out)+1) << 2 ) | ( (syn) << 4 ) | ( (sem) << 6 ) )

#define MakeRoleMask( a, b, c, d ) \
	( a ? 3 : 0 ) | ( b ? 3 << 2 : 0 ) | ( c ? 3 << 4 : 0 ) | ( d ? 7 << 6 : 0 )

#define SynPart( r )	( ( (r) >> 4 ) & 3 )
#define SemPart( r )	( ( (r) >> 6 ) & 7 )


typedef enum Role {
	AddOpRole		= MakeRole( 2, 1, PostfixCode, NoSpecialCode ),
	IncrByRole		= MakeRole( 1, 1, PrefixCode, NoSpecialCode ),
	DecrByRole		= MakeRole( 1, 1, PrefixCode, NoSpecialCode ),
	AnonRole		= MakeRole( 0, 1, PrefixCode, NoSpecialCode ),
	BinaryOpRole 	= MakeRole( 2, 1, PostfixCode, NoSpecialCode ),
	BothfixRole 	= MakeRole( DONTKNOW, DONTKNOW, BothfixCode, NoSpecialCode ),
	CharRole		= MakeRole( 0, 1, PrefixCode, ImmediateCode ),
	CharSeqRole		= MakeRole( 0, DONTKNOW, PrefixCode, NoSpecialCode ),
	CloserRole		= MakeRole( DONTKNOW, DONTKNOW, NofixCode, NoSpecialCode ),
	EofRole			= MakeRole( DONTKNOW, DONTKNOW, NofixCode, EofCode ),
	ImmediateRole	= MakeRole( 0, 1, PrefixCode, ImmediateCode ),
	InLineRole 		= MakeRole( DONTKNOW, DONTKNOW, PrefixCode, InLineCode ),
	NoRole			= MakeRole( DONTKNOW, DONTKNOW, NofixCode, NoSpecialCode ),
	PostfixRole     = MakeRole( DONTKNOW, DONTKNOW, PostfixCode, NoSpecialCode ),
	PrefixOpRole	= MakeRole( 1, 1, PrefixCode, NoSpecialCode ),
	PrefixRole 		= MakeRole( DONTKNOW, DONTKNOW, PrefixCode, NoSpecialCode ),
	SkipRole		= MakeRole( 0, 0, PrefixCode, NoSpecialCode ),
	StringRole		= MakeRole( 0, 1, PrefixCode, NoSpecialCode ),
	SuffixOpRole	= MakeRole( 1, 1, PostfixCode, NoSpecialCode ),
} Role;

#define IsPrefix(r) 		( ( SynPart( r ) & PrefixCode ) == PrefixCode )
#define IsPostfix(r)  		( ( SynPart( r ) & PostfixCode ) == PostfixCode )
#define IsBothfix(r)		( SynPart( r ) == ( PrefixCode | PostfixCode ) )
#define IsAnyfix(r)			( SynPart( r ) != 0 )
#define IsUnary(r) 			( ( RoleToInArity( r ) == 1 ) && IsPrefix( r ) )
#define IsBinary(r)			( ( RoleToInArity( r ) == 2 ) && IsPostfix( r ) )

#define IsInLine(r) 		( SemPart( r ) == InLineCode )
#define IsImmediate(r) 		( SemPart( r ) == ImmediateCode )

#endif
