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

#ifndef SYS_INDIRECTION_HPP
#define SYS_INDIRECTION_HPP

#include "common.hpp"

namespace Ginger {

//	DEPRECATED
extern Ref * sysMakeIndirection( Ref * pc, class MachineClass * vm );
extern Ref * sysCopyIndirection( Ref * pc, class MachineClass * vm );
extern Ref * sysPushIndirection( Ref * pc, class MachineClass * vm );
extern Ref * sysPopIndirection( Ref * pc, class MachineClass * vm );

//	OK
extern Ref & fastIndirectionCont( Ref indirection );

//	NEW VERSIONS
Ref * sysSetIndirectionCont( Ref * pc, class MachineClass * vm );
Ref * sysIndirectionCont( Ref * pc, class MachineClass * vm );
Ref * sysNewIndirection( Ref * pc, class MachineClass * vm );

} // namespace Ginger

#endif
