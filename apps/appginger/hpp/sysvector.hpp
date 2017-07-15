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

#ifndef SYS_VECTOR_HPP
#define SYS_VECTOR_HPP

#include "common.hpp"
#include "machine.hpp"
#include "mishap.hpp"

namespace Ginger {

//  TODO: 
//  Really this should include the auto-generated headers and NOT
//  datatypes.hpp.auto which is in turn included by sys.hpp.
//    

#include "vector.hpp.auto"


extern Ref * sysVectorAppend( Ref *pc, class MachineClass * vm );
extern Ref * sysVectorExplode( Ref *pc, class MachineClass * vm );
extern Ref * sysIsUpdateableVector( Ref *pc, class MachineClass * vm );
extern Ref * sysVectorLength( Ref *pc, class MachineClass * vm );
extern Ref * sysFastVectorLength( Ref *pc, class MachineClass * vm );

} // namespace Ginger

#endif

