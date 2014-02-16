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

#ifndef GINGER_SYS_MATHS_HPP
#define GINGER_SYS_MATHS_HPP

#include "common.hpp"

namespace Ginger {

extern bool canFitInSmall( const long n );
extern Ref * sysAdd( Ref * pc, class MachineClass * vm );
extern Ref * sysAddHelper( Ref * pc, class MachineClass * vm, Ref ry );
extern Ref * sysSubHelper( Ref * pc, class MachineClass * vm, Ref ry );
extern Ref * sysMulHelper( Ref * pc, class MachineClass * vm, Ref ry );
extern Ref * sysNegHelper( Ref * pc, class MachineClass * vm );
extern Ref * sysDivHelper( Ref * pc, class MachineClass * vm, Ref ry );
extern Ref * sysFlooredQuotientHelper( Ref * pc, class MachineClass * vm, Ref ry );
extern Ref * sysFlooredRemainderHelper( Ref * pc, class MachineClass * vm, Ref ry );
extern bool sysCompareNumbers( Ref x, Ref y, const bool lt, const bool eq, const bool gt );

} // namespace

#endif