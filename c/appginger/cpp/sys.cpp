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
#include "sysmap.hpp"
#include "syskey.hpp"
#include "sysequals.hpp"
#include "sysprint.hpp"

//#define DBG_SYS


/*
 * Does not check vmcount. Which is good because we do not guarantee
 * set the vmcount when it is called. This is purely about performance.
 */
Ref * sysFastGetFastIterator( Ref * pc, class MachineClass * vm ) {
	Ref r = vm->fastPop();
	vm->checkStackRoom( 3 );
	if ( IsObj( r ) ) {
		Ref * obj_K = ObjToPtr4( r );
		Ref key = *obj_K;
		if ( IsSimpleKey( key ) ) {
			switch ( KindOfSimpleKey( key ) ) {
				case PAIR_KIND: {
					vm->fastPush( r );			//	Iteration state.
					vm->fastPush( sys_absent );	//	Iteration context, a dummy.
					vm->fastPush( vm->sysFastListIterator() );
					break;
				}
				case RECORD_KIND: {
					throw ToBeDone();
					break;
				}
				case STRING_KIND: {
					vm->fastPush( LongToRef(0) );	//	Iteration state.
					vm->fastPush( r );				//	Iteration context, a dummy.
					vm->fastPush( vm->sysFastStringIterator() );
					break;
				}
				case MAP_KIND: {
				
					//	Explode the map.
					Ref * mark = vm->vp;
					vm->fastPush( r );
					vm->count = 1;
					pc = sysMapExplode( pc, vm );
					ptrdiff_t n = vm->vp - mark;

					//	Gather the map as a vector.
					vm->count = n;
					pc = sysNewVector( pc, vm );
					Ref x = vm->fastPop();
					
					//	Now treat it as a vector iteration.

					vm->fastPush( LongToRef(1) );	//	Iteration state.
					vm->fastPush( x );				//	Iteration context, a dummy.
					vm->fastPush( vm->sysFastVectorIterator() );
					break;
				}
				case VECTOR_KIND: {
					vm->fastPush( LongToRef(1) );	//	Iteration state.
					vm->fastPush( r );				//	Iteration context, a dummy.
					vm->fastPush( vm->sysFastVectorIterator() );
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
		vm->fastPush( r );			//	Iteration state.
		vm->fastPush( sys_absent );	//	Iteration context, a dummy.
		vm->fastPush( vm->sysFastListIterator() );
	} else {
		throw ToBeDone();
	}
	return pc;
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
			Ref * obj_K = ObjToPtr4( r );
			Ref key = *obj_K;
			if ( IsFunctionKey( key ) ) {
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
						vm->fastDrop( 1 );
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
					case MAP_KIND: {
						pc = sysMapExplode( pc, vm );
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

Ref * sysLength( Ref *pc, class MachineClass * vm ) {
	if ( vm->count == 1 ) {
		Ref r = vm->fastPeek();
		if ( IsObj( r ) ) {
			Ref * obj_K = ObjToPtr4( r );
			Ref key = *obj_K;
			if ( IsFunctionKey( key ) ) {
				throw Mishap( "Trying to explode (...) a function object" );
			} else if ( IsSimpleKey( key ) ) {
				switch ( KindOfSimpleKey( key ) ) {
					case VECTOR_KIND: {
						vm->fastPeek() = LongToSmall( sizeAfterKeyOfVector( obj_K ) );	// Same as pc = sysFastVectorLength( pc, vm );
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
						vm->fastPeek() = LongToSmall( sizeAfterKeyOfVector( obj_K ) );	// Same as pc = sysStringLength( pc, vm );
						break;
					}
					case MAP_KIND: {
						throw Mishap( "Trying to take length of a map" );
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
		} else if ( IsNil( r ) ) {
			vm->fastPeek() = LongToSmall( 0 ); // same as pc = sysListLength( pc, vm );
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
	SysMap::value_type( "+", SysInfo( fnc_add, Arity( 2 ), Arity( 1 ), 0 ) ),
	SysMap::value_type( "-", SysInfo( fnc_sub, Arity( 2 ), Arity( 1 ), 0 ) ),
	SysMap::value_type( "*", SysInfo( fnc_mul, Arity( 2 ), Arity( 1 ), 0 ) ),
	SysMap::value_type( "/", SysInfo( fnc_div, Arity( 2 ), Arity( 1 ), 0 ) ),
	SysMap::value_type( "**", SysInfo( fnc_pow, Arity( 2 ), Arity( 1 ), 0 ) ),
	SysMap::value_type( "<", SysInfo( fnc_lt, Arity( 2 ), Arity( 1 ), 0 ) ),
	SysMap::value_type( "<=", SysInfo( fnc_lte, Arity( 2 ), Arity( 1 ), 0 ) ),
	SysMap::value_type( "==", SysInfo( fnc_eq, Arity( 2 ), Arity( 1 ), 0 ) ),
	SysMap::value_type( ">", SysInfo( fnc_gt, Arity( 2 ), Arity( 1 ), 0 ) ),
	SysMap::value_type( ">=", SysInfo( fnc_gte, Arity( 2 ), Arity( 1 ), 0 ) ),	
	SysMap::value_type( "objectKey", SysInfo( fnc_syscall, Arity( 1 ), Arity( 1 ), sysObjectKey ) ),
	SysMap::value_type( "=", SysInfo( fnc_syscall, Arity( 2 ), Arity( 1 ), sysEquals ) ),
	SysMap::value_type( "gc", SysInfo( fnc_syscall, Arity( 0 ), Arity( 0 ), sysGarbageCollect ) ),
	SysMap::value_type( "hash", SysInfo( fnc_syscall, Arity( 1 ), Arity( 1 ), sysHash ) ),
	SysMap::value_type( "refPrint", SysInfo( fnc_syscall, Arity( 1 ), Arity( 0 ), sysRefPrint ) ),
	SysMap::value_type( "refPrintln", SysInfo( fnc_syscall, Arity( 1 ), Arity( 0 ), sysRefPrintln ) ),
	SysMap::value_type( "explode", SysInfo( fnc_syscall, Arity( 1 ), Arity( 0, true ), sysExplode ) ),
	SysMap::value_type( "listExplode", SysInfo( fnc_syscall, Arity( 1 ), Arity( 0, true ), sysListExplode ) ),
	SysMap::value_type( "vectorExplode", SysInfo( fnc_syscall, Arity( 1 ), Arity( 0, true ), sysVectorExplode ) ),
	SysMap::value_type( "stringExplode", SysInfo( fnc_syscall, Arity( 1 ), Arity( 0, true ), sysStringExplode ) ),
	SysMap::value_type( "length", SysInfo( fnc_syscall, Arity( 1 ), Arity( 1 ), sysLength ) ),	
	SysMap::value_type( "listLength", SysInfo( fnc_syscall, Arity( 1 ), Arity( 1 ), sysListLength ) ),	
	SysMap::value_type( "vectorLength", SysInfo( fnc_syscall, Arity( 1 ), Arity( 1 ), sysVectorLength ) ),	
	SysMap::value_type( "stringLength", SysInfo( fnc_syscall, Arity( 1 ), Arity( 1 ), sysStringLength ) ),	
	SysMap::value_type( "append", SysInfo( fnc_syscall, Arity( 2 ), Arity( 1 ), sysAppend ) ),	
	SysMap::value_type( "listAppend", SysInfo( fnc_syscall, Arity( 2 ), Arity( 1 ), sysListAppend ) ),	
	SysMap::value_type( "vectorAppend", SysInfo( fnc_syscall, Arity( 2 ), Arity( 1 ), sysVectorAppend ) ),	
	SysMap::value_type( "stringAppend", SysInfo( fnc_syscall, Arity( 2 ), Arity( 1 ), sysStringAppend ) ),	
	SysMap::value_type( "isNil", SysInfo( fnc_syscall, Arity( 1 ), Arity( 1 ), sysIsNil ) ),
	SysMap::value_type( "isList", SysInfo( fnc_syscall, Arity( 1 ), Arity( 1 ), sysIsList ) ),
	SysMap::value_type( "newList", SysInfo( fnc_syscall, Arity( 0, true ), Arity( 1 ), sysNewList ) ),
	SysMap::value_type( "newListOnto", SysInfo( fnc_syscall, Arity( 1, true ), Arity( 1 ), sysNewListOnto ) ),
	SysMap::value_type( "newMap", SysInfo( fnc_syscall, Arity( 0, true ), Arity( 1 ), sysNewMap ) ),
	#include "sysmap.inc.auto"
};
const int numElems = sizeof rawData / sizeof rawData[0];
SysMap sysMap( rawData, rawData + numElems );