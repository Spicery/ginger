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

#include "objlayout.hpp"

#include "common.hpp"
#include "key.hpp"
#include "mishap.hpp"

unsigned long sizeAfterKeyOfRecord( Ref * key ) {
	return ( ToULong( *key ) & LENGTH_MASK ) >> KIND_WIDTH >> TAGG;
}

unsigned long sizeAfterKeyOfVector( Ref * key ) {
	return ToULong( *( key - 1 ) ) >> TAG;
}

//	Add 1 for null.
unsigned long sizeAfterKeyOfString( Ref * key ) {
	return ( sizeAfterKeyOfVector( key ) + sizeof( long ) - 1 + 1 ) / sizeof( long );
}

unsigned long lengthOfString( Ref * key ) {
	return sizeAfterKeyOfVector( key );
}

unsigned long sizeAfterKeyOfFn( Ref * key ) {
	return ToULong( *( key - OFFSET_FROM_FN_LENGTH_TO_KEY ) ) >> TAGGG;
}

//
//	This computes obj_A and obj_Z1 from obj_K
void findObjectLimits( Ref * obj_K, Ref * & obj_A, Ref * & obj_Z1 ) {
	//	Keys fall into the following categories: FnKey, SimpleKey, Pointer to Keys
	Ref key = *obj_K;
	Ref * obj_K1 = obj_K + 1;
	if ( IsSimpleKey( key ) ) {
		switch ( KindOfSimpleKey( key ) ) {
			case PAIR_KIND:
			case RECORD_KIND: {
				obj_A = obj_K;
				obj_Z1 = obj_K1 + sizeAfterKeyOfRecord( obj_K );
				return;
			}
			case VECTOR_KIND: {
				obj_A = obj_K - 1;
				obj_Z1 = obj_K1 + sizeAfterKeyOfVector( obj_K );
				return;
			}
			case STRING_KIND: {
				obj_A = obj_K - 1;
				long d = sizeAfterKeyOfString( obj_K );
				obj_Z1 = obj_K1 + d;
				return;
			}
			default: throw "Unreachable";
		}
	} else if ( IsFnKey( key ) ) {		
		obj_A = obj_K - OFFSET_FROM_FN_LENGTH_TO_KEY;
		obj_Z1 = obj_K1 + sizeAfterKeyOfFn( obj_K );
		return;
	} else if ( IsObj( key ) ) {
		//	Compound keys not implemented yet.
		throw "unimplemented";
	} else {
		throw;
	}
}

//
//	This computes obj_K from obj_A
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
		for ( int n = 0; n < MAX_OFFSET_FROM_START_TO_KEY; n++, obj_A++ ) {
			if ( isKey( *obj_A ) ) {
				return obj_A;
			}
		}
		throw Unreachable();
	}
}

//
//	This computes obj_C = obj_Z - obj_A from obj_K
//
unsigned long lengthAfterObjectKey( Ref * obj_K ) {
	//	Keys fall into the following categories: FnKey, SimpleKey, Pointer to Keys
	Ref key = *obj_K;
	if ( IsSimpleKey( key ) ) {
		switch ( KindOfSimpleKey( key ) ) {
			case PAIR_KIND:
			case RECORD_KIND: {
				return sizeAfterKeyOfRecord( obj_K );
				break;
			}
			case VECTOR_KIND: {
				return sizeAfterKeyOfVector( obj_K );
				break;
			}
			case STRING_KIND: {
				return sizeAfterKeyOfString( obj_K );
				break;
			}
			default: throw "Unreachable";
		}
	} else if ( IsFnKey( key ) ) {
		return sizeAfterKeyOfFn( obj_K );
	} else if ( IsObj( key ) ) {
		//	Compound keys not implemented yet.
		throw "unimplemented";
	} else {
		throw;
	}
}
