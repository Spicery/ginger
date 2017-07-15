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

#include <ctype.h>

#include "machine.hpp"
#include "refprint.hpp"
#include "sysprint.hpp"
#include "sys.hpp"

namespace Ginger {

Ref * sysIsLowerCase( Ref * pc, class MachineClass * vm ) {
    if ( vm->count != 1 ) throw Ginger::Mishap( "Wrong number of arguments" );
    
    Ref r = vm->fastPeek();
    if ( IsCharacter( r ) ) {
        vm->fastPeek() = islower( CharacterToChar( r ) ) ? SYS_TRUE : SYS_FALSE;
    } else if ( IsString( r ) ) {
        Ref * str_K = RefToPtr4( r );
        char * s = reinterpret_cast< char * >( &str_K[ 1 ] );
        vm->fastPeek() = SYS_TRUE;
        while ( *s != 0 ) {
            if ( not islower( *s++ ) ) {
                vm->fastPeek() = SYS_FALSE;
                break;
            }
        }
    } else {
        throw Ginger::Mishap( "Non-character argument" ).culprit( "Argument", refToShowString( r ) );  
    } 
    
    return pc;
}
SysInfo infoIsLowerCase( 
    FullName( "isLowerCase" ), 
    Arity( 1 ), 
    Arity( 1 ), 
    sysIsLowerCase, 
    "Returns true for a lower case character or string, otherwise false" 
);

Ref * sysIsUpperCase( Ref * pc, class MachineClass * vm ) {
    if ( vm->count != 1 ) throw Ginger::Mishap( "Wrong number of arguments" );
    
    Ref r = vm->fastPeek();
    if ( IsCharacter( r ) ) {
        vm->fastPeek() = isupper( CharacterToChar( r ) ) ? SYS_TRUE : SYS_FALSE;
    } else if ( IsString( r ) ) {
        Ref * str_K = RefToPtr4( r );
        char * s = reinterpret_cast< char * >( &str_K[ 1 ] );
        vm->fastPeek() = SYS_TRUE;
        while ( *s != 0 ) {
            if ( not isupper( *s++ ) ) {
                vm->fastPeek() = SYS_FALSE;
                break;
            }
        }
    } else {
        throw Ginger::Mishap( "Non-character argument" ).culprit( "Argument", refToShowString( r ) );  
    }     
    return pc;
}
SysInfo infoIsUpperCase( 
    FullName( "isUpperCase" ), 
    Arity( 1 ), 
    Arity( 1 ), 
    sysIsUpperCase, 
    "Returns true for a upper case character or string, otherwise false" 
);

}
