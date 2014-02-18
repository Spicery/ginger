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

#include "common.hpp"
#include "machine.hpp"
#include "cage.hpp"
#include "sys.hpp"

#include "sysvm.hpp"

namespace Ginger {

Ref * sysNewVM( Ref * pc, class MachineClass * vm ) {
    if ( vm->count != 0 ) throw Ginger::Mishap( "ArgsMismatch" );
    XfrClass xfr( pc, vm->heap(), EXTERNAL_KIND_SIZE );
    xfr.setOrigin();
    xfr.xfrRef( sysVirtualMachineKey );
    xfr.xfrCopy( vm->getAppContext().newMachine() );
    vm->checkStackRoom( 1 );
    vm->fastPush( xfr.makeRef() );
    return pc;
}
SysInfo infoNewVM( 
    SysNames( "newVirtualMachine" ), 
    Ginger::Arity( 0 ), 
    Ginger::Arity( 1 ), 
    sysNewVM, 
    "Returns a newly created virtual machine."
);

} // namespace