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

#include "registers.hpp"

#include "key.hpp"
#include "mishap.hpp"

namespace Ginger {

Registers::Registers() :	
	free_hint( 0 ),
	in_use( 0 )
{
	for ( int i = 0; i < NREG; i++ ) {
		this->regs[ i ] = SYS_SYSTEM_ONLY;
	}
}

/*
	Used prior to garbage collection so that
	unused registers do not get traced.
*/
void Registers::clearUnusedRegisters() {
	for ( int i = 0; i < NREG; i++ ) {
		if ( ( ( this->in_use >> i ) & 0x1 ) == 0 ) {
			this->regs[ i ] = SYS_SYSTEM_ONLY;
		}
	}
}

//
//	Start searching for the next free register from
//	free_hint but don't rely on it being accurate.
//	Always check against the in_use bit-mask, which
//	is guaranteed to be accurate. After this operation
//	has completed, if there are any free registers, 
//	-free_hint- will be accurate.
//
Ref & Registers::reserve( unsigned long & mask ) {
	for ( int i = this->free_hint; i < NREG; i++ ) {
		if ( ( ( this->in_use >> i ) & 0x1 ) == 0 ) {
			this->free_hint = ( i + 1 ) & NREG_MASK;
			mask |= ( 1 << i );
			return this->regs[ i ];
		}
	}
	for ( int i = 0; i < this->free_hint; i++ ) {
		if ( ( ( this->in_use >> i ) & 0x1 ) == 0 ) {
			this->free_hint = ( i + 1 ) & NREG_MASK;
			mask |= ( 1 << i );
			return this->regs[ i ];
		}
	}
	throw SystemError( "Too few registers" );
}

void Registers::release( int count, unsigned long mask ) {
	//	This is a way of safely subtracting -count- from -free_hint-.
	//	If it isn't accurate it doesn't matter. It gets fixed at the
	//	next -reserve-.
	this->free_hint = ( this->free_hint + NREG - count ) & NREG_MASK;
	//	
	this->in_use &= ~mask;
}

} // namespace Ginger
