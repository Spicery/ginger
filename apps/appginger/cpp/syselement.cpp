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

#include "syselement.hpp"
#include "sysattrmap.hpp"
#include "mishap.hpp"

Ref * sysElementName( Ref *pc, class MachineClass * vm ) {
	if ( vm->count != 1 ) throw Ginger::Mishap( "ArgsMismatch" );
	Ref element = vm->fastPeek();
	if ( !IsElement( element ) ) throw Ginger::Mishap( "TypeError" );
	Ref * element_K = RefToPtr4( element );
	vm->fastPeek() = RefToPtr4( element_K[ 1 ] )[ 1 ];
	return pc;
}

Ref * sysElementAttribute( Ref *pc, class MachineClass * vm ) {
	if ( vm->count != 2 ) throw Ginger::Mishap( "ArgsMismatch" );
	Ref idx = vm->fastPop();
	Ref element = vm->fastPeek();
	if ( !IsElement( element ) ) throw Ginger::Mishap( "TypeError" );
	Ref * element_K = RefToPtr4( element );
	vm->fastPeek() = element_K[ 1 ];
	vm->fastPush( idx );
	return sysAttrMapIndex( pc, vm );
}
