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

#include <string>
#include <iostream>

#include "mishap.hpp"
#include "machine.hpp"
#include "sysstack.hpp"
#include "sysprint.hpp"

#include "sys.hpp"
#include "key.hpp"
#include "cell.hpp"
#include "heap.hpp"
#include "externalkind.hpp"
#include "inputstreamexternal.hpp"

//namespace Ginger {

using namespace Ginger;


Ref * sysNewInputStream( Ref * pc, MachineClass * vm ) {
    if ( vm->count != 1 ) {
        throw Mishap( "Wrong number of arguments supplied" );
    }

    XfrClass xfr( pc, *vm, EXTERNAL_KIND_SIZE );
    
    Cell fname( vm->fastPeek() );
    // Type checks on the fly.
    std::string filename = fname.asHeapObject().asStringObject().getString();
    /// @todo potential store leak (in event of exception)
    InputStreamExternal * e = new InputStreamExternal( filename );
    //std::cerr << "Input Stream: " << std::hex << long( e ) << ", " << e << std::dec << std::endl;
    if ( e->isGood() ) {
        xfr.setOrigin();
        xfr.xfrRef( sysInputStreamKey );
        xfr.xfrCopy( e );
        Ref * r = xfr.makeRefRef();
        vm->trackExternalObject( r );
        vm->fastPeek() = Ptr4ToRef( r );
    } else {
        delete e;
        throw Mishap( "Could not open file" ).culprit( "File", filename );    
    }
    return pc;
}
SysInfo infoNewInputStream( 
    SysNames( "newInputStream" ), 
    Arity( 1 ), 
    Arity( 1 ), 
    sysNewInputStream, 
    "Builds an input stream from a file name" 
);


Ref * sysNewOutputStream( Ref * pc, MachineClass * vm ) {
    /// @todo 
    std::cerr << "TO BE DONE" << std::endl;
    return pc;
}
SysInfo infoNewOutputStream( 
    SysNames( "newOutputStream" ), 
    Arity( 1 ), 
    Arity( 1 ), 
    sysNewOutputStream, 
    "Builds an output stream from a file name" 
);


//} // namespace Ginger