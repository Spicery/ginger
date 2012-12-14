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

#include "mishap.hpp"


#include "common.hpp"
#include "key.hpp"
#include "sysprint.hpp"
#include "sysfunction.hpp"


#include "sysapply.hpp"



Ref * sysApply( Ref * pc, MachineClass *vm ) {
    if ( vm->count < 1 ) throw Ginger::Mishap( "Wrong number of arguments" );
    Ref object = vm->fastPeek();
    if ( not IsObj( object ) ) throw Ginger::Mishap( "Trying to apply inappropriate value" ).culprit( "Value", refToString( object ) );
    Ref * object_K = RefToPtr4( object );
    Ref key = object_K[ 0 ];

    if ( IsFunctionKey( key ) ) {
        return sysApplyFunction( pc, vm );
        throw Ginger::Mishap( "Internal error (sysApply) - not implemented" );
    } else if ( key == sysMapletKey ) {
        throw Ginger::Mishap( "Internal error (sysApply) - not implemented" );
    } else if ( IsSimpleKey( key ) ) {
        switch ( KindOfSimpleKey( key ) ) {
            case VECTOR_KIND: {
                throw Ginger::Mishap( "Internal error (sysApply) - not implemented" );
                break;
            }
            case ATTR_KIND:
            case MIXED_KIND: {
                throw Ginger::Mishap( "Internal error (sysApply) - not implemented" );
                break;
            }
            case PAIR_KIND: {
                throw Ginger::Mishap( "Internal error (sysApply) - not implemented" );
                break;
            }
            case MAP_KIND: {
                throw Ginger::Mishap( "Internal error (sysApply) - not implemented" );
                break;
            }
            case RECORD_KIND: {
                throw Ginger::Mishap( "Internal error (sysApply) - not implemented" );
                break;
            }
            case WRECORD_KIND: {
                if ( key == sysDoubleKey ) {
                    throw Ginger::Mishap( "Internal error (sysApply) - not implemented" );
                } else {
                    throw Ginger::Mishap( "Internal error (sysApply) - not implemented" );
                }
                break;
            }
            case STRING_KIND: {
                throw Ginger::Mishap( "Internal error (sysApply) - not implemented" );
                break;
            }
            default: {
                throw Ginger::Mishap( "Internal error (sysApply) - not implemented" );
                break;
            }
        }
    } else if ( IsObj( key ) ) {
        throw Ginger::Mishap( "Internal error (sysApply) - not implemented" );
    } else {
        throw Ginger::Mishap( "Internal error (sysApply) - not implemented" );
    }

    return pc;
}

