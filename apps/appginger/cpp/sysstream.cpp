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
#include "inputstreamlayout.hpp"
#include "inputstreamexternal.hpp"
#include "outputstreamexternal.hpp"

namespace Ginger {
using namespace Ginger;

static Ref * getInputStreamKeyPtr( MachineClass * vm, int n ) {
    if ( vm->count <= n ) throw Mishap( "Too few arguments supplied" );
    Ref input_stream = vm->fastPeek( n );
    if ( not IsObj( input_stream ) ) throw Ginger::Mishap( "Invalid argument instead of InputStream" ).culprit( "Argument", refToShowString( input_stream ) );
    Ref * input_stream_K = RefToPtr4( input_stream );
    if ( input_stream_K[0] != sysInputStreamKey ) {
        throw Ginger::Mishap( "Argument not an InputStream" ).culprit( "Argument", refToShowString( input_stream ) );
    }    
    return input_stream_K;
}

Ref * sysCloseInputStream( Ref * pc, MachineClass * vm ) {
    Ref * input_stream_K = getInputStreamKeyPtr( vm, 0 );
    input_stream_K[ PUSHBACK_OFFSET_INPUT_STREAM ] = SYS_NIL;
    reinterpret_cast< Ginger::InputStreamExternal * >( input_stream_K[ EXTERNAL_KIND_OFFSET_VALUE ] )->close();
    return pc;
}
SysInfo infoCloseInputStream( 
    FullName( "closeInputStream" ), 
    Arity( 1 ), 
    Arity( 0 ), 
    sysCloseInputStream, 
    "Closes an input stream"
);

Ref * sysNextLineInputStream( Ref * pc, MachineClass * vm ) {
    Ref * input_stream_K = getInputStreamKeyPtr( vm, 0 );
    Ref pushed = input_stream_K[ PUSHBACK_OFFSET_INPUT_STREAM ];
    if ( pushed == SYS_NIL ) {
        // Safe to assign to top, replacing the ref to the input stream.
        std::string line;
        InputStreamExternal * e = reinterpret_cast< Ginger::InputStreamExternal * >( input_stream_K[ EXTERNAL_KIND_OFFSET_VALUE ] );
        if ( e->getline( line ) ) {
            vm->fastPeek() = vm->heap().copyString( pc, line.c_str() );
        } else {
            vm->fastPeek() = SYS_TERMIN;
        }
    } else {
        Ref * pair_K = RefToPtr4( pushed );
        Ref item = pair_K[ PAIR_HEAD_OFFSET ];
        vm->fastPeek() = item;
        input_stream_K[ PUSHBACK_OFFSET_INPUT_STREAM ] = pair_K[ PAIR_TAIL_OFFSET ];
    }
    return pc;
}
SysInfo infoNextLineInputStream( 
    FullName( "nextLineInputStream" ), 
    Arity( 1 ), 
    Arity( 1 ), 
    sysNextLineInputStream, 
    "Gets the next line from an input stream"
);

Ref * sysPeekLineInputStream( Ref * pc, MachineClass * vm ) {
    Ref * input_stream_K = getInputStreamKeyPtr( vm, 0 );
    Ref pushed = input_stream_K[ PUSHBACK_OFFSET_INPUT_STREAM ];
    if ( pushed == SYS_NIL ) {
        std::string line;
        const bool ok = reinterpret_cast< Ginger::InputStreamExternal * >( input_stream_K[ EXTERNAL_KIND_OFFSET_VALUE ] )->getline( line );
        if ( ok ) {
            vm->fastPeek() = vm->heap().copyString( pc, line.c_str() );  

            //  INVALIDATES input_stream_K! Must recalculate.
            Ref * input_stream_K = getInputStreamKeyPtr( vm, 0 );
            Ref pushed = input_stream_K[ PUSHBACK_OFFSET_INPUT_STREAM ];

            XfrClass xfr( pc, *vm, LengthOfSimpleKey( sysPairKey ) );
            xfr.setOrigin();
            xfr.xfrRef( sysPairKey );
            xfr.xfrRef( vm->fastPeek() );   //  New item on top of stack.
            xfr.xfrRef( pushed );
            input_stream_K[ PUSHBACK_OFFSET_INPUT_STREAM ] = xfr.makeRef();
        } else {
            vm->fastPeek() = SYS_TERMIN;
            return pc;
        }
    } else {
        vm->fastPeek() = RefToPtr4( pushed )[ PAIR_HEAD_OFFSET ];
    } 
    return pc;
}
SysInfo infoPeekLineInputStream(
    FullName( "peekLineInputStream" ), 
    Arity( 1 ), 
    Arity( 1 ), 
    sysPeekLineInputStream, 
    "Peeks at the next line from an input stream without removing it from the stream"
);



static Ref * isOpenInputStream( Ref * pc, MachineClass * vm, Ref trueval, Ref falseval ) {
    Ref * input_stream_K = getInputStreamKeyPtr( vm, 0 );
    Ref pushed = input_stream_K[ PUSHBACK_OFFSET_INPUT_STREAM ];
    if ( pushed == SYS_NIL ) {
        //  No pushback, so simply return the status of the output stream
        vm->fastPeek() = reinterpret_cast< Ginger::InputStreamExternal * >( input_stream_K[ EXTERNAL_KIND_OFFSET_VALUE ] )->isGood() ? trueval : falseval;
    } else {
        //  There is pushback, so return -false- if the head of the pushback
        //  list is -termin-, else -true-.
        vm->fastPeek() = RefToPtr4( pushed )[ PAIR_HEAD_OFFSET ] == SYS_TERMIN ? falseval : trueval;
    }
    return pc;
}

Ref * sysIsOpenInputStream( Ref * pc, MachineClass * vm ) {
    return isOpenInputStream( pc, vm, SYS_TRUE, SYS_FALSE );
}
SysInfo infoIsOpenInputStream( 
    FullName( "isOpenInputStream" ), 
    Arity( 1 ), 
    Arity( 1 ), 
    sysIsOpenInputStream, 
    "Returns true if the input stream is open (the next value to be returned will not be termin)"
);

Ref * sysIsClosedInputStream( Ref * pc, MachineClass * vm ) {
    return isOpenInputStream( pc, vm, SYS_FALSE, SYS_TRUE );
}
SysInfo infoIsClosedInputStream( 
    FullName( "isClosedInputStream" ), 
    Arity( 1 ), 
    Arity( 1 ), 
    sysIsClosedInputStream, 
    "Returns true if the input stream is closed (the next value to be returned will be termin)"
);


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
        xfr.xfrRef( SYS_NIL );      //  The pushback list, initially empty.
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
    FullName( "newInputStream" ), 
    Arity( 1 ), 
    Arity( 1 ), 
    sysNewInputStream, 
    "Builds an input stream from a file name" 
);


Ref * sysNextInputStream( Ref * pc, MachineClass * vm ) {
    Ref * input_stream_K = getInputStreamKeyPtr( vm, 0 );
    return reinterpret_cast< Ginger::InputStreamExternal * >( input_stream_K[ EXTERNAL_KIND_OFFSET_VALUE ] )->sysApply( pc, vm );
}
SysInfo infoNextInputStream(
    FullName( "nextInputStream" ), 
    Arity( 1 ), 
    Arity( 1 ), 
    sysNextInputStream, 
    "Returns the next item from the input stream"
);

Ref * sysPeekInputStream( Ref * pc, MachineClass * vm ) {
    Ref * input_stream_K = getInputStreamKeyPtr( vm, 0 );
    Ref pushed = input_stream_K[ PUSHBACK_OFFSET_INPUT_STREAM ];
    if ( pushed == SYS_NIL ) {
        XfrClass xfr( pc, *vm, LengthOfSimpleKey( sysPairKey ) );
        pc = reinterpret_cast< Ginger::InputStreamExternal * >( input_stream_K[ EXTERNAL_KIND_OFFSET_VALUE ] )->sysApply( pc, vm );
        xfr.setOrigin();
        xfr.xfrRef( sysPairKey );
        xfr.xfrRef( vm->fastPeek() );   //  New item on top of stack.
        xfr.xfrRef( pushed );
        input_stream_K[ PUSHBACK_OFFSET_INPUT_STREAM ] = xfr.makeRef();
    } else {
        vm->fastPeek() = RefToPtr4( pushed )[ PAIR_HEAD_OFFSET ];
    } 
    return pc;
}
SysInfo infoPeekInputStream(
    FullName( "peekInputStream" ), 
    Arity( 1 ), 
    Arity( 1 ), 
    sysPeekInputStream, 
    "Peeks at the next item from an input stream without removing it from the stream"
);

//  input_stream.pushInputStream( values )
Ref * sysPushbackInputStream( Ref * pc, MachineClass * vm ) {
    int vm_count_1 = vm->count - 1;
    Ref * input_stream_K = getInputStreamKeyPtr( vm, vm_count_1 );
    
    //  We will need to allocate vm->count - 1 pairs.
    XfrClass xfr( pc, *vm, vm_count_1 * LengthOfSimpleKey( sysPairKey ) );
    
    Ref pushed = input_stream_K[ PUSHBACK_OFFSET_INPUT_STREAM ];
    for ( int i = 0; i < vm_count_1; i++ ) {
        xfr.setOrigin();
        xfr.xfrRef( sysPairKey );
        xfr.xfrRef( vm->fastPeek( i ) );
        xfr.xfrRef( pushed );
        pushed = xfr.makeRef();
    }
    input_stream_K[ PUSHBACK_OFFSET_INPUT_STREAM ] = pushed;

    vm->fastDrop( vm->count );
    return pc;
}
SysInfo infoPushbackInputStream(
    FullName( "pushInputStream" ), 
    Arity( 1, true ), 
    Arity( 0 ), 
    sysPushbackInputStream, 
    "Pushes items back onto an input stream" 
);


Ref * sysNewOutputStream( Ref * pc, MachineClass * vm ) {
    if ( vm->count != 1 ) {
        throw Mishap( "Wrong number of arguments supplied" );
    }

    XfrClass xfr( pc, *vm, EXTERNAL_KIND_SIZE );
    
    Cell fname( vm->fastPeek() );
    // Type checks on the fly.
    std::string filename = fname.asHeapObject().asStringObject().getString();
    /// @todo potential store leak (in event of exception)
    OutputStreamExternal * e = new OutputStreamExternal( filename );
    //std::cerr << "Input Stream: " << std::hex << long( e ) << ", " << e << std::dec << std::endl;
    if ( e->isGood() ) {
        xfr.setOrigin();
        xfr.xfrRef( sysOutputStreamKey );
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
SysInfo infoNewOutputStream( 
    FullName( "newOutputStream" ), 
    Arity( 1 ), 
    Arity( 1 ), 
    sysNewOutputStream, 
    "Builds an output stream from a file name" 
);


Ref * getOutputStreamK( MachineClass * vm ) {
    if ( vm->count != 1 ) {
        throw Mishap( "Wrong number of arguments supplied" );
    }
    Ref object = vm->fastPeek();
    if ( not IsObj( object ) ) {
        throw Mishap( "OutputStream needed" ).culprit( "Value", refToShowString( object ) );
    }
    return RefToPtr4( object );   
}

Ref * isOpenOutputStream( Ref * pc, MachineClass * vm, Ref trueval, Ref falseval ) {
    Ref * object_K = getOutputStreamK( vm );
    OutputStreamExternal * e = reinterpret_cast< Ginger::OutputStreamExternal * >( object_K[ EXTERNAL_KIND_OFFSET_VALUE ] );
    vm->fastPeek() = e->isOpen() ? trueval : falseval;
    return pc;
}

Ref * sysIsOpenOutputStream( Ref * pc, MachineClass * vm ) {
    return isOpenOutputStream( pc, vm, SYS_TRUE, SYS_FALSE );
}
SysInfo infoIsOpenOutputStream(
    FullName( "isOpenOutputStream" ), 
    Arity( 1 ), 
    Arity( 1 ), 
    sysIsOpenOutputStream, 
    "Returns true if the output stream is open, else false" 
);

Ref * sysIsClosedOutputStream( Ref * pc, MachineClass * vm ) {
    return isOpenOutputStream( pc, vm, SYS_FALSE, SYS_TRUE );
}
SysInfo infoIsClosedOutputStream(
    FullName( "isClosedOutputStream" ), 
    Arity( 1 ), 
    Arity( 1 ), 
    sysIsClosedOutputStream, 
    "Returns true if the output stream is closed, else false" 
);

Ref * sysCloseOutputStream( Ref * pc, MachineClass * vm ) {
    Ref * object_K = getOutputStreamK( vm );
    OutputStreamExternal * e = reinterpret_cast< Ginger::OutputStreamExternal * >( object_K[ EXTERNAL_KIND_OFFSET_VALUE ] );
    e->close();
    vm->fastDrop( 1 );
    return pc;
}
SysInfo infoCloseOutputStream(
    FullName( "closeOutputStream" ), 
    Arity( 1 ), 
    Arity( 0 ), 
    sysCloseOutputStream, 
    "Closes an output stream" 
);


} // namespace Ginger
