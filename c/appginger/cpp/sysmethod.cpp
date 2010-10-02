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

#include "sysmethod.hpp"

#include "common.hpp"
#include "machine.hpp"

#include "mishap.hpp"
#include "vmi.hpp"

Ref * sysNewMethod( Ref * pc, MachineClass * vm ) {
	//	newMethod( name:String, ninputs:Small, noutputs:Small ) -> m:Method
	if ( vm->count != 3 ) throw Mishap( "Wrong number of arguments" );
	
	Ref noutputs = vm->fastPop();
	Ref ninputs = vm->fastPop();
	/*Ref name =*/ vm->fastPop();	//	Currently discarded.
	
	if ( !IsSmall( noutputs ) || !IsSmall( ninputs ) ) throw Mishap( "Invalid arguments" ).culprit( "#Outputs", noutputs ).culprit( "Inputs", ninputs );
	
	Plant plant = vm->plant();
	vmiFUNCTION( plant, SmallToLong( ninputs ), SmallToLong( noutputs ) );
	vmiINVOKE( plant );
	Ref r = vmiENDFUNCTION( plant, sysMethodKey );
	vm->fastPush( r );	//	No check needed, as stack has room for 3.
	return pc;
}

Ref * sysSetMethod( Ref * pc, MachineClass * vm ) {
	
	//	setMethod( Class, Method, Function )
	if ( vm->count != 3 ) throw Mishap( "Wrong number of arguments" );
	
	//	We may need to allocate some store. In order to avoid doing
	//	this at an inconvenient time we will preflight enough store.
	//	This is a last chance to GC for a bit.
	vm->heap().preflight( pc, ASSOC_SIZE );
	vm->gcUnnecessary();
	
	//	These 3 variables are the reason we do not want a GC. These
	//	are not known roots.
	Ref function = vm->fastPop();
	Ref method = vm->fastPop();
	Ref gclass = vm->fastPop();
	
	if ( !IsFunction( function ) ) throw Mishap( "Function needed" );
	if ( !IsMethod( method ) ) throw Mishap( "Method needed" );
	if ( !IsClass( gclass ) ) throw Mishap( "Class needed" );
	
	Ref * gclass_K = RefToPtr4( gclass );
	Ref dispatch_table = gclass_K[ CLASS_OFFSET_DISPATCH_TABLE ];
	while ( dispatch_table != sys_absent ) {
		Ref * bucket_K = RefToPtr4( dispatch_table );
		if ( method == bucket_K[ ASSOC_KEY_OFFSET ] ) {	
			bucket_K[ ASSOC_VALUE_OFFSET ] = function;
			vm->gcNormal();
			return pc;
		} else {
			dispatch_table = bucket_K[ ASSOC_NEXT_OFFSET ];
		}
	}	
	
	// 	Failed to find a matching method, so insert bucket. Garbage
	//	collection must be inhibited in this section.
	
	XfrClass xfr( vm->heap().preflight( pc, ASSOC_SIZE ) );
	xfr.setOrigin();
	xfr.xfrRef( sysAssocKey );
	xfr.xfrRef( method );
	xfr.xfrRef( function );
	xfr.xfrRef( gclass_K[ CLASS_OFFSET_DISPATCH_TABLE ] );
	gclass_K[ CLASS_OFFSET_DISPATCH_TABLE ] = xfr.makeRef();
	
	vm->gcNormal();
	return pc;
}
