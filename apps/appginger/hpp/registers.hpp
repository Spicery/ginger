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

#ifndef REGISTERS_HPP
#define REGISTERS_HPP

#include "common.hpp"

namespace Ginger {

class Registers {
friend class GarbageCollect;
friend class Roots;
private:
	static const int NREG = 8;			//	Must be a power of 2.
	static const int NREG_MASK = 0x7;	//	Must be the bitmask for that power of 2.
	int				free_hint;
	unsigned long	in_use;
	Ref				regs[ NREG ];
	
private:	
	//	Functions for garbage collection.
	void 		clearUnusedRegisters();
	Ref &		get( int n ) { return this->regs[ n ]; }

public:
	Ref &		reserve( unsigned long & mask );
	void		release( int count, unsigned long mask );

public:
	Registers();
};

} // namespace Ginger

#endif
