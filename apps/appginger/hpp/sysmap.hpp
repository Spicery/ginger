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

namespace Ginger {

extern Ref * sysNewHardEqMap( Ref * pc, MachineClass * vm );
extern Ref * sysNewHardIdMap( Ref * pc, MachineClass * vm );
extern Ref * sysNewWeakIdMap( Ref * pc, MachineClass * vm );
extern Ref * sysNewCacheEqMap( Ref * pc, MachineClass * vm );
extern Ref * sysMapExplode( Ref *pc, class MachineClass * vm );
extern Ref * sysMapValues( Ref *pc, class MachineClass * vm );
extern Ref * sysMapIndex( Ref *pc, class MachineClass * vm );

extern void gngPrintMapPtr( std::ostream & out, Ref * r );
extern void gngRehashMapPtr( Ref * map_K );
extern unsigned int gngIdHash( Ref ref );
extern unsigned int gngIdHash( Ref ref );


/// Moving this utility class into the Ginger namespace is just
/// part of a highly extended refactoring to move all the Ginger
/// implementation into its own namespaces.
class MapCrawl {
private:
    long size_of_data;
    int index_of_data;
    Ref * data;
    Ref bucket;

public:
    MapCrawl( Ref * map_K );

public:
    Ref * nextBucket();
    bool hasBeenCalled() const;

};

} // namespace Ginger


#endif
