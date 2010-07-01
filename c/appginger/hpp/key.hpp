#ifndef KEY_HPP
#define KEY_HPP

#include "common.hpp"

// zzz.yyy.xx tagging scheme.

////////////////////////////////////////////////////////////////////////
// ------xx tags
////////////////////////////////////////////////////////////////////////

#define TAG             2
#define TAG_MASK		0x3
#define INT_TAG         0
#define PTR_TAG         1
#define DEF_TAG         2
#define SIM_TAG         3

#define RefToPtr4( r )  	ToRefRef( ToULong( r ) & ~PTR_TAG )
#define Ptr4ToRef( p ) 		ToRef( ToULong( p ) | PTR_TAG )

#define IntToRef( i )		( (Ref)( ToULong( i ) << TAG | INT_TAG ) )
#define IntToSmall( i ) 	( (Ref)( ToULong( i ) << TAG | INT_TAG ) )
#define RefToLong( r )   	( (long)( ToLong( r ) >> TAG ) )
#define SmallToLong( r )	( (long)( ToLong( r ) >> TAG ) )

#define IsPtr4( r )			( ( TAG_MASK & ToULong( r ) ) == PTR_TAG )
#define IsDefer( r )		( ( TAG_MASK & ToULong( r ) ) == DEF_TAG )

#define IntToChar( i )		( (Ref)( ( i ) << TAGGG | CHAR_TAGGG ) )



////////////////////////////////////////////////////////////////////////
// ---yyy11 tags
////////////////////////////////////////////////////////////////////////
//  Experimentation with current architectures shows that you can get
//  expect good performance if you are working with 5-bit constants.
//  The 2 questionable members of this set are RGB constants and
//  deferrals.
////////////////////////////////////////////////////////////////////////

#define TAGG            5
#define TAGG_MASK		0x1F
#define FLOAT_MASK		0xF
#define sys_absent      ToRef( 0 << TAG | SIM_TAG )
#define sys_false       ToRef( 1 << TAG | SIM_TAG )
#define sys_true		ToRef( 2 << TAG | SIM_TAG )
#define FN_TAGG       	( 3 << TAG | SIM_TAG )
#define KEY_TAGG        ( 4 << TAG | SIM_TAG )
#define ESC_TAGG      	( 5 << TAG | SIM_TAG )
#define FLOAT0_TAGG    	( 6 << TAG | SIM_TAG )
#define FLOAT1_TAGG     ( 7 << TAG | SIM_TAG )

//	Floats
#define IsFloat( x )	( ( FLOAT_MASK & ToULong( x ) ) == FLOAT0_TAGG )

//	Functions
#define sysFunctionKey		ToRef( 0 << TAGG | FN_TAGG )
#define IsFunction( x ) 	( ( TAGG_MASK & ToULong( x ) ) == FN_TAGG )


////////////////////////////////////////////////////////////////////////
// ??zzz01111 tags: Simple Keys
////////////////////////////////////////////////////////////////////////
//  It is unclear how many of these we can reasonably have and remain
//  efficient.  I think the convenience of having as many as we please
//  probably outweighs any small efficiency loss.
////////////////////////////////////////////////////////////////////////

#define sysAbsentKey    ToRef( 0 << TAGG | KEY_TAGG )
#define sysBoolKey      ToRef( 1 << TAGG | KEY_TAGG )
#define sysTerminKey    ToRef( 2 << TAGG | KEY_TAGG )
#define sysNilKey       ToRef( 3 << TAGG | KEY_TAGG )
#define sysPairKey      ToRef( 4 << TAGG | KEY_TAGG )
#define sysVectorKey    ToRef( 5 << TAGG | KEY_TAGG )
#define sysStringKey    ToRef( 6 << TAGG | KEY_TAGG )
#define sysWordKey      ToRef( 7 << TAGG | KEY_TAGG )
#define sysSmallKey		ToRef( 8 << TAGG | KEY_TAGG )
#define sysFloatKey		ToRef( 9 << TAGG | KEY_TAGG )
#define sysKeyKey		ToRef( 10 << TAGG | KEY_TAGG )
#define sysUnicodeKey	ToRef( 11 << TAGG | KEY_TAGG )
#define sysCharKey		ToRef( 12 << TAGG | KEY_TAGG )

//	Lists
#define IsPair( x )		( IsPtr4( x ) && ( *RefToPtr4( x ) == sysPairKey ) )


////////////////////////////////////////////////////////////////////////
// zzz11111 tags
////////////////////////////////////////////////////////////////////////
//  I am inclined to reserve these for 24-bit quantities if possible.
////////////////////////////////////////////////////////////////////////

#define TAGGG           8
#define TAGGG_MASK		0xFF
#define MISC_TAGGG      ( 0 << TAGG | ESC_TAGG )
#define RGB_TAGGG   	( 1 << TAGG | ESC_TAGG )
#define CHAR_TAGGG   	( 2 << TAGG | ESC_TAGG )
#define FUNC_LEN_TAGGG  ( 3 << TAGG | ESC_TAGG )
#define UNUSED4_TAGGG   ( 4 << TAGG | ESC_TAGG )
#define UNUSED5_TAGGG   ( 5 << TAGG | ESC_TAGG )
#define UNUSED6_TAGGG   ( 6 << TAGG | ESC_TAGG )
#define UNUSED7_TAGGG   ( 7 << TAGG | ESC_TAGG )

#define CharToCharacter( c ) \
	ToRef( (c) << TAGGG | CHAR_TAGGG )

#define CharacterToChar( r ) \
	( (char)( ToULong(r) >> TAGGG ) )


////////////////////////////////////////////////////////////////////////
// Elaborate constants that do not have to be efficient
//	00010111 tags
////////////////////////////////////////////////////////////////////////

#define sys_nil			ToRef( 0 << TAGGG | MISC_TAGGG )
#define sys_underflow	ToRef( 1 << TAGGG | MISC_TAGGG )
#define sys_undefined	ToRef( 2 << TAGGG | MISC_TAGGG )

//	Nil
#define IsNil( x )		( x == sys_nil )


////////////////////////////////////////////////////////////////////////



#endif
