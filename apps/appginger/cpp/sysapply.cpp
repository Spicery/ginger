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

#define DBG_SYS_APPLY

#ifdef DBG_SYS_APPLY
#include <iostream>
#endif

#include "mishap.hpp"


#include "common.hpp"
#include "key.hpp"
#include "sysprint.hpp"
#include "sysfunction.hpp"
#include "sysvector.hpp"
#include "syslist.hpp"
#include "sysstack.hpp"
#include "sysstring.hpp"
#include "sysmap.hpp"

//  TODO: I would REALLY like to get rid of this include. The basic problem
//  is that I have not decorated the auto generated header snippets with 
//  load-once guards.
#include "sys.hpp"

#include "sysapply.hpp"
#include "externalkind.hpp"

using namespace std;

static std::exception failApply( Ref object ) {
    throw Ginger::Mishap( "Trying to apply inappropriate value" ).culprit( "Value", refToString( object ) );
}

static Ref * gngApplySimpleKey( Ref simple_key, Ref * pc, MachineClass * vm ) {
    switch ( KindOfSimpleKey( simple_key ) ) {
        case VECTOR_KIND: {
            return sysNewVector( pc, vm );
        }
        /*
        case PAIR_KIND: {
            return sysNewList( pc, vm );
        }
        */
        case MAP_KIND: {
            //  TODO: We should really get exactly the right kind of map by
            //  examining the type of key.
            return sysNewHardEqMap( pc, vm );
        }
        /*
        case ATTR_KIND:
        case MIXED_KIND: {
            throw Ginger::Mishap( "Internal error (sysApply) - not implemented" );
            break;
        }
        */
        /*
        case RECORD_KIND: {
            throw Ginger::Mishap( "Internal error (sysApply) - not implemented" );
            break;
        }
        */
        /*
        case WRECORD_KIND: {
            if ( key == sysDoubleKey ) {
                throw Ginger::Mishap( "Internal error (sysApply) - not implemented" );
            } else {
                throw Ginger::Mishap( "Internal error (sysApply) - not implemented" );
            }
            break;
        }
        */
        case STRING_KIND: {
            return sysNewString( pc, vm );
        }
        default: {
            throw failApply( simple_key );
        }
    }
}

/*
    We are trying to apply a Class. This is typically an attempt to
    run the default constructor for the Class. 

    In general this means looks at the SELF_APPLY field of the class.
    If it is SYS_ABSENT then an error is raised.
*/
static Ref * gngApplyClass( Ref * class_K, Ref * pc, MachineClass * vm ) {
    //  TODO: Implement the self-apply field for classes.
    throw failApply( ToRef( class_K ) );
}


/*
    We are trying to apply an Object of a Class. This is the equivalent of
    Pop11's class_apply( datakey( object ) )( args, ..., object). It should
    be implemented by looking in the Classes APPLY_INSTANCE field (complaining)
    if it is blank.
*/
static Ref * gngApplyObject( Ref * object_K, Ref key, Ref * pc, MachineClass * vm ) {
    //  TODO: Implement the class apply
    throw failApply( ToRef( object_K ) );
}


Ref * sysApply( Ref * pc, MachineClass *vm ) {
    if ( vm->count < 1 ) throw Ginger::Mishap( "Wrong number of arguments" );
    Ref object = vm->fastPeek();
    if ( not IsObj( object ) ) {
        if ( IsSimpleKey( object ) ) {
            vm->count -= 1;
            vm->vp -= 1;
            return gngApplySimpleKey( object, pc, vm );
        } else {
            throw failApply( object );
        }
    } else {
        Ref * object_K = RefToPtr4( object );
        Ref key = object_K[ 0 ];

        if ( IsFunctionKey( key ) ) {
            return sysApplyFunction( pc, vm );
        } else if ( IsSimpleKey( key ) ) {
            switch ( KindOfSimpleKey( key ) ) {
                case VECTOR_KIND: {
                    return sysGetIndexVector( sysFastSwap( pc, vm ), vm );
                }
                case PAIR_KIND: {
                    return sysListIndex( sysFastSwap( pc, vm ), vm );
                }
                case MAP_KIND: {
                    throw Ginger::Mishap( "Internal error (sysApply) - not implemented" );
                    break;
                }
                /*
                case ATTR_KIND:
                case MIXED_KIND: {
                    throw Ginger::Mishap( "Internal error (sysApply) - not implemented" );
                    break;
                }
                */
                case RECORD_KIND: {
                    if ( key == sysClassKey ) {
                        return gngApplyClass( object_K, pc, vm );
                    } else {
                        throw Ginger::Mishap( "Internal error (sysApply) - not implemented" );
                    }
                    break;
                }
                /*
                case WRECORD_KIND: {
                    if ( key == sysDoubleKey ) {
                        throw Ginger::Mishap( "Internal error (sysApply) - not implemented" );
                    } else {
                        throw Ginger::Mishap( "Internal error (sysApply) - not implemented" );
                    }
                    break;
                }
                */
                case STRING_KIND: {
                    return sysStringIndex( sysFastSwap( pc, vm ), vm );
                }
                case EXTERNAL_KIND: {
                    return reinterpret_cast< Ginger::External * >( object_K[ EXTERNAL_KIND_OFFSET_VALUE ] )->sysApply( pc, vm );
                }
                default: {
                    throw Ginger::Mishap( "Trying to apply non-applicable obect" ).culprit( "Object", refToString( object ) );
                }
            }
        } else if ( IsObj( key ) ) {
            return gngApplyObject( object_K, RefToPtr4( key ), pc, vm );
        } else {
            throw failApply( object );
        }

        return pc;
    }
}

