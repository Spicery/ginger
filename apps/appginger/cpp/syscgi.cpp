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

#include "syscgi.hpp"

Ref * cgiValue( Ref * pc, class MachineClass * vm ) {
	if ( vm->count != 1 ) throw Ginger::Mishap( "ArgsMismatch" );
	
	Ref r = vm->fastPeek();
	if ( !IsStringKind( r ) ) throw Ginger::Mishap( "Non-string argument needed for getEnv" );	
	Ref * str_K = RefToPtr4( r );
	
	char * fieldname = reinterpret_cast< char * >( str_K + 1 );
	const char * value = vm->getAppContext().cgiValue( fieldname );
	vm->fastPeek() = vm->heap().copyString( pc, value );
	return pc;
}
