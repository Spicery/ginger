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

#include "scancallstack.hpp"



Ref * ScanCallStack::next() {
	this->is_func = false;	//	debug
	if ( vm->sp_base <= this->frame_pointer && this->frame_pointer < vm->sp_end ) {
		if ( this->state == ScanFunc ) {
			Ref * func = ToRefRef( this->frame_pointer[ SP_FUNC ] );
			this->state = ScanLocals;
			this->is_func = true;	//	debug
			return func;
		} else if ( this->state == ScanLocals ) {
			unsigned long nslots = ToULong( this->frame_pointer[ SP_NSLOTS ] );
			if  ( this->local < nslots ) {
				return &this->frame_pointer[ this->local++ ];
			} else {
				this->state = ScanFunc;
				this->local = 0;
				this->frame_pointer = ToRefRef( this->frame_pointer[ SP_PREV_SP ] );
				return this->next();
			}
		} else {
			throw "Unreachable";
		}
	} else {
		return static_cast< Ref * >( 0 );
	}
}


ScanCallStack::ScanCallStack( MachineClass * m ) : 
	vm( m ),
	frame_pointer( m->sp ),
	state( ScanFunc ),
	local( 0 )
{
}

