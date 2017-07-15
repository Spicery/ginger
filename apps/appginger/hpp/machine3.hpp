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

#ifndef MACHINE3_EXCLUDED
#ifndef MACHINE3_HPP
#define MACHINE3_HPP

#include "machine.hpp"
#include "callstacklayout.hpp"
#include "listlayout.hpp"
#include "instruction_set3.hpp"
#include "appcontext.hpp"

namespace Ginger {

class Machine3 : public MachineClass {

private:
	InstructionSet3 instruction_set;
	
private:
	void core( bool init_mode, Ref * pc );
	
public:
	virtual void execute( Ref r, const bool clear_stack = true );
	const InstructionSet & instructionSet() {
		return this->instruction_set;
	}

public:
	Machine3( AppContext * application );
	virtual ~Machine3() {}
};

}

#endif
#endif
