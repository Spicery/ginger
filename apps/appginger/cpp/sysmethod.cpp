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

/**
	setSlot( CLASS:Class, POSITION:Small, METHOD:Method )
	
	1. 	The method is inserted into the correct position in the class's
		slot array. To start with, only one method per slot will be
		permitted.
		
	2.	A call to setMethod is then made with an unsafe access function
		as the method's function. The values are passed on the stack. 
		No stack checks are needed as the size of the argument lists of
		the two functions are the same.
*/
Ref * sysSetSlot( Ref * pc, MachineClass * vm ) {
	if ( vm->count != 3 ) throw Mishap( "Wrong number of arguments" );

	Ref method = vm->fastPop();
	Ref position = vm->fastPop();
	Ref gclass = vm->fastPop();

	if ( !IsMethod( method ) ) throw Mishap( "Method needed" ).culprit( "Method", method );
	if ( !IsSmall( position ) ) throw Mishap( "Small needed" ).culprit( "Position", position );
	if ( !IsClass( gclass ) ) throw Mishap( "Class needed" ).culprit( "Class", gclass );
	
	long pos = SmallToLong( position );
	long nfields = SmallToLong( RefToPtr4( gclass )[ CLASS_OFFSET_NFIELDS ] );
	if ( not( 1 <= pos && pos <= nfields ) ) {
		throw 
			Mishap( "Position out of range" ).
			culprit( "Position", position ).
			culprit( "Number of fields", nfields )
		;
	}
	
	//	Update the class-slot.
	INDEX( INDEX( gclass, CLASS_OFFSET_SLOTS ), pos ) = method;

	//	Push onto the stack to get protection from garbage collection.
	vm->fastPush( gclass );
	vm->fastPush( method );

	//	ENDFUNCTION does not in fact cause a garbage collection, as it
	//	forces the heap to grow. However this is a more accurate way
	//	to write the code. 
	//
	//	The following block should not be in-lined but extracted as a 
	//	service function.
	{
		Plant plant = vm->plant();
		vmiFUNCTION( plant, 1, 1 );
		vmiFIELD( plant, pos );
		vmiSYS_RETURN( plant );
		vm->fastPush( vmiENDFUNCTION( plant ) );
	}

	//	We do not need to modify vm->count, it's already 3.
	//	Simply chain into sysSetMethod. 
	return sysSetMethod( pc, vm );
}

/*
	Expects the stack to consist of 
		class : Class, method : Method, function : Function
	Garbage collection is suspended during this call to permit
	variables to be held in non-root values.
*/
static Ref * gngSetMethod( Ref * pc, MachineClass * vm, const bool override ) {
	
	//	setMethod( Class, Method, Function )
	if ( vm->count != 3 ) throw Mishap( "Wrong number of arguments" );
	
	//	We may need to allocate some store. In order to avoid doing
	//	this at an inconvenient time we will preflight enough store.
	//	This is a last chance to GC for a bit.
	vm->heap().preflight( pc, ASSOC_SIZE );
	vm->gcVeto();
	
	//	These 3 variables are the reason we do not want a GC. These
	//	are not known roots. GC suspension could be avoided by indexing 
	//	the value-stack instead of popping it and then cutting the 
	//	stack on exit.
	Ref function = vm->fastPop();
	Ref method = vm->fastPop();
	Ref gclass = vm->fastPop();
	
	if ( !IsFunction( function ) ) throw Mishap( "Function needed" );
	if ( !IsMethod( method ) ) throw Mishap( "Method needed" );
	if ( !IsClass( gclass ) ) throw Mishap( "Class needed" );
	
	//Ref * gclass_K = RefToPtr4( gclass );
	Ref dispatch_table = INDEX( method, METHOD_OFFSET_DISPATCH_TABLE );
	while ( dispatch_table != sys_absent ) {
		Ref * bucket_K = RefToPtr4( dispatch_table );
		if ( gclass == bucket_K[ ASSOC_OFFSET_KEY ] ) {	
			bucket_K[ ASSOC_OFFSET_VALUE ] = function;
			vm->gcLiftVeto();
			return pc;
		} else {
			dispatch_table = bucket_K[ ASSOC_OFFSET_NEXT ];
		}
	}	
	
	// 	Failed to find a matching method, so insert bucket. Garbage
	//	collection must be inhibited in this section.
	
	XfrClass xfr( vm->heap().preflight( pc, ASSOC_SIZE ) );
	xfr.setOrigin();
	xfr.xfrRef( sysAssocKey );
	xfr.xfrRef( gclass );
	xfr.xfrRef( function );
	xfr.xfrRef( INDEX( method, METHOD_OFFSET_DISPATCH_TABLE ) );
	INDEX( method, METHOD_OFFSET_DISPATCH_TABLE ) = xfr.makeRef();
		
	vm->gcLiftVeto();
	return pc;
}

Ref * sysSetMethod( Ref * pc, MachineClass * vm ) {
	return gngSetMethod( pc, vm, false );
}

Ref * sysOverrideMethod( Ref * pc, MachineClass * vm ) {
	return gngSetMethod( pc, vm, true );
}

