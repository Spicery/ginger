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

#include "sysequals.hpp"
#include "key.hpp"
#include "misclayout.hpp"
#include "vectorlayout.hpp"
#include "stringlayout.hpp"
#include "mishap.hpp"

#include <string.h>

static inline Ref FastPairHead( Ref r ) {
	return RefToPtr4( r )[ 1 ];
}

static inline Ref FastPairTail( Ref r ) {
	return RefToPtr4( r )[ 2 ];
}


/*
 *	Dummy implementation.
 */
class HashEngine {
public:
	void add( unsigned long ) {
	}
	
public:
	unsigned long hash() {
		return 0;
	}
	
public:
	HashEngine( unsigned long x ) {
	}
};

static unsigned long trivHash( Ref r ) {
	if ( IsObj( r ) ) {
		Ref * obj_K = RefToPtr4( r );
		Ref key = *obj_K;
		if ( IsObj( key ) ) {
			return 0;			//	To be improved.
		} else {
			return ToULong( key );
		}
	} else {
		return ToULong( r );
	}
}

static bool refVectorAndMixedEquals( Ref * vx, Ref * vy ) {
	unsigned long lx = sizeAfterKeyOfVectorLayout( vx );
	unsigned long ly = sizeAfterKeyOfVectorLayout( vy );
	if ( lx == ly ) {
		for ( unsigned long i = 1; i <= lx; i++ ) {
			if ( ! refEquals( vx[ i ], vx[ i ] ) ) return false;
		}
		return true;
	} else {
		return false;
	}
}

static bool refRecordEquals( Ref * rx, Ref * ry ) {
	unsigned long n = sizeAfterKeyOfRecordLayout( rx );
	for ( unsigned long i = 1; i <= n; i++ ) {
		if ( !refEquals( rx[ i ], ry[ i ] ) ) return false;
	}
	return true;
}

static bool refPairEquals( Ref rx, Ref ry ) {
	do {
		if ( ! refEquals( FastPairHead( rx ), FastPairHead( ry ) ) ) return false;
		rx = FastPairTail( rx );
		ry = FastPairTail( ry );
	} while ( IsPair( rx ) && IsPair( ry ) );
	return rx == ry;
}

static bool refStringEquals( Ref * rx, Ref * ry ) {
	unsigned long lx = lengthOfStringLayout( rx );
	unsigned long ly = lengthOfStringLayout( ry );
	if ( lx == ly ) {
		char * cx = reinterpret_cast< char * >( &rx[1] );
		char * cy = reinterpret_cast< char * >( &ry[1] );
		return memcmp( cx, cy, lx ) == 0;
	} else {
		return false;
	}
}

static bool refWRecordEquals( Ref * rx, Ref * ry ) {
	unsigned long n = sizeAfterKeyOfRecordLayout( rx );
	for ( unsigned long i = 1; i <= n; i++ ) {
		if ( rx[ i ] == ry[ i ] ) return false;
	}
	return true;
}

static bool refMapEquals( Ref * rx, Ref * ry ) {
	throw Ginger::Mishap( "ToBeDone" );
}

bool refEquals( Ref x, Ref y ) {
	if ( x == y ) {
		return true;
	} else if ( IsObj( x ) ) {
		if ( !IsObj( y ) ) {
			return false;
		} else {
			Ref * x_K = RefToPtr4( x );
			Ref * y_K = RefToPtr4( y );
			Ref xkey = *x_K;
			Ref ykey = *y_K;
			
			if ( IsFunctionKey( xkey ) ) {
				//	Functions must be identical to be equal.
				return false;
			} else if ( IsSimpleKey( xkey ) ) {
				if ( xkey == ykey ) {
					switch ( KindOfSimpleKey( xkey ) ) {
						case MIXED_KIND:	//	Shares implementation with Vector.
						case VECTOR_KIND: 	return refVectorAndMixedEquals( x_K, y_K );
						case PAIR_KIND: 	return refPairEquals( x, y );
						case MAP_KIND:		return refMapEquals( x_K, y_K );
						case RECORD_KIND: 	return refRecordEquals( x_K, y_K );
						case EXTERNAL_KIND:	//	fallthru.
						case WRECORD_KIND: 	return refWRecordEquals( x_K, y_K );
						case STRING_KIND: 	return refStringEquals( x_K, y_K );
						default: {
							throw Ginger::Mishap( "ToBeDone" );
						}
					}
				} else {
					return false;	//	At least for now.
				}
			} else if ( IsObj( xkey ) ) {
				throw Ginger::Mishap( "ToBeDone" );
			} else {
				throw Ginger::Mishap( "ToBeDone" );
			}			
		}
	} else {
		//	Simple values must be identical to be equal.
		return false;
	}
}

unsigned long gngEqHash( Ref r ) {
	if ( IsObj( r ) ) {
		Ref * obj_K = RefToPtr4( r );
		Ref key = *obj_K;
		if ( IsFunctionKey( key ) ) {
			HashEngine e( ToULong( key ) );
			e.add( ToULong(obj_K[ -OFFSET_TO_NSLOTS_TO_KEY ] ) );
			e.add( ToULong( obj_K[ -OFFSET_FROM_FN_LENGTH_TO_KEY ] ) );
			return e.hash();
		} else if ( IsSimpleKey( key ) ) {
			switch ( KindOfSimpleKey( key ) ) {
				case VECTOR_KIND: {
					HashEngine e( ToULong( key ) );
					unsigned long n = sizeAfterKeyOfVectorLayout( obj_K );
					e.add( n );
					if ( n > 0 ) {
						e.add( trivHash( obj_K[1] ) );
						e.add( trivHash( obj_K[n] ) );
					}
					return e.hash();
				}
				case MIXED_KIND: {
					HashEngine e( ToULong( key ) );
					unsigned long n = sizeAfterKeyOfMixedLayout( obj_K );
					e.add( n );
					if ( n > 0 ) {
						e.add( trivHash( obj_K[1] ) );
						e.add( trivHash( obj_K[n] ) );
					}
					return e.hash();
				}
				case MAP_KIND: {
					HashEngine e( ToULong( key ) );
					e.add( ToULong( obj_K[2] ) );
					return e.hash();
				}
				case PAIR_KIND:
				case RECORD_KIND: {
					HashEngine e( ToULong( key ) );
					unsigned long n = sizeAfterKeyOfRecordLayout( obj_K );
					e.add( n );
					if ( n > 0 ) {
						e.add( trivHash( obj_K[1] ) );
						e.add( trivHash( obj_K[n] ) );
					}
					return e.hash();
				}
				case EXTERNAL_KIND:
				case WRECORD_KIND: {
					HashEngine e( ToULong( key ) );
					unsigned long n = sizeAfterKeyOfRecordLayout( obj_K );
					e.add( n );
					if ( n > 0 ) {
						e.add( ToULong( obj_K[1] ) );
						e.add( ToULong( obj_K[n] ) );
					}
					return e.hash();
				}
				case STRING_KIND: {
					HashEngine e( ToULong( key ) );
					unsigned long n = lengthOfStringLayout( obj_K );
					char * obj_K1 = (char *)( obj_K + 1 );
					if ( n > 0 ) {
						e.add( obj_K1[ 0 ] );
						e.add( obj_K1[ n - 1 ] );						
					}
					return e.hash();
				}
				default: {
					return 0;
				}
			}
		} else if ( IsObj( key ) ) {
			//	Compound keys not implemented yet.
			return 0;
		} else {
			return 0;
		}
	} else {
		return ToULong( r );
	}
}

Ref * sysHash( Ref *pc, class MachineClass * vm ) {
	if ( vm->count == 1 ) {
		vm->fastPeek() = ULongToSmall( gngEqHash( vm->fastPeek() ) );
		return pc;
	} else {
		throw Ginger::Mishap( "Wrong number of arguments for hash" );
	}
}

Ref * sysEquals( Ref * pc, class MachineClass * vm ) {
	if ( vm->count == 2 ) {
		Ref y = vm->fastPop();
		Ref x = vm->fastPeek();
		vm->fastPeek() = refEquals( x, y ) ? SYS_TRUE : SYS_FALSE;
		return pc;
	} else {
		throw Ginger::Mishap( "Wrong number of arguments for =" );
	}
}

