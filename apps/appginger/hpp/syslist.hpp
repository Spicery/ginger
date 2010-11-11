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

#ifndef SYS_LIST_HPP
#define SYS_LIST_HPP

#include "common.hpp"
#include "machine.hpp"
#include "mishap.hpp"

extern Ref * sysNewList( Ref * pc, MachineClass * vm );
extern Ref * sysNewListOnto( Ref * pc, MachineClass * vm );
extern Ref * sysIsNil( Ref * pc, class MachineClass * vm );
extern Ref * sysIsList( Ref * pc, class MachineClass * vm );
extern Ref * sysListAppend( Ref *pc, class MachineClass * vm );
extern Ref * sysListExplode( Ref *pc, class MachineClass * vm );
extern Ref * sysListIndex( Ref *pc, class MachineClass * vm );
extern Ref * sysListLength( Ref *pc, class MachineClass * vm );
extern Ref * sysFastListLength( Ref *pc, class MachineClass * vm );


#endif
