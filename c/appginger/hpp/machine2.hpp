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

#ifndef MACHINE2_HPP
#define MACHINE2_HPP

#include "machine.hpp"
#include "callstacklayout.hpp"
#include "listlayout.hpp"
#include "instruction_set2.hpp"
#include "appginger.hpp"

class Machine2 : public MachineClass {

private:
	InstructionSet2 instruction_set;
	
public:
	virtual void execute( Ref r );
	const InstructionSet & instructionSet() {
		return this->instruction_set;
	}
	
public:
	Machine2( AppGinger & application );
	virtual ~Machine2() {}

};

#endif