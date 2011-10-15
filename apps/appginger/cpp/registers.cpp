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
	throw Ginger::SystemError( "Too few registers" );
}

void Registers::release( int count, unsigned long mask ) {
	this->free_hint = ( this->free_hint + NREG - count ) & NREG_MASK;
	this->in_use &= ~mask;
}
