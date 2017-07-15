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

#include <iostream>

#include "scanfunc.hpp"
#include "instruction_set.hpp"
#include "key.hpp"

namespace Ginger {

ScanFunc::ScanFunc( const InstructionSet & ins, Ref * r ) {
	this->inset = ins;
	this->pc = r + 1;
	this->offset = 0;
	Ref * start = r - 4;
	unsigned long L = ToULong( *start ) >> TAGGG;			
	this->end = start + L;
}

Ref * ScanFunc::next() {
	while ( this->pc < this->end ) {
		//std::cerr << "Instruction " << this->inset.name( *this->pc ) << std::endl;
		const char *types = this->inset.signature( *this->pc );
		for (;;) {
			const char ch = types[ this->offset++ ];
			if ( ch == '\0' ) {
				break;			
			} else if ( ch == 'c' || ch == 'v' ) {
				return this->pc  + this->offset - 1;
			}
		}
		this->pc += this->offset - 1;
		this->offset = 0;
	}
	return 0;
}

} // namespace Ginger
