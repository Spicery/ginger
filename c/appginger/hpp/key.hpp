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
#define sysFunctionKey				ToRef( 0 << TAGG | FN_TAGG )
#define sysCoreFunctionKey			ToRef( 1 << TAGG | FN_TAGG )
#define sysMethodKey				ToRef( 2 << TAGG | FN_TAGG )
#define IsFunctionKey( key )		( ( TAGG_MASK & ToULong( key ) ) == FN_TAGG )
#define IsHeapFunctionKey( key )	( ( key ) == sysFunctionKey )
#define IsCoreFunctionKey( key ) 	( ( key ) == sysCoreFunctionKey )
#define IsMethodKey( key ) 			( ( key ) == sysMethodKey )
#define IsFunction( ref )			( IsObj( ref ) && IsFunctionKey( *RefToPtr4( ref ) ) )
#define IsMethod( ref )				( IsObj( ref ) && IsMethodKey( *RefToPtr4( ref ) ) )


////////////////////////////////////////////////////////////////////////
//	Key_ID.nnnnnnnn.fff.011.11 tags: Simple Keys
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


#define KIND_WIDTH				3
#define KIND_MASK				0x7 << TAGG
#define IsSimpleKey( r )		( ( ToULong( r ) & TAGG_MASK ) == KEY_TAGG )
#define KindOfSimpleKey( k )	( ( ToULong( k ) & KIND_MASK ) >> TAGG )
#define KEYLESS_KIND			0
#define RECORD_KIND				1
#define VECTOR_KIND				2
#define STRING_KIND				3
#define PAIR_KIND				4
#define MAP_KIND				5
#define UNUSED_KIND				6
#define OTHER_KIND				7
	
#define LEN_WIDTH				8
#define LENGTH_OFFSET 			( KIND_WIDTH + TAGG )
#define LENGTH_MASK				( 0xFF << LENGTH_OFFSET )

#define SimpleKeyID( k ) 		( ToULong(k) >> LEN_WIDTH >> KIND_WIDTH >> TAGG )

#define MAKE_KEY( id, n, flav )		ToRef( ( ( id << LEN_WIDTH | n ) << KIND_WIDTH | flav ) << TAGG | KEY_TAGG )
#define sysAbsentKey    		MAKE_KEY( 0, 0, KEYLESS_KIND )
#define sysBoolKey      		MAKE_KEY( 1, 0, KEYLESS_KIND )
#define sysKeyKey				MAKE_KEY( 2, 4, RECORD_KIND )
#define sysTerminKey    		MAKE_KEY( 3, 0, KEYLESS_KIND )
#define sysNilKey       		MAKE_KEY( 4, 0, KEYLESS_KIND )
#define sysPairKey      		MAKE_KEY( 5, 2, PAIR_KIND )
#define sysVectorKey    		MAKE_KEY( 6, 0, VECTOR_KIND )
#define sysStringKey    		MAKE_KEY( 7, 0, STRING_KIND )
#define sysSymbolKey      		MAKE_KEY( 8, 1, KEYLESS_KIND )
#define sysSmallKey				MAKE_KEY( 9, 0, KEYLESS_KIND )
#define sysFloatKey				MAKE_KEY( 10, 0, KEYLESS_KIND )
#define sysUnicodeKey			MAKE_KEY( 11, 0, KEYLESS_KIND )
#define sysCharKey				MAKE_KEY( 12, 0, KEYLESS_KIND )
#define sysMapletKey			MAKE_KEY( 13, 2, RECORD_KIND )
#define sysIndirectionKey		MAKE_KEY( 14, 1, RECORD_KIND )
#define sysAssocKey				MAKE_KEY( 15, 3, RECORD_KIND )
//	Do NOT move references with changing IsRefSimpleKey
#define sysHardRefKey			MAKE_KEY( 16, 1, RECORD_KIND )
#define sysSoftRefKey			MAKE_KEY( 17, 1, RECORD_KIND )
#define sysWeakRefKey			MAKE_KEY( 18, 1, RECORD_KIND )
#define sysReservedRefKey		MAKE_KEY( 19, 1, RECORD_KIND )
//	Do NOT move references with changing IsMapSimpleKey
#define CacheEqMapID			20
#define sysCacheEqMapKey		MAKE_KEY( CacheEqMapID, 2, MAP_KIND )
#define HardEqMapID				21
#define sysHardEqMapKey			MAKE_KEY( HardEqMapID, 2, MAP_KIND )
#define HardIdMapID				22
#define sysHardIdMapKey			MAKE_KEY( HardIdMapID, 2, MAP_KIND )
#define WeakIdMapID				23
#define sysWeakIdMapKey			MAKE_KEY( WeakIdMapID, 2, MAP_KIND )


#define IsRefSimpleKey( k )		( ( SimpleKeyID( k ) >> 2 ) == 4 )
#define IsRefKey( k )			( IsSimpleKey( k ) && IsRefSimpleKey( k ) )

#define IsMapSimpleKey( k )		( ( SimpleKeyID( k ) >> 2 ) == 5 )
#define IsMapKey( k )			( IsSimpleKey( k ) && IsMapSimpleKey( k ) )
#define MapKeyEq( k )			( ( SimpleKeyID( k ) & 0x10 ) == 1 )


//	Recognisers
#define IsPair( x )				( IsObj( x ) && ( *RefToPtr4( x ) == sysPairKey ) )
#define IsVector( x )			( IsObj( x ) && ( *RefToPtr4( x ) == sysVectorKey ) )
#define IsMaplet( x ) 			( IsObj( x ) && ( *RefToPtr4( x ) == sysMapletKey ) )
#define IsAssoc( x ) 			( IsObj( x ) && ( *RefToPtr4( x ) == sysAssocKey ) )
#define IsString( x )			( IsObj( x ) && ( *RefToPtr4( x ) == sysStringKey ) )
#define IsRef( x )				( IsObj( x ) && IsRefKey( *RefToPtr4( x ) ) )
#define IsMap( x ) 				( IsObj( x ) && IsMapKey( *RefToPtr4( x ) ) )

#define IsVectorKind( x )		( IsObj( x ) && KindOfSimpleKey( *RefToPtr4( x ) ) == VECTOR_KIND )
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

#define sys_nil					ToRef( 0 << TAGGG | MISC_TAGGG )
#define sys_termin				ToRef( 1 << TAGGG | MISC_TAGGG )

//	sys_system_only is a system version of sys_absent. User 
//	programmers never see this.
#define sys_system_only			ToRef( 2 << TAGGG | MISC_TAGGG )

#define sys_undef				ToRef( 3 << TAGGG | MISC_TAGGG )

//	Nil
#define IsNil( x )				( x == sys_nil )


////////////////////////////////////////////////////////////////////////



#endif

