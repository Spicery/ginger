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
#include "objlayout.hpp"
#include "mishap.hpp"

static inline Ref FastPairHead( Ref r ) {
	return ObjToPtr4( r )[ 1 ];
}

static inline Ref FastPairTail( Ref r ) {
	return ObjToPtr4( r )[ 2 ];
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
		Ref * obj_K = ObjToPtr4( r );
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

static bool refVectorEquals( Ref * vx, Ref * vy ) {
	unsigned long lx = sizeAfterKeyOfVector( vx );
	unsigned long ly = sizeAfterKeyOfVector( vy );
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
	unsigned long n = sizeAfterKeyOfRecord( rx );
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
	unsigned long lx = lengthOfString( rx );
	unsigned long ly = lengthOfString( ry );
	if ( lx == ly ) {
		char * cx = reinterpret_cast< char * >( &rx[1] );
		char * cy = reinterpret_cast< char * >( &ry[1] );
		return memcmp( cx, cy, lx ) == 0;
	} else {
		return false;
	}
}

bool refEquals( Ref x, Ref y ) {
	if ( x == y ) {
		return true;
	} else if ( IsObj( x ) ) {
		if ( !IsObj( y ) ) {
			return false;
		} else {
			Ref * x_K = ObjToPtr4( x );
			Ref * y_K = ObjToPtr4( y );
			Ref xkey = *x_K;
			Ref ykey = *y_K;
			
			if ( IsFnKey( xkey ) ) {
				//	Functions must be identical to be equal.
				return false;
			} else if ( IsSimpleKey( xkey ) ) {
				if ( xkey == ykey ) {
					switch ( KindOfSimpleKey( xkey ) ) {
						case VECTOR_KIND: 	return refVectorEquals( x_K, y_K );
						case PAIR_KIND: 	return refPairEquals( x, y );
						case RECORD_KIND: 	return refRecordEquals( x_K, y_K );
						case STRING_KIND: 	return refStringEquals( x_K, y_K );
						default: {
							throw ToBeDone();
						}
					}
				} else {
					return false;	//	At least for now.
				}
			} else if ( IsObj( xkey ) ) {
				throw ToBeDone();
			} else {
				throw ToBeDone();
			}			
		}
	} else {
		//	Simple values must be identical to be equal.
		return false;
	}
}

unsigned long refHash( Ref r ) {
	if ( IsObj( r ) ) {
		Ref * obj_K = ObjToPtr4( r );
		Ref key = *obj_K;
		if ( IsFnKey( key ) ) {
			HashEngine e( ToULong( key ) );
			e.add( ToULong(obj_K[ -OFFSET_TO_NSLOTS_TO_KEY ] ) );
			e.add( ToULong( obj_K[ -OFFSET_FROM_FN_LENGTH_TO_KEY ] ) );
			return e.hash();
		} else if ( IsSimpleKey( key ) ) {
			switch ( KindOfSimpleKey( key ) ) {
				case VECTOR_KIND: {
					HashEngine e( ToULong( key ) );
					unsigned long n = sizeAfterKeyOfVector( obj_K );
					e.add( n );
					if ( n > 0 ) {
						e.add( trivHash( obj_K[1] ) );
						e.add( trivHash( obj_K[n] ) );
					}
					return e.hash();
				}
				case PAIR_KIND:
				case RECORD_KIND: {
					HashEngine e( ToULong( key ) );
					unsigned long n = sizeAfterKeyOfRecord( obj_K );
					e.add( n );
					if ( n > 0 ) {
						e.add( trivHash( obj_K[1] ) );
						e.add( trivHash( obj_K[n] ) );
					}
					return e.hash();
				}
				case STRING_KIND: {
					HashEngine e( ToULong( key ) );
					unsigned long n = lengthOfString( obj_K );
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
		vm->fastPeek() = ULongToSmall( refHash( vm->fastPeek() ) );
		return pc;
	} else {
		throw Mishap( "Wrong number of arguments for hash" );
	}
}

Ref * sysEquals( Ref * pc, class MachineClass * vm ) {
	if ( vm->count == 2 ) {
		Ref y = vm->fastPop();
		Ref x = vm->fastPeek();
		vm->fastPeek() = refEquals( x, y ) ? sys_true : sys_false;
		return pc;
	} else {
		throw Mishap( "Wrong number of arguments for =" );
	}
}
