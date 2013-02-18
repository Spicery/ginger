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



#include "fnobjcrawl.hpp"

#include "misclayout.hpp"

Ref * FnObjCrawl::next() {
	if ( this->datapool == NULL ) {
		while ( this->pc < this->obj_N ) {
			if ( *types == '\0' ) {
				this->types = ins.signature( *pc );
				this->name = ins.name( *pc );
				this->sig = this->types;
			} else {
				const char ch = *types++;
				Ref * current = pc++;
				if ( ch == 'c' ) return current;
				
				//	If we want to be able to delete the global dictionary
				//	then we must add in tracing through Idents too. They
				//	have the char 'v'.
				
			}
		}
		return static_cast< Ref * >( 0 );
	} else {
		Ref * d = this->datapool;
		this->datapool = NULL;
		return d;
	}
}

FnObjCrawl::FnObjCrawl( MachineClass * vm, Ref * obj_K ) :
	datapool( &obj_K[ OFFSET_TO_DATA_POOL ] ),
	ins( vm->instructionSet() ),
	types( "" )
{	
	this->pc = obj_K + 1;
	this->obj_N = this->pc + sizeInstructionsAfterKeyOfFn( obj_K );
}


