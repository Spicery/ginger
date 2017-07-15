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
#include <string>
#include <sstream>

#include "common.hpp"
#include "machine.hpp"
#include "cage.hpp"
#include "sys.hpp"
#include "rcep.hpp"


#include "toolmain.hpp" // We need Toolmain::loadFileFromPackage.

#include "sysvm.hpp"

namespace Ginger {
using namespace std;


Ref * sysVirtualMachineLoadFile( Ref * pc, class MachineClass * vm ) {
    if ( vm->count != 2 ) {
        throw Mishap( "Wrong number of arguments supplied" );
    }
    Cell putative_vm_obj( vm->fastPeek( 0 ) );
    VirtualMachineObject vm_obj = putative_vm_obj.asHeapObject().asVirtualMachineObject();
    class MachineClass * vm_target = vm_obj.getExternal();
    string filename = StringObject( vm->fastPeek( 1 ) ).getString();
    RCEP recp( vm_target->getInteractivePackage(), false );

    //  We pop at the end rather than anywhere else so that we
    //  retain references to the VM.
    vm->fastDrop( 2 );

    return pc;

}
SysInfo infoVirtualMachineLoadFile( 
    FullName( "virtualMachineLoadFile" ), 
    Ginger::Arity( 2 ), 
    Ginger::Arity( 0 ), 
    sysVirtualMachineLoadFile, 
    "Loads a file into a virtual machine."
);



Ref * sysVMResults( Ref * pc, class MachineClass * vm ) {
    if ( vm->count != 1 ) {
        throw Mishap( "Wrong number of arguments supplied" );
    }
    Cell putative_vm_obj( vm->fastPop() );
    VirtualMachineObject vm_obj = putative_vm_obj.asHeapObject().asVirtualMachineObject();
    class MachineClass * vm_target = vm_obj.getExternal();
    vm_target->printResults( 0.0 );
    return pc;
}
SysInfo infoVMResults( 
    FullName( "virtualMachineResults" ), 
    Ginger::Arity( 1 ), 
    Ginger::Arity( 0 ), 
    sysVMResults, 
    "Given a virtualmachine, prints the results to the console."
);


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
    FullName( "newVirtualMachine" ), 
    Ginger::Arity( 0 ), 
    Ginger::Arity( 1 ), 
    sysNewVM, 
    "Returns a newly created virtual machine."
);

} // namespace
