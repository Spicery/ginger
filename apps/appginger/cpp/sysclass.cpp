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

#include <set>
#include <list>
using namespace std;

#include "sysclass.hpp"
#include "syskey.hpp"
#include "common.hpp"
#include "key.hpp"
#include "mishap.hpp"
//#include "vmi.hpp"
#include "machine.hpp"
#include "classlayout.hpp"
#include "misclayout.hpp"
#include "vectorlayout.hpp"
#include "roots.hpp"
#include "sysmethod.hpp"
#include "sysprint.hpp"

static long preflightNewClass( MachineClass * vm ) {
	Ref overrides = vm->vp[ 0 ];
	Ref parents = vm->vp[ -1 ];
	Ref slots = vm->vp[ -2 ];

	if ( !IsVector( overrides ) ) throw Ginger::Mishap(  "Vector needed" );	
	if ( !IsVector( parents ) ) throw Ginger::Mishap(  "Vector needed" );	
	if ( !IsVector( slots ) ) throw Ginger::Mishap(  "Vector needed" );	
	
	long sofar = lengthOfVectorLayout( RefToPtr4( slots ) );
		
	const long N = lengthOfVectorLayout( RefToPtr4( parents ) );
	for ( long i = 1; i <= N; i++ ) {
		Ref c = INDEX( parents, i );
		if ( !IsClass( c ) ) throw Ginger::Mishap(  "Class needed" );
		const long nfields = SmallToLong( INDEX( c, CLASS_OFFSET_NFIELDS ) );
		sofar += nfields;
	}
	
	return sofar;
}

/*
	We expect the stack to consist of
		slots : Vector( Method ),
		parents : Vector( Class ),
		overrides : Vector( Method )
*/
static Ref * sysClassSlots( Ref * pc, MachineClass * vm ) {
	if ( vm->count != 3 ) throw Ginger::Mishap( "ArgsMismatch" );
	
	const long P = preflightNewClass(vm );		//	Additionally performs type check.
	vm->heap().preflight( pc, P + 2 );		//	2 = Vector overhead.
	
	vm->gcVeto();
	
	Ref overrides = vm->fastPop();
	Ref parents = vm->fastPop();
	Ref primary_slots = vm->fastPop();	
		
	list< Ref > required_slots;
	set< Ref > seen;
	{
		// 	Add overrides to initial 'seen'.
		const long n = lengthOfVectorLayout( RefToPtr4( overrides ) );
		for ( long i = 0; i < n; i++ ) {
			seen.insert( INDEX( overrides, i ) );
		}
	}
	{
		//	Add parents to the slots.
		const long n = lengthOfVectorLayout( RefToPtr4( parents ) );
		for ( long i = 1; i <= n; i++ ) {
			Ref gclass = INDEX( parents, i );
			Ref parent_slots = INDEX( gclass, CLASS_OFFSET_SLOTS );
			const long nfields = lengthOfVectorLayout( RefToPtr4( parent_slots ) );
			for ( long j = 1; j <= nfields; j++ ) {
				Ref m = INDEX( parent_slots, j );
				if ( seen.find( m ) == seen.end() ) {
					seen.insert( m );
					required_slots.push_back( m );
				}
			}
		}
	}
	{
		//	Add primary slots.
		const long n = lengthOfVectorLayout( RefToPtr4( primary_slots ) );
		for ( long i = 1; i <= n; i++ ) {
			Ref m = INDEX( primary_slots, i );
			if ( seen.find( m ) == seen.end() ) {	
				seen.insert( m );
				required_slots.push_back( m );
			}
		}		
	}
	
	const long Q = required_slots.size();
	if ( Q > P ) {
		throw Ginger::SystemError( "Incorrect calculation of slots for class!" );
	}
	
	XfrClass xfr( vm->heap().preflight( 2 + Q ) );
	xfr.xfrRef( LongToSmall( Q ) );
	xfr.setOrigin();
	xfr.xfrRef( sysVectorKey );
	for ( list< Ref >::iterator it = required_slots.begin(); it != required_slots.end(); ++it ) {
		xfr.xfrRef( *it );
	}
	
	//	Plenty of room for this push. 3 words free on the value stack.
	vm->fastPush( xfr.makeRef() );
	
	vm->gcLiftVeto();
	return pc;
}

Ref * sysNewClass( Ref * pc, MachineClass * vm ) {
	if ( vm->count != 4 ) throw Ginger::Mishap( "ArgsMismatch" );
	
	//	Takes top 3 items on stack and returns 1.
	vm->count = 3;
	pc = sysClassSlots( pc, vm );		
	
	//	Save the slots list and compute a new class object.
	Roots roots( vm );
	Ref & slots = roots.ref( vm->fastPeek() );
	vm->fastPeek() = RefToPtr4( slots )[ VECTOR_LAYOUT_OFFSET_LENGTH ];
	
	//	Compute the new class object.
	vm->count = 2;
	pc = sysNewRecordClass( pc, vm );
	Ref & gclass = roots.ref( vm->fastPop() );
	
	//	For each position, set up the slot.
	const long nslots = lengthOfVectorLayout( RefToPtr4( slots ) );
	for ( long i = 1; i <= nslots; i++ ) {
		vm->count = 3;
		vm->fastPush( gclass );
		vm->fastPush( LongToSmall( i ) );
		vm->fastPush( INDEX( slots, i ) );
		pc = sysSetSlot( pc, vm );
	}
	
	vm->fastPush( gclass );
	return pc;
}

Ref * sysNewRecordClass( Ref * pc, MachineClass * vm ) {
	if ( vm->count == 2 ) {

		//	A Small is GC insensitive, so it is OK to grab it.
		Ref nfields = vm->fastPeek();
		if ( !IsSmall( nfields ) ) throw Ginger::Mishap( "Non-integer argument to newRecordClass" );

		{
			long n = SmallToLong( nfields );
			XfrClass xfr( vm->heap().preflight( pc, n + 2 ) );
			xfr.xfrRef( nfields );
			xfr.setOrigin();
			xfr.xfrRef( sysVectorKey );
			xfr.xfrDup( SYS_ABSENT, n );
			vm->fastPeek() = xfr.makeRef();
		}
		{
			XfrClass xfr( vm->heap().preflight( pc, CLASS_SIZE ) );
	
			//	Pop stack to the correct size.
			Ref slots = vm->fastPop();
	
			//	Wait to grab title until we're in a GC-free region.
			Ref title = vm->fastPeek();
	
			xfr.setOrigin();
			xfr.xfrRef( sysClassKey );
			xfr.xfrRef( title );
			xfr.xfrRef( nfields );
			xfr.xfrRef( SYS_ABSENT );
			xfr.xfrRef( slots );
			
			vm->fastPeek() = xfr.makeRef();
		}
		return pc;
	} else {
		throw Ginger::Mishap( "Wrong number of arguments" );
	}
}

static Ref * sysargRecognise( Ref * pc, MachineClass * vm ) {
	if ( vm->count != 1 ) throw Ginger::Mishap( "Wrong number of arguments" );
	Ref this_item = vm->fastPeek();
	Ref that_key = pc[ -1 ];
	vm->fastPeek() = refDataKey( this_item ) == that_key ? SYS_TRUE : SYS_FALSE;
	return pc;
}

Ref * sysClassRecogniser( Ref * pc, MachineClass *vm ) {
	if ( vm->count != 1 ) throw Ginger::Mishap( "Wrong number of arguments" );
	Ref kk = vm->fastPeek();
	if ( !IsObj( kk ) || *RefToPtr4( kk ) != sysClassKey ) throw Ginger::Mishap( "Key needed" );
	CodeGen codegen = vm->codegen();
	codegen->vmiFUNCTION( 1, 1 );
	codegen->vmiSYS_CALL_ARG( sysargRecognise, kk );
	codegen->vmiSYS_RETURN();
	vm->fastPeek() = codegen->vmiENDFUNCTION();
	return pc;
}

//#include <iostream>
//#include "sysprint.hpp"

static Ref * sysargdatConstruct( Ref * pc, MachineClass *vm ) {
	long N = ToULong( pc[-1] );
	Ref kk = pc[-2];
	if ( vm->count != N ) throw Ginger::Mishap( "Wrong number of arguments" );
	//std::cout << "NFIELDS = " << N << std::endl;
	//refPrint( std::cout, kk );
	//std::cout << std::endl;
	XfrClass xfr( vm->heap().preflight( pc, N + 1 ) );
	xfr.setOrigin();
	xfr.xfrRef( kk );
	vm->vp -= N;
	xfr.xfrCopy( vm->vp + 1, N );
	vm->fastPush( xfr.makeRef() );
	return pc;
}

Ref * sysClassConstructor( Ref * pc, MachineClass *vm ) {
	if ( vm->count != 1 ) throw Ginger::Mishap( "Wrong number of arguments" );
	Ref kk = vm->fastPeek();
	if ( !IsObj( kk ) || *RefToPtr4( kk ) != sysClassKey ) throw Ginger::Mishap( "Key needed" );
	Ref * obj_K = RefToPtr4( kk );
	long n = SmallToLong( obj_K[ CLASS_OFFSET_NFIELDS ] );
	CodeGen codegen = vm->codegen();
	codegen->vmiFUNCTION( n, 1 );
	//vmiCHECK_COUNT( codegen, n );
	codegen->vmiSYS_CALL_ARGDAT( sysargdatConstruct, kk, n );
	codegen->vmiSYS_RETURN();
	vm->fastPeek() = codegen->vmiENDFUNCTION();
	return pc;
}

static Ref * sysargdatAccess( Ref * pc, MachineClass *vm ) {
	unsigned long N = ToULong( pc[-1] );
	Ref that_key = pc[-2];
	if ( vm->count != 1 ) throw Ginger::Mishap( "Wrong number of arguments" );
	Ref this_item  = vm->fastPeek();
	if ( refDataKey( this_item ) == that_key ) {
		vm->fastPeek() = RefToPtr4( this_item )[ N ];
	} else {
		throw Ginger::Mishap( "ToBeDone" );
	}
	return pc;
}

Ref * sysClassAccessor( Ref * pc, MachineClass *vm ) {
	if ( vm->count != 2 ) throw Ginger::Mishap( "Wrong number of arguments" );
	Ref N = vm->fastPop();
	if ( !IsSmall( N ) ) throw Ginger::Mishap( "Integer index needed" );
	Ref kk = vm->fastPeek();
	if ( !isKey( kk ) ) throw Ginger::Mishap( "Key needed" );
	long nargs = SmallToLong( RefToPtr4( kk )[ CLASS_OFFSET_NFIELDS ] );
	long index = SmallToLong( N );
	if ( 1 <= index && index <= nargs ) {
		CodeGen codegen = vm->codegen();
		codegen->vmiFUNCTION( 1, 1 );
		codegen->vmiSYS_CALL_ARGDAT( sysargdatAccess, kk, index );
		codegen->vmiSYS_RETURN();
		vm->fastPeek() = codegen->vmiENDFUNCTION();
	} else {
		throw Ginger::Mishap( "ToBeDone" );
	}
	return pc;
}

Ref * sysClassUnsafeAccessor( Ref * pc, MachineClass *vm ) {
	if ( vm->count != 2 ) throw Ginger::Mishap( "Wrong number of arguments" );
	Ref N = vm->fastPop();
	if ( !IsSmall( N ) ) throw Ginger::Mishap( "Integer index needed" );
	Ref kk = vm->fastPeek();
	if ( !isKey( kk ) ) throw Ginger::Mishap( "Key needed" );
	long nargs = SmallToLong( RefToPtr4( kk )[ CLASS_OFFSET_NFIELDS ] );
	long index = SmallToLong( N );
	if ( 1 <= index && index <= nargs ) {
		CodeGen codegen = vm->codegen();
		codegen->vmiFUNCTION( 1, 1 );
		codegen->vmiFIELD( index );
		codegen->vmiSYS_RETURN();
		vm->fastPeek() = codegen->vmiENDFUNCTION();
	} else {
		throw Ginger::Mishap( "ToBeDone" );
	}
	return pc;
}

static Ref * sysargExplode( Ref * pc, MachineClass * vm ) {
	Ref the_key = pc[-1];
	long nfields = SmallToLong( RefToPtr4( the_key )[ CLASS_OFFSET_NFIELDS ] );
	if ( vm->count != 1 ) throw Ginger::Mishap( "Wrong number of arguments" );
	Ref obj = vm->fastPop();
	if ( !IsObj( obj ) ) throw Ginger::Mishap( "Object needed" ).culprit( "Argument", refToString( obj ) );
	Ref * obj_K = RefToPtr4( obj );
	if ( obj_K[ 0 ] != the_key ) throw Ginger::Mishap( "Wrong type of Object" ).culprit( "Object", refToString( obj ) );
	for ( int i = 1; i <= nfields; i++ ) {
		vm->fastPush( obj_K[ i ] );
	}
	return pc;
}

Ref * sysClassExploder( Ref * pc, MachineClass * vm ) {
	if ( vm->count != 1 ) throw Ginger::Mishap( "Wrong number of arguments" );
	Ref key = vm->fastPeek();
	if ( !IsObj( key ) ) throw Ginger::Mishap( "Class of object needed" );
	Ref * key_K = RefToPtr4( key );
	if ( *key_K != sysClassKey ) throw Ginger::Mishap( "Class of object needed" );
	const long N = SmallToLong( key_K[ CLASS_OFFSET_NFIELDS ] );

	CodeGen codegen = vm->codegen();
	codegen->vmiFUNCTION( 1, N );
	codegen->vmiSYS_CALL_ARG( sysargExplode, key );
	codegen->vmiSYS_RETURN();
	vm->fastPeek() = codegen->vmiENDFUNCTION();
	return pc;
}
