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

#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "mishap.hpp"
#include "sysunix.hpp"
#include "sys.hpp"

namespace Ginger {
using namespace std;

unsigned long sizeNeededForStrLen( size_t strlen_n ) {
	return 2 + ( strlen_n + sizeof( long ) - 1 + 1 ) / sizeof( long );
}

/*Ref * pushCString( Ref * pc, class MachineClass * vm, char * cstring ) {
	int result_n = strlen( cstring );
	XfrClass xfr( vm->heap().preflight( pc, sizeNeededForStrLen( result_n ) ) );
	xfr.xfrRef( LongToSmall( result_n ) );
	xfr.setOrigin();
	xfr.xfrRef( sysStringKey );
	xfr.xfrSubstringStep( 0, cstring, 0, result_n );
	xfr.xfrSubstringFinish( result_n + 1 );
	vm->fastPush( xfr.makeRef() );
	return pc;
}*/


//	Arity: 1 -> 1
Ref * sysGetEnv( Ref * pc, class MachineClass * vm ) {
	if ( vm->count != 1 ) throw Ginger::Mishap( "ArgsMismatch" );
	
	Ref r = vm->fastPeek();
	if ( !IsStringKind( r ) ) throw Ginger::Mishap( "Non-string argument needed for getEnv" );	
	Ref * str_K = RefToPtr4( r );
	
	char * data = reinterpret_cast< char * >( str_K + 1 );
	char * result = getenv( data );
	
	if ( result == NULL ) {
		vm->fastPeek() = SYS_ABSENT;
	} else {
		vm->fastPeek() = vm->heap().copyString( pc, result );
	}
	
	return pc;
}
SysInfo infoSysGetEnv( 
    FullName( "sysGetEnv" ), 
    Ginger::Arity( 1 ), 
    Ginger::Arity( 1 ), 
    sysGetEnv, 
    "Returns the value of an environment variable."
);

//	Arity: 2 -> 0
Ref * sysPutEnv( Ref * pc, class MachineClass * vm ) {
	if ( vm->count != 2 ) throw Ginger::Mishap( "ArgsMismatch" );
	Ref rhs = vm->fastPop();
	Ref lhs = vm->fastPop();
	if ( !IsStringKind( rhs ) ) throw Ginger::Mishap( "Non-string argument needed for putEnv" );	
	if ( !IsStringKind( lhs ) ) throw Ginger::Mishap( "Non-string argument needed for putEnv" );	
	
	Ref * lhs_K = RefToPtr4( lhs );
	Ref * rhs_K = RefToPtr4( rhs );
	
	char * name = reinterpret_cast< char * >( lhs_K + 1 );
	char * value = reinterpret_cast< char * >( rhs_K + 1 );
	
	setenv( name, value, 1 );

	return pc;
}
SysInfo infoSysPutEnv( 
    FullName( "sysPutEnv" ), 
    Ginger::Arity( 2 ), 
    Ginger::Arity( 0 ), 
    sysPutEnv, 
    "Sets the value of an environment variable."
);

//	Arity: 0 -> 1
Ref * sysEnvVars( Ref * pc, class MachineClass * vm ) {
	if ( vm->count != 0 ) throw Ginger::Mishap( "ArgsMismatch" );
	AppContext & cxt = vm->getAppContext();
	char ** envp = cxt.getEnvironmentVariables();
	
	int count = 0;
	for ( char ** env = envp; *env != 0; env++ ) {
		count += 1;
	}
	
	vm->checkStackRoom( count );
	
	for ( char ** env = envp; *env != 0; env++ ) {
		vm->fastPush( vm->heap().copyString( pc, *env ) );
	}
	
	return pc;
}
SysInfo infoSysEnvVars( 
    FullName( "sysEnv" ), 
    Ginger::Arity( 0 ), 
    Ginger::Arity( 1 ), 
    sysEnvVars, 
    "Returns the environment list."
);

} // namespace Ginger
