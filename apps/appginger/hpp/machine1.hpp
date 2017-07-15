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

#ifndef MACHINE1_EXCLUDED

#ifndef MACHINE1_HPP
#define MACHINE1_HPP

#include "machine.hpp"
#include "callstacklayout.hpp"
#include "listlayout.hpp"
#include "instruction_set1.hpp"
#include "appcontext.hpp"

namespace Ginger {

class Machine1 : public MachineClass {

private:
	InstructionSet1 instruction_set;
	
public:
	virtual void execute( Ref r, const bool clear_stack = true );
	virtual const InstructionSet & instructionSet() {
		return this->instruction_set;
	}

public:
	Machine1( AppContext * g );
	virtual ~Machine1() {}

};

}

#endif
#endif
