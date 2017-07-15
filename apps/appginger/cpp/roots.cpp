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

#include "common.hpp"
#include "roots.hpp"

namespace Ginger {

DynamicRoots::DynamicRoots( MachineClass * vm ) : 
    vm( vm ),
    saved_length( vm->spare_registers.size() )
{
}

DynamicRoots::~DynamicRoots() {
    this->vm->spare_registers.resize( this->saved_length );
}

Cell & DynamicRoots::nextRoot( const Cell initial_value ) {
    this->vm->spare_registers.push_back( initial_value );
    return this->vm->spare_registers.back();
}

} // namespace Ginger
