#ifndef KEY_HPP
#define KEY_HPP

#include "common.hpp"

bool isKey( Ref x );
const char * keyName( Ref key );


////////////////////////////////////////////////////////////////////////
// ------.xx tags
////////////////////////////////////////////////////////////////////////

#define TAG             2
#define TAG_MASK		0x3
#define INT_TAG         0
#define PTR_TAG         1
#define DEF_TAG         2
#define SIM_TAG         3

#define RefToPtr4( r )  	ToRefRef( ToULong( r ) & ~PTR_TAG )
#define Ptr4ToRef( p ) 		ToRef( ToULong( p ) | PTR_TAG )



#define LongToRef( i )		( (Ref)( ToULong( i ) << TAG | INT_TAG ) )
#define LongToSmall( i ) 	( (Ref)( ToULong( i ) << TAG | INT_TAG ) )
#define RefToLong( r )   	( (long)( ToLong( r ) >> TAG ) )
#define SmallToLong( r )	( (long)( ToLong( r ) >> TAG ) )

#define IsPtr4( r )			( ( TAG_MASK & ToULong( r ) ) == PTR_TAG )
#define IsSmall( r )		( ( TAG_MASK & ToULong( r ) ) == INT_TAG )
#define IsDefer( r )		( ( TAG_MASK & ToULong( r ) ) == DEF_TAG )

#define IntToChar( i )		( (Ref)( ( i ) << TAGGG | CHAR_TAGGG ) )



////////////////////////////////////////////////////////////////////////
// ---.yyy.11 tags
////////////////////////////////////////////////////////////////////////
//  Experimentation with current architectures shows that you can get
//  expect good performance if you are working with 5-bit constants.
//  The 2 questionable members of this set are RGB constants and
//  deferrals.
////////////////////////////////////////////////////////////////////////

#define TAGG            5
#define TAGG_MASK				0x1F
#define FLOAT_MASK				0xF
#define sys_absent     		 	ToRef( 0 << TAG | SIM_TAG )
#define sys_false      			ToRef( 1 << TAG | SIM_TAG )
#define sys_true				ToRef( 2 << TAG | SIM_TAG )
#define FN_TAGG       			( 3 << TAG | SIM_TAG )
#define KEY_TAGG        		( 4 << TAG | SIM_TAG )
#define ESC_TAGG      			( 5 << TAG | SIM_TAG )
#define FLOAT0_TAGG    			( 6 << TAG | SIM_TAG )
#define FLOAT1_TAGG     		( 7 << TAG | SIM_TAG )

//	Floats
#define IsFloat( x )			( ( FLOAT_MASK & ToULong( x ) ) == FLOAT0_TAGG )

//	Functions
#define sysFunctionKey			ToRef( 0 << TAGG | FN_TAGG )
#define IsFnKey( key )			( ( key ) == sysFunctionKey )
#define IsFunction( x ) 		( ( TAGG_MASK & ToULong( x ) ) == FN_TAGG )



////////////////////////////////////////////////////////////////////////
//	Key_ID.nnnnnnnn.fff.011.11 tags: Simple Keys
////////////////////////////////////////////////////////////////////////
//  It is unclear how many of these we can reasonably have and remain
//  efficient.  I think the convenience of having as many as we please
//  probably outweighs any small efficiency loss.
////////////////////////////////////////////////////////////////////////

#define KIND_WIDTH				3
#define KIND_MASK				0x7 << TAGG
#define IsSimpleKey( r )		( ( ToULong( r ) & TAGG_MASK ) == KEY_TAGG )
#define KindOfSimpleKey( k )	( ( ToULong( k ) & KIND_MASK ) >> TAGG )
#define PRIMITIVE_KIND			0
#define RECORD_KIND				1
#define VECTOR_KIND				2
#define STRING_KIND				3
#define OTHER_KIND				7
	
#define LEN_WIDTH					8
#define LENGTH_MASK					( 0xFF << KIND_WIDTH << TAGG )

#define SimpleKeyID( k ) 		( ToULong(k) >> LEN_WIDTH >> KIND_WIDTH >> TAGG )

#define MAKE_KEY( id, n, flav )		ToRef( ( ( id << LEN_WIDTH | n ) << KIND_WIDTH | flav ) << TAGG | KEY_TAGG )
#define sysAbsentKey    		MAKE_KEY( 0, 0, PRIMITIVE_KIND )
#define sysBoolKey      		MAKE_KEY( 1, 0, PRIMITIVE_KIND )
#define sysTerminKey    		MAKE_KEY( 2, 0, PRIMITIVE_KIND )
#define sysNilKey       		MAKE_KEY( 3, 0, PRIMITIVE_KIND )
#define sysPairKey      		MAKE_KEY( 4, 2, RECORD_KIND )
#define sysVectorKey    		MAKE_KEY( 5, 0, VECTOR_KIND )
#define sysStringKey    		MAKE_KEY( 6, 0, STRING_KIND )
#define sysWordKey      		MAKE_KEY( 7, 0, OTHER_KIND )
#define sysSmallKey				MAKE_KEY( 8, 0, PRIMITIVE_KIND )
#define sysFloatKey				MAKE_KEY( 9, 0, OTHER_KIND )
#define sysKeyKey				MAKE_KEY( 10, 0, OTHER_KIND )
#define sysUnicodeKey			MAKE_KEY( 11, 0, PRIMITIVE_KIND )
#define sysCharKey				MAKE_KEY( 12, 0, PRIMITIVE_KIND )
#define sysMapletKey			MAKE_KEY( 13, 2, RECORD_KIND )

//	Recognisers
#define IsPair( x )		( IsPtr4( x ) && ( *RefToPtr4( x ) == sysPairKey ) )
#define IsVector( x )	( IsPtr4( x ) && ( *RefToPtr4( x ) == sysVectorKey ) )


////////////////////////////////////////////////////////////////////////
// zzz.101.11 tags
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
	
#define IsFnLength( k )	( ( ToULong(k) & TAGGG_MASK ) == FUNC_LEN_TAGGG )


////////////////////////////////////////////////////////////////////////
// Elaborate constants that do not have to be super-efficient.
//	nnnnnnnn.000.101.11 tags
////////////////////////////////////////////////////////////////////////

#define sys_nil					ToRef( 0 << TAGGG | MISC_TAGGG )
#define sys_termin				ToRef( 1 << TAGGG | MISC_TAGGG )
#define sys_underflow			ToRef( 2 << TAGGG | MISC_TAGGG )
#define sys_undefined			ToRef( 3 << TAGGG | MISC_TAGGG )

//	Nil
#define IsNil( x )		( x == sys_nil )


////////////////////////////////////////////////////////////////////////



#endif
