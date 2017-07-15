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

#ifndef ROOTS_HPP
#define ROOTS_HPP

#include "common.hpp"
#include "machine.hpp"
#include "registers.hpp"

namespace Ginger {

class DynamicRoots {
private:
	MachineClass * vm;
	size_t saved_length;
public:
	DynamicRoots( MachineClass * vm );
	~DynamicRoots();
public:
	Cell & nextRoot() { return this->nextRoot( SYS_ABSENT ); }
	Cell & nextRoot( const Cell initial_value );
	Cell & nextRoot( const Ref initial_value ) { return this->nextRoot( Cell( initial_value ) ); }
};

} // namespace Ginger

#endif
