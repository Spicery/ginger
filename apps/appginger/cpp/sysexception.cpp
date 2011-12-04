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

#include <sstream>

#include "sysexception.hpp"
#include "exceptionlayout.hpp"
#include "mishap.hpp"
#include "sysprint.hpp"

using namespace std;

Ref * sysNewException( Ref * pc, class MachineClass * vm ) {
	if ( vm->count == 1 ) {
		XfrClass xfr( vm->heap().preflight( pc, EXCEPTION_SIZE ) );
		xfr.setOrigin();
		xfr.xfrRef( sysExceptionKey );
		xfr.xfrRef( vm->fastPeek() );
		xfr.xfrRef( SYS_NIL );
		xfr.xfrRef( SYS_NIL );
		vm->fastPeek() = xfr.makeRef();
	} else {
		throw Ginger::Mishap( "Wrong number of arguments to construct an Exception" );
	}
	return pc;
}

/*
	A panic is raised at a point when the machine state
	still has integrity. We can therefore perform quite a few
	operations safely on the state, such as sys-printing.
*/
Ref * sysPanic( Ref * pc, class MachineClass * vm ) {
	if ( vm->count != 1 ) {
		throw Ginger::SystemError();
	} else {
		Ref exn = vm->fastPeek();
		if ( IsException( exn ) ) {
			stringstream out;
			refPrint( out, RefToPtr4( exn )[ EXCEPTION_OFFSET_TAG ] );
			throw Ginger::SystemError( out.str() );
		} else {
			throw Ginger::SystemError();
		}
	}
}
