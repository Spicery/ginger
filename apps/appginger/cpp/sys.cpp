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
#include <string.h>


#include "machine.hpp"
#include "sys.hpp"
#include "key.hpp"
#include "cage.hpp"
#include "mishap.hpp"
#include "garbagecollect.hpp"

#include "misclayout.hpp"
#include "vectorlayout.hpp"

#include "syscgi.hpp"
#include "syslist.hpp"
#include "sysvector.hpp"
#include "sysmixed.hpp"
#include "sysstring.hpp"
#include "sysmap.hpp"
#include "syskey.hpp"
#include "sysclass.hpp"
#include "sysequals.hpp"
#include "sysprint.hpp"
#include "sysfunction.hpp"
#include "sysmethod.hpp"
#include "sysclass.hpp"
#include "sysunix.hpp"
#include "sysattrmap.hpp"
#include "syselement.hpp"
#include "sysexception.hpp"

using namespace Ginger;

//#define DBG_SYS


/*
 * Does not check vmcount. Which is good because we do not guarantee
 * set the vmcount when it is called. This is purely about performance.
 */
Ref * sysFastGetFastIterator( Ref * pc, class MachineClass * vm ) {
	Ref r = vm->fastPop();
	vm->checkStackRoom( 3 );
	if ( IsObj( r ) ) {
		Ref * obj_K = RefToPtr4( r );
		Ref key = *obj_K;
		if ( IsSimpleKey( key ) ) {
			switch ( KindOfSimpleKey( key ) ) {
				case PAIR_KIND: {
					vm->fastPush( r );			//	Iteration state.
					vm->fastPush( SYS_ABSENT );	//	Iteration context, a dummy.
					vm->fastPush( vm->sysFastListIterator() );
					break;
				}
				case RECORD_KIND: {
					throw Mishap( "ToBeDone" );
					break;
				}
				case STRING_KIND: {
					vm->fastPush( LongToRef(0) );	//	Iteration state.
					vm->fastPush( r );				//	Iteration context, a dummy.
					vm->fastPush( vm->sysFastStringIterator() );
					break;
				}
				case ATTR_KIND:
				case MAP_KIND: {
				
					//	Explode the map.
					Ref * mark = vm->vp;
					vm->fastPush( r );
					vm->count = 1;
					pc = sysExplode( pc, vm );
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
				case MIXED_KIND: {
					vm->fastPush( LongToRef(1) );	//	Iteration state.
					vm->fastPush( r );				//	Iteration context, a dummy.
					vm->fastPush( vm->sysFastMixedIterator() );
					break;
				}
				default: {
					throw Ginger::Mishap( "ToBeDone" );
				}
			}
		} else if ( IsObj( key ) ) {
			//	Compound keys not implemented yet.
			throw Ginger::Mishap( "ToBeDone" );	//	(compound keys)
		} else {
			throw Ginger::Mishap( "ToBeDone" );
		}
	} else if ( IsNil( r ) ) {
		vm->fastPush( r );			//	Iteration state.
		vm->fastPush( SYS_ABSENT );	//	Iteration context, a dummy.
		vm->fastPush( vm->sysFastListIterator() );
	} else {
		throw Ginger::Mishap( "ToBeDone" );
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
			Ref * lhs_K = RefToPtr4( lhs );
			Ref * rhs_K = RefToPtr4( rhs );
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

/*
	index( map_like, idx )
*/
Ref * sysIndex( Ref *pc, class MachineClass * vm ) {
	if ( vm->count != 2 ) throw Ginger::Mishap( "ArgsMismatch" );
	Ref idx = vm->fastPop();
	Ref map = vm->fastPeek();
	vm->fastPush( idx );
	
	if ( IsObj( map ) ) {
		Ref * map_K = RefToPtr4( map );
		if ( IsSimpleKey( *map_K ) ) {
			switch ( KindOfSimpleKey( *map_K ) ) {
				case VECTOR_KIND: return pc = sysVectorIndex( pc, vm );
				case MIXED_KIND: return pc = sysMixedKindIndex( pc, vm, *map_K );
				case PAIR_KIND: return pc = sysListIndex( pc, vm );
				case STRING_KIND: return pc = sysStringIndex( pc, vm );
				case MAP_KIND: return pc = sysMapIndex( pc, vm );
				case ATTR_KIND: return pc = sysAttrMapIndex( pc, vm );
				default: throw Ginger::Mishap( "ToBeDone" );
			}
		} else {
			throw Ginger::Mishap( "ToBeDone" );
		}
	} else if ( IsNil( map ) ) {
		return sysListIndex( pc, vm );
	} else {
		throw Ginger::Mishap( "ToBeDone" );
	}
	return pc;
}

Ref * sysExplode( Ref *pc, class MachineClass * vm ) {
	if ( vm->count == 1 ) {
		Ref r = vm->fastPeek();
		if ( IsObj( r ) ) {
			Ref * obj_K = RefToPtr4( r );
			Ref key = *obj_K;
			if ( IsFunctionKey( key ) ) {
				throw Mishap( "Trying to explode (...) a function object" );
			} else if ( IsSimpleKey( key ) ) {
				switch ( KindOfSimpleKey( key ) ) {
					case VECTOR_KIND: {
						pc = sysVectorExplode( pc, vm );
						break;
					}
					case MIXED_KIND: {
						pc = sysMixedKindExplode( pc, vm, key );
						break;
					}
					case PAIR_KIND: {
						pc = sysListExplode( pc, vm );
						break;
					}
					case RECORD_KIND: {
						//	ARGUABLY THIS IS INCORRECT.
						vm->fastDrop( 1 );
						unsigned long n = sizeAfterKeyOfRecordLayout( obj_K );
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
					case ATTR_KIND: {
						pc = sysAttrMapExplode( pc, vm );
						break;
					}
					default: {
						throw Ginger::Mishap( "ToBeDone" );
					}
				}
			} else if ( IsObj( key ) ) {
				//	Compound keys not implemented yet.
				throw Ginger::Mishap( "ToBeDone" );	//	(compound keys)
			} else {
				throw Ginger::Mishap( "ToBeDone" );
			}
		} else if ( IsNil( r ) ) {
			return sysListExplode( pc, vm );
		} else {
			throw Ginger::Mishap( "ToBeDone" );
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
			Ref * obj_K = RefToPtr4( r );
			Ref key = *obj_K;
			if ( IsFunctionKey( key ) ) {
				throw Mishap( "Trying to explode (...) a function object" );
			} else if ( IsSimpleKey( key ) ) {
				switch ( KindOfSimpleKey( key ) ) {
					case VECTOR_KIND: {
						vm->fastPeek() = obj_K[ VECTOR_LAYOUT_OFFSET_LENGTH ];
						break;
					}
					case MIXED_KIND: {
						vm->fastPeek() = obj_K[ MIXED_LAYOUT_OFFSET_LENGTH ];
						break;
					}
					case PAIR_KIND: {
						pc = sysListLength( pc, vm );
						break;
					}
					case RECORD_KIND: {
						vm->fastPeek() = LongToSmall( sizeAfterKeyOfRecordLayout( obj_K ) );
						break;
					}
					case STRING_KIND: {
						vm->fastPeek() = LongToSmall( sizeAfterKeyOfVectorLayout( obj_K ) );	// Same as pc = sysStringLength( pc, vm );
						break;
					}
					case ATTR_KIND:
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
	SysMap::value_type( "+", SysInfo( fnc_add, Arity( 2 ), Arity( 1 ), 0, "Adds two numbers" ) ),
	SysMap::value_type( "-", SysInfo( fnc_sub, Arity( 2 ), Arity( 1 ), 0, "Substracts two numbers" ) ),
	SysMap::value_type( "*", SysInfo( fnc_mul, Arity( 2 ), Arity( 1 ), 0, "Multiplies two numbers" ) ),
	SysMap::value_type( "/", SysInfo( fnc_div, Arity( 2 ), Arity( 1 ), 0, "Divides two numbers, result is a float" ) ),
	SysMap::value_type( "**", SysInfo( fnc_pow, Arity( 2 ), Arity( 1 ), 0, "Exponentiates two numbers, result is float" ) ),
	SysMap::value_type( "<", SysInfo( fnc_lt, Arity( 2 ), Arity( 1 ), 0, "Less than, compares two real numbers" ) ),
	SysMap::value_type( "<=", SysInfo( fnc_lte, Arity( 2 ), Arity( 1 ), 0, "Less than or equal to, compares two real number" ) ),
	SysMap::value_type( "==", SysInfo( fnc_eq, Arity( 2 ), Arity( 1 ), 0, "Identity two two values, result is boolean" ) ),
	SysMap::value_type( ">", SysInfo( fnc_gt, Arity( 2 ), Arity( 1 ), 0, "Greater than, compares two real numbers" ) ),
	SysMap::value_type( ">=", SysInfo( fnc_gte, Arity( 2 ), Arity( 1 ), 0, "Greater than or equal to, compares two real numbers" ) ),	
	SysMap::value_type( "instanceClass", SysInfo( fnc_syscall, Arity( 1 ), Arity( 1 ), sysObjectKey, "Returns the key of any value" ) ),
	SysMap::value_type( "newRecordClass", SysInfo( fnc_syscall, Arity( 2 ), Arity( 1 ), sysNewRecordClass, "Returns a new class object for records" ) ),
	SysMap::value_type( "newClass", SysInfo( fnc_syscall, Arity( 4 ), Arity( 1 ), sysNewClass, "Returns a new class object for instances" ) ),
	SysMap::value_type( "newClassRecogniser", SysInfo( fnc_syscall, Arity( 1 ), Arity( 1 ), sysClassRecogniser, "Returns a function that recognises a given class" ) ),
	SysMap::value_type( "newClassConstructor", SysInfo( fnc_syscall, Arity( 1 ), Arity( 1 ), sysClassConstructor, "Returns the constructor for a given class" ) ),
	SysMap::value_type( "newClassAccessor", SysInfo( fnc_syscall, Arity( 1 ), Arity( 1 ), sysClassAccessor, "Returns the accessor for a given class" ) ),
	SysMap::value_type( "newClassExploder", SysInfo( fnc_syscall, Arity( 1 ), Arity( 0, true ), sysClassExploder, "Returns the exploder for a given class" ) ),
	SysMap::value_type( "newMethod", SysInfo( fnc_syscall, Arity( 3 ), Arity( 1 ), sysNewMethod, "Constructs a new empty method" ) ),
	SysMap::value_type( "setMethod", SysInfo( fnc_syscall, Arity( 3 ), Arity( 0 ), sysSetMethod, "Sets the function for a method on an object" ) ),
	SysMap::value_type( "setSlot", SysInfo( fnc_syscall, Arity( 3 ), Arity( 0 ), sysSetSlot, "Sets a method as the Nth slot of a class" ) ),
	SysMap::value_type( "=", SysInfo( fnc_syscall, Arity( 2 ), Arity( 1 ), sysEquals, "Compare any two values as equal" ) ),
	SysMap::value_type( "gc", SysInfo( fnc_syscall, Arity( 0 ), Arity( 0 ), sysGarbageCollect, "Forces a garbage collection - useful for tests" ) ),
	SysMap::value_type( "hash", SysInfo( fnc_syscall, Arity( 1 ), Arity( 1 ), sysHash, "Computes a hash code for any value, returns a positive Small" ) ),
	SysMap::value_type( "index", SysInfo( fnc_syscall, Arity( 2 ), Arity( 1 ), sysIndex, "Indexes any sequence" ) ),
	SysMap::value_type( "mapIndex", SysInfo( fnc_syscall, Arity( 2 ), Arity( 1 ), sysMapIndex, "Indexes a map" ) ),
	SysMap::value_type( "listIndex", SysInfo( fnc_syscall, Arity( 2 ), Arity( 1 ), sysListIndex, "Indexes a list" ) ),
	SysMap::value_type( "explode", SysInfo( fnc_syscall, Arity( 1 ), Arity( 0, true ), sysExplode, "Explodes any sequence into its members" ) ),
	SysMap::value_type( "listExplode", SysInfo( fnc_syscall, Arity( 1 ), Arity( 0, true ), sysListExplode, "Explodes a list into its members" ) ),
	SysMap::value_type( "length", SysInfo( fnc_syscall, Arity( 1 ), Arity( 1 ), sysLength, "Returns the length of any sequence" ) ),	
	SysMap::value_type( "listLength", SysInfo( fnc_syscall, Arity( 1 ), Arity( 1 ), sysListLength, "Returns the length of a list" ) ),	
	SysMap::value_type( "append", SysInfo( fnc_syscall, Arity( 2 ), Arity( 1 ), sysAppend, "Appends two sequences of the same type, result is a sequence of the same type" ) ),	
	SysMap::value_type( "listAppend", SysInfo( fnc_syscall, Arity( 2 ), Arity( 1 ), sysListAppend, "Appends two lists" ) ),	
	SysMap::value_type( "isNil", SysInfo( fnc_syscall, Arity( 1 ), Arity( 1 ), sysIsNil, "Tests whether a list is empty or not" ) ),
	SysMap::value_type( "isList", SysInfo( fnc_syscall, Arity( 1 ), Arity( 1 ), sysIsList, "Tests whether an object is a list or not" ) ),
	SysMap::value_type( "newList", SysInfo( fnc_syscall, Arity( 0, true ), Arity( 1 ), sysNewList, "Builds a list from the arguments" ) ),
	SysMap::value_type( "newListOnto", SysInfo( fnc_syscall, Arity( 1, true ), Arity( 1 ), sysNewListOnto, "Builds a list from the first N-1 args and appends that to the last argument" ) ),
	SysMap::value_type( "newMap", SysInfo( fnc_syscall, Arity( 0, true ), Arity( 1 ), sysNewHardEqMap, "Builds a new map from maps and maplets" ) ),
	SysMap::value_type( "newHardIdMap", SysInfo( fnc_syscall, Arity( 0, true ), Arity( 1 ), sysNewHardIdMap, "Builds a new map from maps and maplets that uses identity (==) for the equality test" ) ),
	SysMap::value_type( "newWeakIdMap", SysInfo( fnc_syscall, Arity( 0, true ), Arity( 1 ), sysNewWeakIdMap, "Builds a new map from maps and maplets that only weakly retains its entries" ) ),
	SysMap::value_type( "newCacheMap", SysInfo( fnc_syscall, Arity( 0, true ), Arity( 1 ), sysNewCacheEqMap, "Builds a new map from maps and maplets that the garbage collector can clear under pressure" ) ),
	SysMap::value_type( "mapIndex", SysInfo( fnc_syscall, Arity( 2 ), Arity( 1 ), sysMapIndex, "Looks up a map for a given index" ) ),
	SysMap::value_type( "partApply", SysInfo( fnc_syscall, Arity( 1, true ), Arity( 1 ), sysPartApply, "Freezes arguments and a function together to make a new function" ) ),
	SysMap::value_type( "functionInArity", SysInfo( fnc_syscall, Arity( 1  ), Arity( 1 ), sysFunctionInArity, "Input arity of a function" ) ),
	SysMap::value_type( "functionOutArity", SysInfo( fnc_syscall, Arity( 1 ), Arity( 1 ), sysFunctionOutArity, "Output arity of a function" ) ),
#include "sysattrmap.inc"
#include "syscgi.inc"
#include "syselement.inc"
#include "sysexception.inc"
#include "sysprint.inc"
#include "sysstring.inc"
#include "sysunix.inc"
#include "sysvector.inc"
#include "sysmap.inc.auto"
};
const int numElems = sizeof rawData / sizeof rawData[0];
SysMap sysMap( rawData, rawData + numElems );
