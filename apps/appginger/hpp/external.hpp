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

#ifndef GNG_EXTERNAL_HPP
#define GNG_EXTERNAL_HPP

#include <ostream>
#include "common.hpp"

namespace Ginger {

class External {
public:
    External() {}
    virtual ~External() {}

public:
    virtual void print( std::ostream & out ) = 0;

public:
    virtual Ref * sysApply( Ref * pc, MachineClass * vm ) = 0;
};

} // namespace Ginger

#endif
