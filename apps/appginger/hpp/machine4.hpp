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

#ifndef MACHINE4_HPP
#define MACHINE4_HPP

#include "machine.hpp"
#include "instruction_set4.hpp"
#include "appcontext.hpp"

class Machine4 : public MachineClass {

private:
	InstructionSet4 instruction_set;
	
public:
	virtual void execute( Ref r );
	virtual const InstructionSet & instructionSet() {
		return this->instruction_set;
	}

public:
	Machine4( AppContext * g );
	virtual ~Machine4() {}

};

#endif
