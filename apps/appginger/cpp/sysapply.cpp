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
#include "key.hpp"


#include "sysapply.hpp"



Ref * sysApply( Ref * pc, MachineClass *vm ) {
    /*
    if ( vm->count != 1 ) throw Ginger::Mishap( "Wrong number of arguments" );
    Ref key = vm->fastPeek();
    if ( not IsKey( key ) ) throw Ginger::Mishap( "Class of object needed" );
    Ref * key_K = RefToPtr4( key );
    */
    return pc;
}

