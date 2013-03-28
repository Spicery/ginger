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
#include "syssymbol.hpp"
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
#include "sysapply.hpp"
#include "sysstack.hpp"

using namespace Ginger;

//#define DBG_SYS

SysInfo::SysInfo( SysNames _names, Ginger::Arity in, Ginger::Arity out, SysCall * s, const char * ds ) :
	names( _names ),
	flavour( SYS_CALL_FLAVOUR ),
	instruction( vmc_halt ),
	cmp_op( CMP_EQ ),
	in_arity( in ),
	out_arity( out ),
	syscall( s ),
	docstring( ds ),
	coreFunctionObject( NULL )
{		
	//	Self-registration.
	SysMap::sysMap()[ std::string( this->name() ) ] = *this;
}


/**
 * 	Does not check vmcount. Which is good because we do not guarantee to
 * 	set the vmcount when it is called. This is purely about performance.
 *	
 *	"on-style" iteration should also be defined separately. 
 */
Ref * sysFastGetFastIterator( Ref * pc, class MachineClass * vm ) {
	Ref r = vm->fastPop();
	vm->checkStackRoom( 3 );
	if ( IsObj( r ) ) {
		Ref * obj_K = RefToPtr4( r );
		Ref key = *obj_K;
		if ( IsSimpleKey( key ) ) {
			if ( SublayoutOfSimpleKey( key ) == ATOMIC_SUBLAYOUT ) {
				throw Mishap( "Trying to iterate over atomic value" ).culprit( "Value", refToString( obj_K ) );
			} else {
				switch ( KindOfSimpleKey( key ) ) {
					case PAIR_KIND: {
						vm->fastPush( r );			//	Iteration state.
						vm->fastPush( SYS_ABSENT );	//	Iteration context, a dummy.
						vm->fastPush( vm->sysFastListIterator() );
						break;
					}
					case WRECORD_KIND:
					case RECORD_KIND: {
						throw Mishap( "Trying to iterate over record value" ).culprit( "Value", refToString( obj_K ) );
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

/**
 * 	Append is defined for list-like objects.
 *	This should be decomposed by implemented vectorAppend, listAppend
 *	and stringAppend.
 */
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

Ref * sysUpdaterOfIndex( Ref *pc, class MachineClass * vm ) {
	if ( vm->count != 3 ) throw Ginger::Mishap( "ArgsMismatch: sysUpdaterOfIndex" );
	
	//	Quickly copy map value for decoding.
	//Ref val = vm->fastPop();
	//Ref idx = vm->fastPop();
	Ref map = vm->fastPeek();
	//vm->fastPush( idx );
	//vm->fastPush( val );

	if ( IsObj( map ) ) {
		Ref * map_K = RefToPtr4( map );
		if ( IsSimpleKey( *map_K ) ) {
			switch ( KindOfSimpleKey( *map_K ) ) {
				case VECTOR_KIND: return pc = sysSetIndexVector( pc, vm );
				//case MIXED_KIND: return pc = sysMixedKindIndex( pc, vm, *map_K );
				//case PAIR_KIND: return pc = sysListIndex( pc, vm );
				//case STRING_KIND: return pc = sysStringIndex( pc, vm );
				//case MAP_KIND: return pc = sysMapIndex( pc, vm );
				//case ATTR_KIND: return pc = sysAttrMapIndex( pc, vm );
				default: throw Ginger::Mishap( "ToBeDone: sysUpdaterOfIndex" ).culprit( "Item", refToString( map ) );
			}
		} else {
			throw Ginger::Mishap( "ToBeDone: sysUpdaterOfIndex (1)" );
		}
	} else if ( IsNil( map ) ) {
		//return sysListIndex( pc, vm );
		throw Ginger::Mishap( "ToBeDone: sysUpdaterOfIndex (2)" );
	} else {
		throw Ginger::Mishap( "ToBeDone: sysUpdaterOfIndex (3)" ).culprit( "Item", refToString( map ) );
	}

}


/**
 *	Index is only defined for list-like objects
 *		index( map_like, idx )
*/
Ref * sysIndex( Ref *pc, class MachineClass * vm ) {
	if ( vm->count != 2 ) throw Ginger::Mishap( "ArgsMismatch: sysIndex" );
	Ref map = vm->fastPeek();
	
	if ( IsObj( map ) ) {
		Ref * map_K = RefToPtr4( map );
		if ( IsSimpleKey( *map_K ) ) {
			switch ( KindOfSimpleKey( *map_K ) ) {
				case VECTOR_KIND: return pc = sysGetIndexVector( pc, vm );
				case MIXED_KIND: return pc = sysMixedKindIndex( pc, vm, *map_K );
				case PAIR_KIND: return pc = sysListIndex( pc, vm );
				case STRING_KIND: return pc = sysStringIndex( pc, vm );
				case MAP_KIND: return pc = sysMapIndex( pc, vm );
				case ATTR_KIND: return pc = sysAttrMapIndex( pc, vm );
				default: throw Ginger::Mishap( "ToBeDone: sysIndex" );
			}
		} else {
			throw Ginger::Mishap( "ToBeDone: sysIndex" );
		}
	} else if ( IsNil( map ) ) {
		return sysListIndex( pc, vm );
	} else {
		throw Ginger::Mishap( "ToBeDone: sysIndex" );
	}
	return pc;
}

Ref * sysUpdaterOfExplode( Ref * pc, MachineClass * vm ) {
	//	TODO:
	throw Ginger::Mishap( "ToBeDone: sysUpdaterOfExplode" );
}

/**
 * 	Explode is the inverse of construction. It is therefore defined for a
 * 	wide variety of objects, not just list-like objects. 
 */
Ref * sysExplode( Ref *pc, class MachineClass * vm ) {
	if ( vm->count == 1 ) {
		Ref r = vm->fastPeek();
		if ( IsObj( r ) ) {
			Ref * obj_K = RefToPtr4( r );
			Ref key = *obj_K;
			if ( IsFunctionKey( key ) ) {
				throw Mishap( "Trying to explode (...) a function object" );
			} else if ( IsSimpleKey( key ) ) {
				if ( SublayoutOfSimpleKey( key ) == ATOMIC_SUBLAYOUT ) {
					throw Mishap( "Trying to explode an atomic value" ).culprit( "Value", refToString( obj_K ) );
				} else {
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
						case EXTERNAL_KIND:
						case WRECORD_KIND: {
							//	The problem here is that we need to box the words
							//	before pushing them onto the stack. But what do the
							//	words represent? The obvious view is that they represent
							//	unsigned/signed longs. 
							throw Mishap( "To be done" );
						}
						case RECORD_KIND: {
							//	ARGUABLY THIS IS INCORRECT. 
							//	What does it mean to explode a record? It means that
							//	it has a default map-like behaviour. If so, what are
							//	the keys? The access procedures??
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

/**
 * 	sysLength is definitely defined for list-like objects. The question is
 *	whether or not it is usefully defined for maps. I think it is.
 */
Ref * sysLength( Ref *pc, class MachineClass * vm ) {
	if ( vm->count == 1 ) {
		Ref r = vm->fastPeek();
		if ( IsObj( r ) ) {
			Ref * obj_K = RefToPtr4( r );
			Ref key = *obj_K;
			if ( IsFunctionKey( key ) ) {
				throw Mishap( "Trying to take the length of a function object" );
			} else if ( IsSimpleKey( key ) ) {
				if ( SublayoutOfSimpleKey( key ) == ATOMIC_SUBLAYOUT ) {
					throw Mishap( "Trying to take the length of an atomic object" ).culprit( "Object", refToString( obj_K ) );
				} else {
					switch ( KindOfSimpleKey( key ) ) {
						case VECTOR_KIND: {
							vm->fastPeek() = obj_K[ VECTOR_LAYOUT_OFFSET_LENGTH ];
							break;
						}
						case ATTR_KIND:
						case MIXED_KIND: {
							vm->fastPeek() = obj_K[ MIXED_LAYOUT_OFFSET_LENGTH ];
							break;
						}
						case PAIR_KIND: {
							pc = sysListLength( pc, vm );
							break;
						}
						case STRING_KIND: {
							vm->fastPeek() = LongToSmall( sizeAfterKeyOfVectorLayout( obj_K ) );	// Same as pc = sysStringLength( pc, vm );
							break;
						}
						case MAP_KIND: {
							vm->fastPeek() = fastMapCount( r );
							break;
						}
						case EXTERNAL_KIND:
						case WRECORD_KIND:
						case RECORD_KIND: {
							//vm->fastPeek() = LongToSmall( sizeAfterKeyOfRecordLayout( obj_K ) );
							throw Mishap( "Trying to take the length of a record" ).culprit( "Object", refToString( obj_K ) );
							break;
						}
						default: {
							throw "unimplemented (other)";
						}
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

Ref * sysIsntAbsent( Ref *pc, class MachineClass * vm ) {
	vm->fastPeek() = vm->fastPeek() == SYS_ABSENT ? SYS_FALSE : SYS_TRUE;
	return pc;
}

Ref * sysAbsNot( Ref *pc, class MachineClass * vm ) {
	vm->fastPeek() = vm->fastPeek() == SYS_ABSENT ? SYS_PRESENT : SYS_ABSENT;
	return pc;
}

Ref * sysBoolAbs( Ref *pc, class MachineClass * vm ) {
	if ( vm->fastPeek() == SYS_FALSE ) {
		vm->fastPeek() = SYS_ABSENT;
	}
	return pc;
}





// -----------------------------------------------------------------------------


#include "datatypes.cpp.auto"

//typedef std::map< std::string, SysInfo > SysMap;
typedef SysMap::value_type SysMaplet;

SysMap & SysMap::sysMap() {

	static SysMap::value_type rawData[] = {
		SysMaplet( "not", SysInfo( vmc_not, Arity( 1 ), Arity( 1 ), "Negates a boolean value" ) ),
		SysMaplet( "boolAbs", SysInfo( Arity( 1 ), Arity( 1 ), sysBoolAbs, "Maps boolean into absence: false -> absent, true -> true" ) ),
		SysMaplet( "absNot", SysInfo( Arity( 1 ), Arity( 1 ), sysAbsNot, "Negation of presence: absent->present; non-absent->absent" ) ),
		SysMaplet( "+", SysInfo( vmc_add, Arity( 2 ), Arity( 1 ), "Adds two numbers" ) ),
		SysMaplet( "-", SysInfo( vmc_sub, Arity( 2 ), Arity( 1 ), "Substracts two numbers" ) ),
		SysMaplet( "negate", SysInfo( vmc_neg, Arity( 1 ), Arity( 1 ), "Negates a number" ) ),
		SysMaplet( "*", SysInfo( vmc_mul, Arity( 2 ), Arity( 1 ), "Multiplies two numbers" ) ),
		SysMaplet( "/", SysInfo( vmc_div, Arity( 2 ), Arity( 1 ), "Divides two numbers, result is a float" ) ),
		SysMaplet( "quo", SysInfo( vmc_quo, Arity( 2 ), Arity( 1 ), "Integer quotient" ) ),
		SysMaplet( "mod", SysInfo( vmc_mod, Arity( 2 ), Arity( 1 ), "Integer modulus" ) ),
		//SysMaplet( "**", SysInfo( VM_OP_FLAVOUR, vmc_pow, fnc_pow, Arity( 2 ), Arity( 1 ), 0, "Exponentiates two numbers, result is float" ) ),
		SysMaplet( "<", SysInfo( CMP_LT, Arity( 2 ), Arity( 1 ), "Less than, compares two real numbers" ) ),
		SysMaplet( "<=", SysInfo( CMP_LTE, Arity( 2 ), Arity( 1 ), "Less than or equal to, compares two real number" ) ),
		SysMaplet( "==", SysInfo( CMP_EQ, Arity( 2 ), Arity( 1 ), "Identity of two values, result is boolean" ) ),
		SysMaplet( ">", SysInfo( CMP_GT, Arity( 2 ), Arity( 1 ), "Greater than, compares two real numbers" ) ),
		SysMaplet( ">=", SysInfo( CMP_GTE, Arity( 2 ), Arity( 1 ), "Greater than or equal to, compares two real numbers" ) ),	
		SysMaplet( "className", SysInfo( Arity( 1 ), Arity( 1 ), sysKeyName, "Returns the name of a key" ) ),
		SysMaplet( "dataClass", SysInfo( Arity( 1 ), Arity( 1 ), sysObjectKey, "Returns the key of any value" ) ),
		SysMaplet( "makeSymbol", SysInfo( Arity( 1 ), Arity( 1 ), sysMakeSymbol, "Returns a symbol with the same spelling as the string" ) ),
		SysMaplet( "newRecordClass", SysInfo( Arity( 2 ), Arity( 1 ), sysNewRecordClass, "Returns a new class object for records" ) ),
		SysMaplet( "newClass", SysInfo( Arity( 4 ), Arity( 1 ), sysNewClass, "Returns a new class object for instances" ) ),
		SysMaplet( "newClassRecogniser", SysInfo( Arity( 1 ), Arity( 1 ), sysClassRecogniser, "Returns a function that recognises a given class" ) ),
		SysMaplet( "newClassConstructor", SysInfo( Arity( 1 ), Arity( 1 ), sysClassConstructor, "Returns the constructor for a given class" ) ),
		SysMaplet( "newClassAccessor", SysInfo( Arity( 1 ), Arity( 1 ), sysClassAccessor, "Returns the accessor for a given class" ) ),
		SysMaplet( "newClassExploder", SysInfo( Arity( 1 ), Arity( 0, true ), sysClassExploder, "Returns the exploder for a given class" ) ),
		SysMaplet( "newMethod", SysInfo( Arity( 3 ), Arity( 1 ), sysNewMethod, "Constructs a new empty method" ) ),
		SysMaplet( "setMethod", SysInfo( Arity( 3 ), Arity( 0 ), sysSetMethod, "Sets the function for a method on an object" ) ),
		SysMaplet( "setSlot", SysInfo( Arity( 3 ), Arity( 0 ), sysSetSlot, "Sets a method as the Nth slot of a class" ) ),
		SysMaplet( "=", SysInfo( Arity( 2 ), Arity( 1 ), sysEquals, "Compare any two values as equal" ) ),
		SysMaplet( "sysgarbage", SysInfo( Arity( 0 ), Arity( 0 ), sysGarbageCollect, "Forces a garbage collection - useful for tests" ) ),
		SysMaplet( "hash", SysInfo( Arity( 1 ), Arity( 1 ), sysHash, "Computes a hash code for any value, returns a positive Small" ) ),
		SysMaplet( "index", SysInfo( Arity( 2 ), Arity( 1 ), sysIndex, "Indexes any sequence" ) ),
		SysMaplet( "updaterOfIndex", SysInfo( Arity( 3 ), Arity( 0 ), sysUpdaterOfIndex, "Updates the index of any sequence" ) ),
		SysMaplet( "mapIndex", SysInfo( Arity( 2 ), Arity( 1 ), sysMapIndex, "Indexes a map" ) ),
		SysMaplet( "listIndex", SysInfo( Arity( 2 ), Arity( 1 ), sysListIndex, "Indexes a list" ) ),
		SysMaplet( "explode", SysInfo( Arity( 1 ), Arity( 0, true ), sysExplode, "Explodes any sequence into its members" ) ),
		SysMaplet( "updaterOfExplode", SysInfo( Arity( 1, true ), Arity( 0 ), sysUpdaterOfExplode, "Updates the explode value of any sequence i.e. fills" ) ),
		SysMaplet( "listExplode", SysInfo( Arity( 1 ), Arity( 0, true ), sysListExplode, "Explodes a list into its members" ) ),
		SysMaplet( "length", SysInfo( Arity( 1 ), Arity( 1 ), sysLength, "Returns the length of any sequence" ) ),	
		SysMaplet( "listLength", SysInfo( Arity( 1 ), Arity( 1 ), sysListLength, "Returns the length of a list" ) ),	
		SysMaplet( "append", SysInfo( Arity( 2 ), Arity( 1 ), sysAppend, "Appends two sequences of the same type, result is a sequence of the same type" ) ),	
		SysMaplet( "listAppend", SysInfo( Arity( 2 ), Arity( 1 ), sysListAppend, "Appends two lists" ) ),	
		SysMaplet( "isNil", SysInfo( Arity( 1 ), Arity( 1 ), sysIsNil, "Tests whether a list is empty or not" ) ),
		SysMaplet( "isList", SysInfo( Arity( 1 ), Arity( 1 ), sysIsList, "Tests whether an object is a list or not" ) ),	
		SysMaplet( "isntAbsent", SysInfo( Arity( 1 ), Arity( 1 ), sysIsntAbsent, "Tests whether an object is non-absent" ) ),
		SysMaplet( "newList", SysInfo( Arity( 0, true ), Arity( 1 ), sysNewList, "Builds a list from the arguments" ) ),
		SysMaplet( "newListOnto", SysInfo( Arity( 1, true ), Arity( 1 ), sysNewListOnto, "Builds a list from the first N-1 args and appends that to the last argument" ) ),
		SysMaplet( "newMap", SysInfo( Arity( 0, true ), Arity( 1 ), sysNewHardEqMap, "Builds a new map from maps and maplets" ) ),
		SysMaplet( "newHardIdMap", SysInfo( Arity( 0, true ), Arity( 1 ), sysNewHardIdMap, "Builds a new map from maps and maplets that uses identity (==) for the equality test" ) ),
		SysMaplet( "newWeakIdMap", SysInfo( Arity( 0, true ), Arity( 1 ), sysNewWeakIdMap, "Builds a new map from maps and maplets that only weakly retains its entries" ) ),
		SysMaplet( "newCacheMap", SysInfo( Arity( 0, true ), Arity( 1 ), sysNewCacheEqMap, "Builds a new map from maps and maplets that the garbage collector can clear under pressure" ) ),
		SysMaplet( "mapIndex", SysInfo( Arity( 2 ), Arity( 1 ), sysMapIndex, "Looks up a map for a given index" ) ),
		SysMaplet( "partApply", SysInfo( Arity( 1, true ), Arity( 1 ), sysPartApply, "Freezes arguments and a function together to make a new function" ) ),
		SysMaplet( "functionInArity", SysInfo( Arity( 1  ), Arity( 1 ), sysFunctionInArity, "Input arity of a function" ) ),
		SysMaplet( "functionOutArity", SysInfo( Arity( 1 ), Arity( 1 ), sysFunctionOutArity, "Output arity of a function" ) ),
		#include "sysapply.inc"
		#include "sysattrmap.inc"
		#include "syscgi.inc"
		#include "syselement.inc"
		#include "sysexception.inc"
		#include "sysstring.inc"
		#include "sysunix.inc"
		#include "sysvector.inc"
		#include "sysmap.inc.auto"
	};

	static SysMap s( rawData, rawData + sizeof rawData / sizeof rawData[0] );	
	
	return s;
}