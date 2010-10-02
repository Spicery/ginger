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

#include "syskey.hpp"

#include "common.hpp"
#include "key.hpp"
#include "mishap.hpp"
#include "vmi.hpp"
#include "machine.hpp"
#include "classlayout.hpp"

Ref * sysNewRecordClass( Ref * pc, MachineClass * vm ) {
	if ( vm->count == 2 ) {
		Ref nfields = vm->fastPop();
		Ref title = vm->fastPop();
		if ( !IsSmall( nfields ) ) throw Mishap( "Non-integer argument to newRecordClass" );
		XfrClass xfr( vm->heap().preflight( pc, 1 ) );
		xfr.setOrigin();
		xfr.xfrRef( sysKeyKey );
		xfr.xfrRef( title );
		xfr.xfrRef( nfields );
		xfr.xfrRef( sys_absent );
		vm->fastPush( xfr.makeRef() );
		return pc;
	} else {
		throw Mishap( "Wrong number of arguments" );
	}
}

static Ref * sysargRecognise( Ref * pc, MachineClass * vm ) {
	if ( vm->count != 1 ) throw Mishap( "Wrong number of arguments" );
	Ref this_item = vm->fastPeek();
	Ref that_key = pc[ -1 ];
	vm->fastPeek() = refKey( this_item ) == that_key ? sys_true : sys_false;
	return pc;
}

Ref * sysClassRecogniser( Ref * pc, MachineClass *vm ) {
	if ( vm->count != 1 ) throw Mishap( "Wrong number of arguments" );
	Ref kk = vm->fastPeek();
	if ( !IsObj( kk ) || *RefToPtr4( kk ) != sysKeyKey ) throw Mishap( "Key needed" );
	Plant plant = vm->plant();
	vmiFUNCTION( plant, 1, 1 );
	vmiSYS_CALL_ARG( plant, sysargRecognise, kk );
	vmiSYS_RETURN( plant );
	vm->fastPeek() = vmiENDFUNCTION( plant );
	return pc;
}

//#include <iostream>
//#include "sysprint.hpp"

static Ref * sysargdatConstruct( Ref * pc, MachineClass *vm ) {
	long N = ToULong( pc[-1] );
	Ref kk = pc[-2];
	if ( vm->count != N ) throw Mishap( "Wrong number of arguments" );
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
	if ( vm->count != 1 ) throw Mishap( "Wrong number of arguments" );
	Ref kk = vm->fastPeek();
	if ( !IsObj( kk ) || *RefToPtr4( kk ) != sysKeyKey ) throw Mishap( "Key needed" );
	Ref * obj_K = RefToPtr4( kk );
	long n = SmallToLong( obj_K[ CLASS_OFFSET_NFIELDS ] );
	Plant plant = vm->plant();
	vmiFUNCTION( plant, n, 1 );
	//vmiCHECK_COUNT( plant, n );
	vmiSYS_CALL_ARGDAT( plant, sysargdatConstruct, kk, n );
	vmiSYS_RETURN( plant );
	vm->fastPeek() = vmiENDFUNCTION( plant );
	return pc;
}

static Ref * sysargdatAccess( Ref * pc, MachineClass *vm ) {
	unsigned long N = ToULong( pc[-1] );
	Ref that_key = pc[-2];
	if ( vm->count != 1 ) throw Mishap( "Wrong number of arguments" );
	Ref this_item  = vm->fastPeek();
	if ( refKey( this_item ) == that_key ) {
		vm->fastPeek() = RefToPtr4( this_item )[ N ];
	} else {
		throw ToBeDone();
	}
	return pc;
}

Ref * sysClassAccessor( Ref * pc, MachineClass *vm ) {
	if ( vm->count != 2 ) throw Mishap( "Wrong number of arguments" );
	Ref N = vm->fastPop();
	if ( !IsSmall( N ) ) throw Mishap( "Integer index needed" );
	Ref kk = vm->fastPeek();
	if ( !isKey( kk ) ) throw Mishap( "Key needed" );
	long nargs = SmallToLong( RefToPtr4( kk )[ CLASS_OFFSET_NFIELDS ] );
	long index = SmallToLong( N );
	if ( 1 <= index && index <= nargs ) {
		Plant plant = vm->plant();
		vmiFUNCTION( plant, 1, 1 );
		vmiSYS_CALL_ARGDAT( plant, sysargdatAccess, kk, index );
		vmiSYS_RETURN( plant );
		vm->fastPeek() = vmiENDFUNCTION( plant );
	} else {
		throw ToBeDone();
	}
	return pc;
}

Ref * sysClassUnsafeAccessor( Ref * pc, MachineClass *vm ) {
	if ( vm->count != 2 ) throw Mishap( "Wrong number of arguments" );
	Ref N = vm->fastPop();
	if ( !IsSmall( N ) ) throw Mishap( "Integer index needed" );
	Ref kk = vm->fastPeek();
	if ( !isKey( kk ) ) throw Mishap( "Key needed" );
	long nargs = SmallToLong( RefToPtr4( kk )[ CLASS_OFFSET_NFIELDS ] );
	long index = SmallToLong( N );
	if ( 1 <= index && index <= nargs ) {
		Plant plant = vm->plant();
		vmiFUNCTION( plant, 1, 1 );
		vmiFIELD( plant, index );
		vmiSYS_RETURN( plant );
		vm->fastPeek() = vmiENDFUNCTION( plant );
	} else {
		throw ToBeDone();
	}
	return pc;
}

static Ref * sysargExplode( Ref * pc, MachineClass * vm ) {
	Ref the_key = pc[-1];
	long nfields = SmallToLong( RefToPtr4( the_key )[ CLASS_OFFSET_NFIELDS ] );
	if ( vm->count != 1 ) throw Mishap( "Wrong number of arguments" );
	Ref obj = vm->fastPop();
	if ( !IsObj( obj ) ) throw Mishap( "Object needed" ).culprit( "Argument", obj );
	Ref * obj_K = RefToPtr4( obj );
	if ( obj_K[ 0 ] != the_key ) throw Mishap( "Wrong type of Object" ).culprit( "Object", obj );
	for ( int i = 1; i <= nfields; i++ ) {
		vm->fastPush( obj_K[ i ] );
	}
	return pc;
}

Ref * sysClassExploder( Ref * pc, MachineClass * vm ) {
	if ( vm->count != 1 ) throw Mishap( "Wrong number of arguments" );
	Ref key = vm->fastPeek();
	if ( !IsObj( key ) ) throw Mishap( "Class of object needed" );
	Ref * key_K = RefToPtr4( key );
	if ( *key_K != sysKeyKey ) throw Mishap( "Class of object needed" );
	const long N = SmallToLong( key_K[ CLASS_OFFSET_NFIELDS ] );

	Plant plant = vm->plant();
	vmiFUNCTION( plant, 1, N );
	vmiSYS_CALL_ARG( plant, sysargExplode, key );
	vmiSYS_RETURN( plant );
	vm->fastPeek() = vmiENDFUNCTION( plant );
	return pc;
}
