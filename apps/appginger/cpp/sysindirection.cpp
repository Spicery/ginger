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

#include "debug.hpp"

#include <iostream>
using namespace std;

#include "sysindirection.hpp"
#include "indirectionlayout.hpp"
#include "key.hpp"
#include "cage.hpp"
#include "machine.hpp"
#include "mishap.hpp"


/**
	sysMakeIndirection will be called with pc[-1] storing the
	slot number of the variable to be updated. It is inherently
	fast and dangerous!
*/
Ref * sysMakeIndirection( Ref * pc, class MachineClass * vm ) {
	XfrClass xfr( vm->heap().preflight( pc, INDIRECTION_SIZE ) );
	xfr.setOrigin();
	xfr.xfrRef( sysIndirectionKey );
	xfr.xfrRef( SYS_ABSENT );
	Ref r = xfr.makeRef();
	vm->sp[ ToULong( pc[-1] ) ] = r;
	#ifdef DBG_LIFTING
		cerr << "Made indirection " << r << endl;
	#endif
	return pc;
}

/** fastIndirectionCont returns a reference to the only field of
	an Indirection record in the heap. Fast and dangerous!
*/
Ref & fastIndirectionCont( Ref indirection ) {
	return RefToPtr4( indirection )[ INDIRECTION_CONT_OFFSET ];
}

/**
	sysCopyIndirection will be called with pc[-1] storing the
	slot number of the variable to be updated. It is inherently
	fast and dangerous!
*/
Ref * sysCopyIndirection( Ref * pc, class MachineClass * vm ) {
	Ref & var = vm->vp[ ToULong( pc[-1] ) ];
	XfrClass xfr( vm->heap().preflight( pc, INDIRECTION_SIZE ) );
	xfr.setOrigin();
	xfr.xfrRef( sysIndirectionKey );
	xfr.xfrRef( fastIndirectionCont( var ) );
	var = xfr.makeRef();
	return pc;
}

/**
	sysPushIndirection will be called with pc[-1] storing the
	slot number of the variable to be pushed. It is inherently
	fast and dangerous!
*/
Ref * sysPushIndirection( Ref * pc, class MachineClass * vm ) {
	Ref var = vm->sp[ ToULong( pc[-1] ) ];
	vm->fastPush( fastIndirectionCont( var ) );
	return pc;
}

/**
	sysPopIndirection will be called with pc[-1] storing the
	slot number of the variable to be popped. It is inherently
	fast and dangerous!
*/
Ref * sysPopIndirection( Ref * pc, class MachineClass * vm ) {
	Ref var = vm->sp[ ToULong( pc[-1] ) ];
	fastIndirectionCont( var ) = vm->fastPop();
	return pc;
}
