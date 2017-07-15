/******************************************************************************\
    Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
    of the GNU General Public License. This file is part of Ginger.

    Ginger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Ginger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Ginger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

#include <sstream>

#include "cell.hpp"
#include "sysexception.hpp"
#include "exceptionlayout.hpp"
#include "mishap.hpp"
#include "sysprint.hpp"

namespace Ginger {
using namespace std;

#ifdef EXCEPTIONS_IMPLEMENTED
	/*
		Constructs an exception object from a name, with no positional
		or keyword arguments.

		TODO: For version 0.8 at least I don't intend to bother with
		constructing exceptions.
	*/
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
	SysInfo infoNewException( 
	    SysNames( "newException" ), 
	    Ginger::Arity( 1 ), 
	    Ginger::Arity( 1 ), 
	    sysNewException, 
	    "Makes an exception."
	);
#endif

static void decorateProblem( class MachineClass * vm, Ginger::Mishap & problem ) {
	if ( vm->count == 2 ) {
		//cerr << "Stacksize = " << vm->stackLength() << endl;
		Cell event_name = vm->fastPeek();
		Cell args = vm->fastPeek( 1 );
		//cerr << "Event: " << event_name.toShowString() << endl;
		//cerr << "Args : " << args.toShowString() << endl;
		problem.setMessage( event_name.toPrintString() );
		if ( args.isVectorObject() ) {
			for ( VectorObject::generator g( args.asHeapObject().asVectorObject() ); !!g; ++g ) {
				Cell x = *g;
				problem.culprit( "Argument", x.toShowString() );
			}
		}
	}
}

/*
	A failover is raised at a point when the machine state
	still has integrity. We can therefore perform quite a few
	operations safely on the state, such as sys-printing.
*/
Ref * sysFailover( Ref * pc, class MachineClass * vm ) {
	//	Initialise message with default.
	Ginger::Mishap mishap( "Internal error: throw called with invalid arguments" ); 
	decorateProblem( vm, mishap );
	throw mishap;
}

/*
	A panic is raised at a point when the machine state
	still has integrity. We can therefore perform quite a few
	operations safely on the state, such as sys-printing.
*/
Ref * sysPanic( Ref * pc, class MachineClass * vm ) {
	//	Initialise message with default.
	Ginger::Mishap syserror( "Internal error: throw called with invalid arguments", "S" ); 
	decorateProblem( vm, syserror );
	throw syserror;
}

} // namespace Ginger
