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

#include <map>
#include <string>
#include <iostream>

#include <stdio.h>

#include "machine.hpp"
#include "sys.hpp"
#include "key.hpp"
#include "cage.hpp"
#include "mishap.hpp"
#include "garbagecollect.hpp"
#include "objlayout.hpp"

#include "syslist.hpp"
#include "sysvector.hpp"
#include "sysstring.hpp"

//#define DBG_SYS

Ref refKey( Ref r ) {
	unsigned long u = ( unsigned long )r;
	unsigned long tag, tagg, taggg;
	tag = u & TAG_MASK;
	if ( tag == INT_TAG ) return sysSmallKey;
	if ( tag == OBJ_TAG ) return *RefToPtr4(r);
	tagg = u & TAGG_MASK;
	if ( tagg == ( 0 | SIM_TAG ) ) return sysAbsentKey;
	if ( tagg == ToULong( sys_false ) || tagg == ToULong( sys_true ) ) return sysBoolKey;
	if ( tagg == FN_TAGG ) return sysFunctionKey;
	if ( tagg == KEY_TAGG ) return sysKeyKey;
	taggg = u & TAGGG_MASK;
	if ( taggg == CHAR_TAGGG ) return sysCharKey;
	if ( taggg == MISC_TAGGG ) {
		if ( r == sys_nil ) {
			return sysNilKey; 
		} else {
			throw;
		}
	}
	throw;
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

void refPrint( Ref r ) {
	refPrint( std::cout, r );
}

void refPtrPrint( Ref * r ) {
	refPrint( Ptr4ToRef( r ) );
}

void refPtrPrint( std::ostream & out, Ref * r ) {
	refPrint( out, Ptr4ToRef( r ) );
}

void refPrint( std::ostream & out, Ref r ) {
	Ref k;
#ifdef DBG_SYS	
	out << "About to print '" << (unsigned int) r << "'\n";
#endif
	k = refKey( r );
#ifdef DBG_SYS
	out << "key = " << ToULong(k) << "\n";
#endif
	if ( k == sysStringKey ) {
		Ref *rr = RefToPtr4( r );
		char *s = ToChars( rr + 1 );
		out << s;
	} else if ( k == sysSmallKey ) {
		out << SmallToLong( r );
	} else if ( k == sysBoolKey ) {
		out << ( r == sys_false ? "false" : "true" );
	} else if ( k == sysAbsentKey ) {
		out << "absent";
	} else if ( k == sysFunctionKey ) {
		out << "<function>";
	} else if ( k == sysCharKey ) {
		out << CharacterToChar( r );
	} else if ( k == sysPairKey ) {
		Ref sofar = r;
		bool sep = false;
		out << "[";
		while ( refKey( sofar ) == sysPairKey ) {
			if ( sep ) { out << ","; } else { sep = true; }
			refPrint( out, *( RefToPtr4( sofar ) + 1 ) );
			sofar = *( RefToPtr4( sofar ) + 2 );
		}
		out << "]";
	} else if ( k == sysNilKey ) {
		out << "[]";
	} else if ( k == sysVectorKey ) {
		bool sep = false;
		out << "{";
		Ref * p = RefToPtr4( r );
		long len = RefToLong( p[ -1 ] );
		for ( int i = 1; i <= len; i++ ) {
			if ( sep ) { out << ","; } else { sep = true; }
			refPrint( p[ i ] ); 
		}
		out << "}";
	} else {
		out << "?(" << std::hex << ToULong( r ) << ")";
	}
}

Ref * sysRefPrint( Ref * pc, class MachineClass * vm ) {
	for ( int i = vm->count - 1; i >= 0; i-- ) {
		Ref r = vm->fastSubscr( i );
		refPrint( r );		
	}
	vm->fastDrop( vm->count );
	return pc;
}

Ref * sysRefPrintln( Ref * pc, class MachineClass * vm ) {
	pc = sysRefPrint( pc, vm );
	std::cout << std::endl;
	return pc;
}

Ref * sysHash( Ref *pc, class MachineClass * vm ) {
	if ( vm->count == 1 ) {
		vm->fastPeek() = ULongToSmall( refHash( vm->fastPeek() ) );
		return pc;
	} else {
		throw Mishap( "Wrong number of arguments for hash" );
	}
}





//	This should be decomposed by implemented vectorAppend, listAppend
//	and stringAppend
Ref * sysAppend( Ref * pc, class MachineClass * vm ) {
	if ( vm->count == 2 ) {
		Ref rhs = vm->fastPeek();
		Ref lhs = vm->fastPeek( 1 );
		if ( IsObj( lhs ) && IsObj( rhs ) ) {
			Ref * lhs_K = ObjToPtr4( lhs );
			Ref * rhs_K = ObjToPtr4( rhs );
			Ref lhs_key = *lhs_K;
			Ref rhs_key = *rhs_K;
			if ( lhs_key == rhs_key ) {	
				if ( IsSimpleKey( lhs_key ) ) {
					switch ( KindOfSimpleKey( lhs_key ) ) {
						case VECTOR_KIND: {
							return sysVectorAppend( pc, vm );
						}
						case PAIR_KIND: {
							return sysListAppend( pc, vm );
						}
						case STRING_KIND: {
							return sysStringAppend( pc, vm );
						}				
						default: {}
					}
				}
			}
		} else if ( IsNil( lhs ) ) {
			return sysListAppend( pc, vm );
		}
	}
	throw Mishap( "Invalid arguments for append (++)" );
}

Ref * sysExplode( Ref *pc, class MachineClass * vm ) {
	if ( vm->count == 1 ) {
		Ref r = vm->fastPeek();
		if ( IsObj( r ) ) {
			vm->fastPop();
			Ref * obj_K = ObjToPtr4( r );
			Ref key = *obj_K;
			if ( IsFnKey( key ) ) {
				throw Mishap( "Trying to explode (...) a function object" );
			} else if ( IsSimpleKey( key ) ) {
				switch ( KindOfSimpleKey( key ) ) {
					case VECTOR_KIND: {
						pc = sysVectorExplode( pc, vm );
						break;
					}
					case PAIR_KIND: {
						pc = sysListExplode( pc, vm );
						break;
					}
					case RECORD_KIND: {
						unsigned long n = sizeAfterKeyOfRecord( obj_K );
						vm->checkStackRoom( n );
						memcpy( vm->vp + 1, obj_K + 1, n * sizeof( Ref ) );
						vm->vp += n;
						break;
					}
					case STRING_KIND: {
						pc = sysStringExplode( pc, vm );
						break;
					}				
					default: {
						throw ToBeDone();
					}
				}
			} else if ( IsObj( key ) ) {
				//	Compound keys not implemented yet.
				throw ToBeDone();	//	(compound keys)
			} else {
				throw ToBeDone();
			}
		} else if ( IsNil( r ) ) {
			return sysListExplode( pc, vm );
		} else {
			throw ToBeDone();
		}
		return pc;
	} else {
		throw Mishap( "Wrong number of arguments for explode (...)" );
	}
}

Ref * sysVectorLength( Ref *pc, class MachineClass * vm ) {
	if ( vm->count =! 1 ) throw Mishap( "Wrong number of arguments for vectorLength" );
	Ref r = vm->fastPeek();
	if ( !IsVector( r ) ) throw Mishap( "Argument mismatch for vectorLength" );
	Ref * obj_K = ObjToPtr4( r );
	
	vm->fastPeek() = LongToSmall( sizeAfterKeyOfVector( obj_K ) );
	return pc;
}

Ref * sysFastVectorLength( Ref *pc, class MachineClass * vm ) {
	Ref r = vm->fastPeek();
	Ref * obj_K = ObjToPtr4( r );
	vm->fastPeek() = LongToSmall( sizeAfterKeyOfVector( obj_K ) );
	return pc;
}

Ref * sysLength( Ref *pc, class MachineClass * vm ) {
	if ( vm->count == 1 ) {
		Ref r = vm->fastPeek();
		if ( IsObj( r ) ) {
			Ref * obj_K = ObjToPtr4( r );
			Ref key = *obj_K;
			if ( IsFnKey( key ) ) {
				throw Mishap( "Trying to explode (...) a function object" );
			} else if ( IsSimpleKey( key ) ) {
				switch ( KindOfSimpleKey( key ) ) {
					case PRIMITIVE_KIND: {
						if ( key == sysNilKey ) {
							vm->fastPeek() = LongToSmall( 0 );
						} else {
							throw;
						}
						break;
					}
					case VECTOR_KIND: {
						vm->fastPeek() = LongToSmall( sizeAfterKeyOfVector( obj_K ) );
						pc = sysFastVectorLength( pc, vm );
						break;
					}
					case PAIR_KIND: {
						pc = sysListLength( pc, vm );
						break;
					}
					case RECORD_KIND: {
						vm->fastPeek() = LongToSmall( sizeAfterKeyOfRecord( obj_K ) );
						break;
					}
					case STRING_KIND: {
						vm->fastPeek() = LongToSmall( sizeAfterKeyOfVector( obj_K ) );
						break;
					}				
					default: {
						throw "unimplemented (other)";
					}
				}
			} else if ( IsObj( key ) ) {
				//	Compound keys not implemented yet.
				throw "unimplemented (compound keys)";
			} else {
				throw "unimplemented";
			}
		} else {
			throw "unimplemented";
		}
		return pc;
	} else {
		throw Mishap( "Wrong number of arguments for explode (...)" );
	}
}

#include "datatypes.cpp.auto"

typedef std::map< std::string, SysInfo > SysMap;
const SysMap::value_type rawData[] = {
	SysMap::value_type( "+", SysInfo( fnc_add, Arity( 2 ), 0 ) ),
	SysMap::value_type( "-", SysInfo( fnc_sub, Arity( 2 ), 0 ) ),
	SysMap::value_type( "*", SysInfo( fnc_mul, Arity( 2 ), 0 ) ),
	SysMap::value_type( "/", SysInfo( fnc_div, Arity( 2 ), 0 ) ),
	SysMap::value_type( "**", SysInfo( fnc_pow, Arity( 2 ), 0 ) ),
	SysMap::value_type( "<", SysInfo( fnc_lt, Arity( 2 ), 0 ) ),
	SysMap::value_type( "<=", SysInfo( fnc_lte, Arity( 2 ), 0 ) ),
	SysMap::value_type( "==", SysInfo( fnc_eq, Arity( 2 ), 0 ) ),
	SysMap::value_type( ">", SysInfo( fnc_gt, Arity( 2 ), 0 ) ),
	SysMap::value_type( ">=", SysInfo( fnc_gte, Arity( 2 ), 0 ) ),	
	SysMap::value_type( "gc", SysInfo( fnc_syscall, Arity( 0 ), sysGarbageCollect ) ),
	SysMap::value_type( "hash", SysInfo( fnc_syscall, Arity( 1 ), sysHash ) ),
	SysMap::value_type( "refPrint", SysInfo( fnc_syscall, Arity( 1 ), sysRefPrint ) ),
	SysMap::value_type( "refPrintln", SysInfo( fnc_syscall, Arity( 1 ), sysRefPrintln ) ),
	SysMap::value_type( "explode", SysInfo( fnc_syscall, Arity( 0, true ), sysExplode ) ),
	SysMap::value_type( "length", SysInfo( fnc_syscall, Arity( 1 ), sysLength ) ),	
	SysMap::value_type( "append", SysInfo( fnc_syscall, Arity( 1 ), sysAppend ) ),	
	SysMap::value_type( "isNil", SysInfo( fnc_syscall, Arity( 1 ), sysIsNil ) ),
	SysMap::value_type( "isList", SysInfo( fnc_syscall, Arity( 1 ), sysIsList ) ),
	SysMap::value_type( "newList", SysInfo( fnc_syscall, Arity( 0, true ), sysNewList ) ),
	SysMap::value_type( "newListOnto", SysInfo( fnc_syscall, Arity( 1, true ), sysNewListOnto ) ),
	#include "sysmap.inc.auto"
};
const int numElems = sizeof rawData / sizeof rawData[0];
SysMap sysMap( rawData, rawData + numElems );