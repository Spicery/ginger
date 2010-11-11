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

#ifndef RCEP_HPP
#define RCEP_HPP

#include <istream>

#include "machine.hpp"
#include "package.hpp"

class RCEP {
private:
	Package * current_package;

public:
	MachineClass * getMachine() { return this->current_package->getMachine(); } 
	
	//	Does not trap mishap.
	bool unsafe_read_comp_exec_print( std::istream & input, std::ostream & output );
	
	//	Traps mishap.
	bool read_comp_exec_print( std::istream & input, std::ostream & output );

public:
	RCEP( Package * current_package ) :
		current_package( current_package )
	{
	}
};

#endif

