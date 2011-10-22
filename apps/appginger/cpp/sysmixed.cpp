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

#include <string.h>


Ref * sysMixedIndex( Ref *pc, class MachineClass * vm ) {
	if ( vm->count != 2 ) throw Ginger::Mishap( "ArgsMismatch" );
	Ref idx = vm->fastPop();
	if ( !IsSmall( idx ) ) throw Ginger::Mishap( "TypeError" );
	Ref vec = vm->fastPeek();
	if ( !IsMixedKind( vec ) ) throw Ginger::Mishap( "TypeError" );
	Ref * vec_K = RefToPtr4( vec );
	
	const long I = SmallToLong( idx );
	const long N = SmallToLong( vec_K[ MIXED_LAYOUT_OFFSET_LENGTH ] );
	
	if ( 1 <= I && I <= N ) {
		vm->fastPeek() = vec_K[ I + LengthOfSimpleKey( *vec_K ) ];
	} else {
		throw Ginger::Mishap( "OutOfRange" );
	}
	
	return pc;
}

Ref * sysMixedExplode( Ref *pc, class MachineClass * vm ) {
	if ( vm->count != 1 ) throw Ginger::Mishap( "Wrong number of arguments for mixedExplode" );
	
	Ref r = vm->fastPop();
	
	if ( !IsMixedKind( r ) ) throw Ginger::Mishap( "Argument mismatch for mixedExplode" );
	
	Ref *obj_K = RefToPtr4( r );
	unsigned long n = lengthOfMixedLayout( obj_K );
	vm->checkStackRoom( n );
	memcpy( vm->vp + 1, obj_K + 1 + numFieldsOfMixedLayout( obj_K ), n * sizeof( Ref ) );
	vm->vp += n;
	
	return pc;
}

Ref * sysMixedLength( Ref *pc, class MachineClass * vm ) {
	if ( vm->count != 1 ) throw Ginger::Mishap( "Wrong number of arguments for mixedLength" );
	Ref r = vm->fastPeek();
	if ( !IsMixedKind( r ) ) throw Ginger::Mishap( "Argument mismatch for mixedLength" );
	Ref * obj_K = RefToPtr4( r );
	
	vm->fastPeek() = LongToSmall( lengthOfMixedLayout( obj_K ) );
	return pc;
}

Ref * sysFastMixedLength( Ref *pc, class MachineClass * vm ) {
	Ref r = vm->fastPeek();
	Ref * obj_K = RefToPtr4( r );
	vm->fastPeek() = LongToSmall( lengthOfMixedLayout( obj_K ) );
	return pc;
}


