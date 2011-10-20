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

#include "key.hpp"
#include "common.hpp"
#include "misclayout.hpp"
#include "sysvector.hpp"
#include "vectorlayout.hpp"

#include <string.h>

Ref * sysVectorAppend( Ref * pc, class MachineClass * vm ) {

	//	Variables here would be unaffected by a GC.
	unsigned long N;
	unsigned long lhs_n;
	unsigned long rhs_n;

	if ( vm->count != 2 ) throw Ginger::Mishap( "Wrong number of arguments in vectorAppend" );

	{
	
		//	May need to GC so leave on the stack.
		Ref rhs = vm->fastPeek();
		Ref lhs = vm->fastPeek( 1 );
		
		if ( !IsObj( lhs ) || !IsObj( rhs ) ) throw Ginger::Mishap( "Invalid arguments in vectorAppend" );
		
		Ref * lhs_K = RefToPtr4( lhs );
		Ref * rhs_K = RefToPtr4( rhs );
		Ref lhs_key = *lhs_K;
		Ref rhs_key = *rhs_K;
		
		if ( lhs_key != rhs_key || !IsSimpleKey( lhs_key ) || KindOfSimpleKey( lhs_key ) != VECTOR_KIND ) throw Ginger::Mishap( "Invalid arguments in vectorAppend" );
	
		lhs_n = sizeAfterKeyOfVectorLayout( lhs_K );
		rhs_n = sizeAfterKeyOfVectorLayout( rhs_K );
		N = lhs_n + rhs_n;
	
	}
	
	XfrClass xfr( vm->heap().preflight( pc, N + 2 ) );

	//	No risk of GC so safe to pop.
	Ref * rhs_K = RefToPtr4( vm->fastPop() );
	Ref * lhs_K = RefToPtr4( vm->fastPop() );

	xfr.xfrRef( ULongToSmall( N ) );
	xfr.setOrigin();
	xfr.xfrRef( *lhs_K );
	xfr.xfrCopy( lhs_K + 1, lhs_n );
	xfr.xfrCopy( rhs_K + 1, rhs_n );

	vm->fastPush( xfr.makeRef() );
	return pc;

}

Ref * sysVectorIndex( Ref *pc, class MachineClass * vm ) {
	if ( vm->count != 2 ) throw Ginger::Mishap( "ArgsMismatch" );
	Ref idx = vm->fastPop();
	if ( !IsSmall( idx ) ) throw Ginger::Mishap( "TypeError" );
	Ref vec = vm->fastPeek();
	if ( !IsVector( vec ) ) throw Ginger::Mishap( "TypeError" );
	Ref * vec_K = RefToPtr4( vec );
	
	const long I = SmallToLong( idx );
	const long N = SmallToLong( vec_K[ VECTOR_OFFSET_LENGTH ] );
	
	if ( 1 <= I && I <= N ) {
		vm->fastPeek() = vec_K[ I ];
	} else {
		throw Ginger::Mishap( "OutOfRange" );
	}
	
	return pc;
}

Ref * sysVectorExplode( Ref *pc, class MachineClass * vm ) {
	if ( vm->count != 1 ) throw Ginger::Mishap( "Wrong number of arguments for vectorExplode" );
	
	Ref r = vm->fastPop();
	
	if ( !IsVectorKind( r ) ) throw Ginger::Mishap( "Argument mismatch for vectorExplode" );
	
	Ref *obj_K = RefToPtr4( r );
	unsigned long n = sizeAfterKeyOfVectorLayout( obj_K );
	vm->checkStackRoom( n );
	memcpy( vm->vp + 1, obj_K + 1, n * sizeof( Ref ));
	vm->vp += n;
	
	return pc;
}

Ref * sysVectorLength( Ref *pc, class MachineClass * vm ) {
	if ( vm->count != 1 ) throw Ginger::Mishap( "Wrong number of arguments for vectorLength" );
	Ref r = vm->fastPeek();
	if ( !IsVector( r ) ) throw Ginger::Mishap( "Argument mismatch for vectorLength" );
	Ref * obj_K = RefToPtr4( r );
	
	vm->fastPeek() = LongToSmall( sizeAfterKeyOfVectorLayout( obj_K ) );
	return pc;
}

Ref * sysFastVectorLength( Ref *pc, class MachineClass * vm ) {
	Ref r = vm->fastPeek();
	Ref * obj_K = RefToPtr4( r );
	vm->fastPeek() = LongToSmall( sizeAfterKeyOfVectorLayout( obj_K ) );
	return pc;
}


