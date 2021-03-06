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

#include <iostream>
#include <cassert>

#include "misclayout.hpp"
#include "vectorlayout.hpp"
#include "stringlayout.hpp"
#include "classlayout.hpp"
#include "wrecordlayout.hpp"

#include "common.hpp"
#include "key.hpp"
#include "mishap.hpp"

namespace Ginger {

////////////////////////////////////////////////////////////////////////////////
//	Full records
////////////////////////////////////////////////////////////////////////////////

unsigned long sizeAfterKeyOfRecordLayout( Ref * key ) {
	return ( ToULong( *key ) & LENGTH_MASK ) >> LENGTH_OFFSET;
}

unsigned long lengthOfRecordLayout( Ref * key ) {
	return sizeAfterKeyOfRecordLayout( key );
}

////////////////////////////////////////////////////////////////////////////////
//	Full vectors
////////////////////////////////////////////////////////////////////////////////
/*
unsigned long sizeAfterKeyOfVectorLayout( Ref * key ) {
	return SmallToULong( key[ VECTOR_OFFSET_LENGTH ] );
	//return ToULong( *( key - 1 ) ) >> TAG;
}

unsigned long lengthOfVectorLayout( Ref * key ) {
	return SmallToULong( key[ VECTOR_OFFSET_LENGTH ] );
}

Ref fastVectorLength( Ref r ) {
	return RefToPtr4( r )[ VECTOR_OFFSET_LENGTH ];
}
*/

////////////////////////////////////////////////////////////////////////////////
//	Mixed vectors
////////////////////////////////////////////////////////////////////////////////

unsigned long sizeAfterKeyOfMixedLayout( Ref * key ) {
	return 
		SmallToULong( key[ MIXED_LAYOUT_OFFSET_LENGTH ] ) + 
		LengthOfSimpleKey( *key );
}

unsigned long numFieldsOfMixedLayout( Ref * key ) {
	return LengthOfSimpleKey( *key );
}

unsigned long lengthOfMixedLayout( Ref * key ) {
	return SmallToULong( key[ MIXED_LAYOUT_OFFSET_LENGTH ] );
}


////////////////////////////////////////////////////////////////////////////////
//	Strings
////////////////////////////////////////////////////////////////////////////////

/*
//	Add 1 for null.
unsigned long sizeAfterKeyOfStringLayout( Ref * key ) {
	return ( sizeAfterKeyOfVectorLayout( key ) + sizeof( long ) - 1 + 1 ) / sizeof( long );
}

unsigned long lengthOfStringLayout( Ref * key ) {
	return sizeAfterKeyOfVectorLayout( key );
}

unsigned long fastStringLength( Ref str ) {
	return ToULong( RefToPtr4( str )[ STRING_LAYOUT_OFFSET_LENGTH ] ) >> TAG;
}
*/

////////////////////////////////////////////////////////////////////////////////
//	Objects
////////////////////////////////////////////////////////////////////////////////

//
//	This computes obj_A and obj_Z1 from obj_K. 
//
void findObjectLimits( Ref * obj_K, Ref * & obj_A, Ref * & obj_Z1 ) {
	//	Keys fall into the following categories: FunctionKey, SimpleKey, Pointer to Keys
	Ref key = *obj_K;
	Ref * obj_K1 = obj_K + 1;
	if ( IsSimpleKey( key ) ) {
		//switch ( KindOfSimpleKey( key ) ) {
		switch ( LayoutOfSimpleKey( key ) ) {
			case RECORD_LAYOUT: {
			//case PAIR_KIND:
			//case MAP_KIND:
			//case RECORD_KIND: {
				assert( LayoutOfSimpleKey( key ) == RECORD_LAYOUT );
				assert( KindOfSimpleKey( key ) == PAIR_KIND || KindOfSimpleKey( key ) == MAP_KIND || KindOfSimpleKey( key ) == RECORD_KIND );
				obj_A = obj_K;
				obj_Z1 = obj_K1 + sizeAfterKeyOfRecordLayout( obj_K );
				return;
			}
			case VECTOR_LAYOUT: {
			//case VECTOR_KIND: {
				assert( LayoutOfSimpleKey( key ) == VECTOR_LAYOUT );
				assert( KindOfSimpleKey( key ) == VECTOR_KIND );
				obj_A = obj_K - 1;
				obj_Z1 = obj_K1 + sizeAfterKeyOfVectorLayout( obj_K );
				return;
			}
			case MIXED_LAYOUT: {
				assert( LayoutOfSimpleKey( key ) == MIXED_LAYOUT );
				obj_A = obj_K - 1;
				obj_Z1 = obj_K1 + sizeAfterKeyOfMixedLayout( obj_K );
				return;
			}
			case STRING_LAYOUT: {
			//case STRING_KIND: {
				assert( LayoutOfSimpleKey( key ) == STRING_LAYOUT );
				assert( KindOfSimpleKey( key ) == STRING_KIND );
				obj_A = obj_K - 1;
				long d = sizeAfterKeyOfStringLayout( obj_K );
				obj_Z1 = obj_K1 + d;
				return;
			}
			case WRECORD_LAYOUT: {
				assert( LayoutOfSimpleKey( key ) == WRECORD_LAYOUT );
				obj_A = obj_K;
				obj_Z1 = obj_K1 + sizeAfterKeyOfWRecordLayout( obj_K );
				//std::cerr << "obj_A: " << obj_A << ", " << "obj_Z1: " << obj_Z1 << std::endl;
				return;
			}
			default: throw "Unreachable";
		}
	} else if ( IsFunctionKey( key ) ) {		
		obj_A = obj_K - OFFSET_FROM_FN_LENGTH_TO_KEY;
		obj_Z1 = obj_K1 + sizeAfterKeyOfFn( obj_K );
		return;
	} else if ( IsObj( key ) ) {
		obj_A = obj_K;
		obj_Z1 = obj_K1 + sizeAfterKeyOfInstance( obj_K );
	} else {
		throw;
	}
}

//
//	This computes obj_K from obj_A. 
//
Ref * findObjectKey( Ref * obj_A ) {
	//	Cases are that 'obj_A' is pointing at
	//		1.	FnLengthKey.
	//		2.	NonKey* Key series
	if ( IsFnLength( *obj_A ) ) {
		//	We are at the start of a function.
		//	It has a fixed offset to the key.
		return obj_A + OFFSET_FROM_FN_LENGTH_TO_KEY;
	} else {
		for ( int n = 0; n < MAX_OFFSET_FROM_START_TO_KEY; n++ ) {
			//
			//	Note that this function is called on objects that have been
			//	forwarded but not yet scanned by the GC. So the test for the
			//	key has to be capable of coping with a forwarded key. This
			//	test is stupider and restricts values in front of the key to
			//	simple values.
			//
			//	A smarter test is 
			//		1. 	Are you a simple key? Job done. A: yes
			//		2.	Are you an object? Define K = *you
			//			2a.	Is *K the keykey? If so job done. A: yes
			//			2b. Is *K forwarded? Define F = *K
			//			2c.	Is *F the keykey? If so A: yes
			//		3. 	Otherwise done A: no
			//	
			/*	I think the code would look like this.
			if ( IsSimpleKey( *obj_A ) ) return obj_A;
			if ( IsObj( *obj_A ) ) {
				if ( *RefToPtr4( *obj_A ) == sysClassKey ) return obj_A;
			}
			if ( IsFwd( *obj_A ) ) {
				Ref K = *FwdToPtr4( *obj_A );
				if ( IsObj( K ) ) {
					if ( *RefToPtr4( K ) == sysClassKey ) return obj_A;
				}
			} 
			*/
			Ref * x = obj_A + n;
			Ref k = *x;
			if ( IsSimpleKey( k ) || IsObj( k ) || IsFwd( k ) ) {
				return x;
			}
		}
		throw UnreachableError();
	}
}

//
//	This computes obj_C = obj_Z - obj_A from obj_K
//
unsigned long lengthAfterObjectKey( Ref * obj_K ) {
	//	Keys fall into the following categories: FunctionKey, SimpleKey, Pointer to Keys
	Ref key = *obj_K;
	if ( IsSimpleKey( key ) ) {
		switch ( LayoutOfSimpleKey( key ) ) {
		//switch ( KindOfSimpleKey( key ) ) {
			case RECORD_LAYOUT: {
			//case MAP_KIND:
			//case PAIR_KIND:
			//case RECORD_KIND: {
				assert( LayoutOfSimpleKey( key ) == RECORD_LAYOUT );
				assert( KindOfSimpleKey( key ) == PAIR_KIND || KindOfSimpleKey( key ) == MAP_KIND  || KindOfSimpleKey( key ) == RECORD_KIND );
				return sizeAfterKeyOfRecordLayout( obj_K );
				break;
			}
			case VECTOR_LAYOUT: {
			//case VECTOR_KIND: {
				assert( LayoutOfSimpleKey( key ) == VECTOR_LAYOUT );
				assert( KindOfSimpleKey( key ) == VECTOR_KIND );
				return sizeAfterKeyOfVectorLayout( obj_K );
				break;
			}
			case MIXED_LAYOUT: {
				assert( LayoutOfSimpleKey( key ) == MIXED_LAYOUT );
				return sizeAfterKeyOfMixedLayout( obj_K );
				break;
			}
			case STRING_LAYOUT: {
			//case STRING_KIND: {
				assert( LayoutOfSimpleKey( key ) == STRING_LAYOUT );
				assert( KindOfSimpleKey( key ) == STRING_KIND );
				return sizeAfterKeyOfStringLayout( obj_K );
				break;
			}
			case WRECORD_LAYOUT: {
				assert( LayoutOfSimpleKey( key ) == WRECORD_LAYOUT );
				return sizeAfterKeyOfWRecordLayout( obj_K );
				break;
			}
			default: 
				throw UnreachableError();
		}
	} else if ( IsFunctionKey( key ) ) {
		return sizeAfterKeyOfFn( obj_K );
	} else if ( IsObj( key ) ) {
		return sizeAfterKeyOfInstance( obj_K );
	} else {
		throw Ginger::Mishap( "Cannot take length of this" );
	}
}

} // namespace Ginger
