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

#include "sysfunction.hpp"
#include "functionlayout.hpp"
#include "cage.hpp"
#include "key.hpp"
#include "mishap.hpp"
#include "vmi.hpp"


/*
	This function is called with the stack looking like
	this:
		item1, ..., itemN, F
	It delivers a function which is equivalent to Pop-11's
		F(% item1, ..., itemN %)
		
		
	Note that the final push is entirely safe because there must have
	been some N+1 arguments.
*/
Ref * sysPartApply( Ref *pc, class MachineClass * vm ) {
	PlantClass * plant = vm->plant();
	
	//	We have to do a heap check immediately. This is intended to 
	//	ensure that garbage collection cannot occur before we exit this
	//	function & we can therefore not worry about creating temporary roots.
	//
	//	We exploit the fact that we can count the size of the PUSHQ and
	//	CHAIN_LITE instructions.
	XfrClass xfr( pc, vm->heap(), static_cast< int >( OFFSET_FROM_FN_LENGTH_TO_KEY + 1 + 2 * vm->count + 1 ) );
	
	Ref F = vm->fastPeek();
	
	//	F is required to be a function.
	if ( !IsObj( F ) ) throw Ginger::Mishap( "Function needed as last arg to partApply" );
	Ref * F_K = RefToPtr4( F );
	if ( !IsFunctionKey( *F_K ) ) throw Ginger::Mishap( "Function needed as last arg to partApply" );
	
	long F_in_arity = numInputsOfFn( F_K );
	long F_out_arity = numOutputsOfFn( F_K );
	
	
	int N = vm->count - 1;
	vm->vp -= N;				//	Drop the N items, items to be accessed by vm->vp[i]

	//	This seems very unlikely to be correct. We can't just go around
	//	substracting arities.
	vmiFUNCTION( plant, F_in_arity - N, F_out_arity );
	for ( int i = 0; i < N; i++ ) {
		vmiPUSHQ( plant, vm->vp[ i ] );
	}
	vmiCHAIN_LITE( plant, F, N );
	Ref part_apply = vmiENDFUNCTION( plant );
	
	vm->fastPeek() = part_apply;

	
	return pc;
}

Ref * sysFunctionInArity( Ref *pc, class MachineClass * vm ) {
	Ref fn = vm->fastPeek();
	
	//	F is required to be a function.
	if ( !IsObj( fn ) ) throw Ginger::Mishap( "Function needed" );
	Ref * fn_K = RefToPtr4( fn );
	if ( !IsFunctionKey( *fn_K ) ) throw Ginger::Mishap( "Function needed" );
	
	vm->fastPeek() = LongToSmall( numInputsOfFn( fn_K ) );
	
	return pc;
}

Ref * sysFunctionOutArity( Ref *pc, class MachineClass * vm ) {
	Ref fn = vm->fastPeek();
	
	//	F is required to be a function.
	if ( !IsObj( fn ) ) throw Ginger::Mishap( "Function needed" );
	Ref * fn_K = RefToPtr4( fn );
	if ( !IsFunctionKey( *fn_K ) ) throw Ginger::Mishap( "Function needed" );
	
	vm->fastPeek() = LongToSmall( numOutputsOfFn( fn_K ) );

	return pc;
}

