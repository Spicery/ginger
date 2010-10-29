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

#ifndef SYS_MAP_HPP
#define SYS_MAP_HPP

#include "common.hpp"
#include "machine.hpp"
#include <ostream>

extern Ref * sysNewHardEqMap( Ref * pc, MachineClass * vm );
extern Ref * sysNewHardIdMap( Ref * pc, MachineClass * vm );
extern Ref * sysNewWeakIdMap( Ref * pc, MachineClass * vm );
extern Ref * sysNewCacheEqMap( Ref * pc, MachineClass * vm );
extern Ref * sysMapExplode( Ref *pc, class MachineClass * vm );
extern Ref * sysMapIndex( Ref *pc, class MachineClass * vm );

extern void gngPrintMapPtr( std::ostream & out, Ref * r );
void gngRehashMapPtr( Ref * map_K );


#endif
