/******************************************************************************\
	Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
	of the GNU General Public License. This file is part of AppGinger.

    AppGinger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AppGinger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AppGinger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

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
#define OBJ_TAG         1
#define FWD_TAG         2
#define SIM_TAG         3

#define RefToPtr4( r )  	ToRefRef( ToULong( r ) & ~OBJ_TAG )
#define Ptr4ToRef( p ) 		ToRef( ToULong( p ) | OBJ_TAG )

#define LongToRef( i )		( (Ref)( ToULong( i ) << TAG | INT_TAG ) )
#define LongToSmall( i ) 	( (Ref)( ToULong( i ) << TAG | INT_TAG ) )
#define RefToLong( r )   	( (long)( ToLong( r ) >> TAG ) )
#define SmallToLong( r )	( (long)( ToLong( r ) >> TAG ) )
#define SmallToULong( r )	( (unsigned long)( ToLong( r ) >> TAG ) )

#define ULongToSmall( x ) 	ToRef( ( x ) << TAG | INT_TAG )

#define FwdToPtr4( r )		( ToULong( r ) & ~0x3 )
#define Ptr4ToFwd( p )		ToRef( ToULong( p ) | FWD_TAG )

#define IsObj( r )			( ( TAG_MASK & ToULong( r ) ) == OBJ_TAG )
#define IsSmall( r )		( ( TAG_MASK & ToULong( r ) ) == INT_TAG )
#define IsFwd( r )			( ( TAG_MASK & ToULong( r ) ) == FWD_TAG )
#define IsFwdObj( r ) 		( IsObj( r ) && IsFwd( *RefToPtr4( r ) ) )
#define IsSimple( r )		( ( TAG_MASK & ToULong( r ) ) == SIM_TAG )
#define IsPrimitive( r )	( IsSimple( r ) || IsSmall( r ) )

#define IntToChar( i )		( (Ref)( ( i ) << TAGGG | CHAR_TAGGG ) )

#define AddSmall( a, b )	ToRef( ToLong( a ) + ToLong( b ) )

////////////////////////////////////////////////////////////////////////
// ---.yyy.11 tags
////////////////////////////////////////////////////////////////////////
//  Experimentation with current architectures shows that you can get
//  expect good performance if you are working with 5-bit constants.
//  The 2 questionable members of this set are RGB constants and
//  deferrals.
////////////////////////////////////////////////////////////////////////

#define TAGG            		5
#define TAGG_MASK				0x1F
#define FLOAT_MASK				0xF
#define SYS_ABSENT     		 	ToRef( 0 << TAG | SIM_TAG )
#define SYS_FALSE      			ToRef( 1 << TAG | SIM_TAG )
#define SYS_TRUE				ToRef( 2 << TAG | SIM_TAG )
#define FN_TAGG       			( 3 << TAG | SIM_TAG )
#define KEY_TAGG        		( 4 << TAG | SIM_TAG )
#define ESC_TAGG      			( 5 << TAG | SIM_TAG )
#define FLOAT0_TAGG    			( 6 << TAG | SIM_TAG )
#define FLOAT1_TAGG     		( 7 << TAG | SIM_TAG )

//	Floats
#define IsFloat( x )			( ( FLOAT_MASK & ToULong( x ) ) == FLOAT0_TAGG )

//	Functions
#define sysFunctionKey				ToRef( 0 << TAGG | FN_TAGG )
#define sysCoreFunctionKey			ToRef( 1 << TAGG | FN_TAGG )
#define sysMethodKey				ToRef( 2 << TAGG | FN_TAGG )
#define IsFunctionKey( key )		( ( TAGG_MASK & ToULong( key ) ) == FN_TAGG )
#define IsHeapFunctionKey( key )	( ( key ) == sysFunctionKey )
#define IsCoreFunctionKey( key ) 	( ( key ) == sysCoreFunctionKey )
#define IsMethodKey( key ) 			( ( key ) == sysMethodKey )
#define IsFunction( ref )			( IsObj( ref ) && IsFunctionKey( *RefToPtr4( ref ) ) )
#define IsMethod( ref )				( IsObj( ref ) && IsMethodKey( *RefToPtr4( ref ) ) )
#define IsBoolean( ref )			( ( ( ref ) == SYS_FALSE ) || ( ( ref ) == SYS_TRUE ) )


////////////////////////////////////////////////////////////////////////
//	Key_ID.nnnnnnnn.gggg.fff.011.11 tags: Simple Keys
////////////////////////////////////////////////////////////////////////
//  It is unclear how many of these we can reasonably have and remain
//  efficient.  I think the convenience of having as many as we please
//  probably outweighs any small efficiency loss.
////////////////////////////////////////////////////////////////////////

/*
KEYLESS_KIND 
	means that it is a simple key for a type of value that does not include 
	a reference to the key, typically because it does not appear in the heap.
*/

#define LAYOUT_WIDTH			3
#define LAYOUT_OFFSET			TAGG
#define LAYOUT_MASK				( 0x7 << LAYOUT_OFFSET )

#define KIND_WIDTH				4
#define KIND_OFFSET				( LAYOUT_OFFSET + LAYOUT_WIDTH )
#define KIND_MASK				( 0xF << ( TAGG + LAYOUT_WIDTH ) )
#define IsSimpleKey( r )		( ( ToULong( r ) & TAGG_MASK ) == KEY_TAGG )
#define KindOfSimpleKey( k )	( ( ToULong( k ) & KIND_MASK ) >> KIND_OFFSET )
#define LayoutOfSimpleKey( k )	( ( ToULong( k ) & LAYOUT_MASK ) >> LAYOUT_OFFSET )
	
#define LEN_WIDTH				8
#define LENGTH_OFFSET 			( KIND_WIDTH + KIND_OFFSET )
#define LENGTH_MASK				( 0xFF << LENGTH_OFFSET )
#define LengthOfSimpleKey( k ) 	( ( ToULong( k ) & LENGTH_MASK ) >> LENGTH_OFFSET )

#define SIM_KEY_ID_OFFSET		( LENGTH_OFFSET + LEN_WIDTH )
#define SimpleKeyID( k ) 		( ToULong(k) >> SIM_KEY_ID_OFFSET )

#define MAKE_KEY( ID, N, K, L ) \
	ToRef( ( ( ( (ID) << LEN_WIDTH | (N) ) << KIND_WIDTH | (K) ) << LAYOUT_WIDTH | (L) ) << TAGG | KEY_TAGG )

//	Include the definitions for the Kinds, the ${SimpleKey}ID and sys${SimpleKey}Key's
#include "simplekey.hpp.auto"


#define IsRefSimpleKey( k )		( ( SimpleKeyID( k ) >> 2 ) == 4 )
#define IsRefKey( k )			( IsSimpleKey( k ) && IsRefSimpleKey( k ) )

#define IsMapKey( k )			( IsSimpleKey( k ) && IsMapSimpleKey( k ) )
#define IsMapSimpleKey( k )		( ( SimpleKeyID( k ) >> 2 ) == 5 )
#define MapKeyEq( k )			( ( SimpleKeyID( k ) & 0x2 ) == 0 )

#define IsAttrMapKey( k )		( IsSimpleKey( k ) && KindOfSimpleKey( k ) == ATTR_KIND )

//	Recognisers
#define IsPair( x )				( IsObj( x ) && ( *RefToPtr4( x ) == sysPairKey ) )
#define IsVector( x )			( IsObj( x ) && ( *RefToPtr4( x ) == sysVectorKey ) )
#define IsMaplet( x ) 			( IsObj( x ) && ( *RefToPtr4( x ) == sysMapletKey ) )
#define IsAssoc( x ) 			( IsObj( x ) && ( *RefToPtr4( x ) == sysAssocKey ) )
#define IsString( x )			( IsObj( x ) && ( *RefToPtr4( x ) == sysStringKey ) )
#define IsRef( x )				( IsObj( x ) && IsRefKey( *RefToPtr4( x ) ) )
#define IsMap( x ) 				( IsObj( x ) && IsMapKey( *RefToPtr4( x ) ) )
#define IsAttrMap( x ) 			( IsObj( x ) && IsAttrMapKey( *RefToPtr4( x ) ) )
#define IsElement( x )			( IsObj( x ) && ( *RefToPtr4( x ) == sysElementKey ) )

#define IsVectorKind( x )		( IsObj( x ) && KindOfSimpleKey( *RefToPtr4( x ) ) == VECTOR_KIND )
#define IsMixedKind( x )		( IsObj( x ) && KindOfSimpleKey( *RefToPtr4( x ) ) == MIXED_KIND )
#define IsRecordKind( x )		( IsObj( x ) && KindOfSimpleKey( *RefToPtr4( x ) ) == RECORD_KIND )
#define IsStringKind( x )		( IsObj( x ) && KindOfSimpleKey( *RefToPtr4( x ) ) == STRING_KIND )

#define IsClassKind( ref )		IsInstance( ref )
#define IsInstance( ref )		( IsObj( ref ) && IsObj( *RefToPtr4( ref ) ) )
#define IsClass( ref )			( IsObj( ref ) && ( *RefToPtr4( ref ) == sysKeyKey ) )

#define INDEX( rv, offset )		( RefToPtr4( rv )[ offset ] )

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
#define SYMBOL_TAGGG   	( 4 << TAGG | ESC_TAGG )
#define UNUSED5_TAGGG   ( 5 << TAGG | ESC_TAGG )
#define UNUSED6_TAGGG   ( 6 << TAGG | ESC_TAGG )
#define UNUSED7_TAGGG   ( 7 << TAGG | ESC_TAGG )

#define CharToCharacter( c ) \
	ToRef( (c) << TAGGG | CHAR_TAGGG )

#define CharacterToChar( r ) \
	( (char)( ToULong(r) >> TAGGG ) )
	
#define IsFnLength( k )	( ( ToULong(k) & TAGGG_MASK ) == FUNC_LEN_TAGGG )
#define IsSymbol( k )	( ( ToULong(k) & TAGGG_MASK ) == SYMBOL_TAGGG )


////////////////////////////////////////////////////////////////////////
// Elaborate constants that do not have to be super-efficient.
//	nnnnnnnn.000.101.11 tags
////////////////////////////////////////////////////////////////////////

#define SYS_NIL					ToRef( 0 << TAGGG | MISC_TAGGG )
#define SYS_TERMIN				ToRef( 1 << TAGGG | MISC_TAGGG )

//	sys_system_only is a system version of sys_absent. User 
//	programmers never see this.
#define SYS_SYSTEM_ONLY			ToRef( 2 << TAGGG | MISC_TAGGG )

#define SYS_UNDEF				ToRef( 3 << TAGGG | MISC_TAGGG )

//	Nil
#define IsNil( x )				( x == SYS_NIL )


////////////////////////////////////////////////////////////////////////
//	End of file
////////////////////////////////////////////////////////////////////////
#endif

