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

#ifndef ROOTS_HPP
#define ROOTS_HPP

#include "common.hpp"
#include "machine.hpp"
#include "registers.hpp"

namespace Ginger {

class Roots {
private:
	Registers & registers;
	unsigned long mask;
	int count;
	
public:
	Ref & reserveRegister( Ref r );
	Cell & reserveRootCell( Ref r = SYS_ABSENT ) { return reinterpret_cast< Cell & >( this->reserveRegister( r ) ); }

public:
	Roots( MachineClass * vm );
	~Roots();
};

} // namespace Ginger

#endif
