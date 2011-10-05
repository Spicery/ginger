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

#include "machine3.hpp"

#include <cstdio>
#include <cstdlib>

using namespace Ginger;


//	Now source the auto-generated C++ file. 
#include "instructions_context.hpp"



Machine3::Machine3( AppContext & g ) :
	MachineClass( g )
{
	this->core( true, 0 );
}

void Machine3::execute( Ref r ) {
	Ref * PC = this->setUpPC( r );
	this->core( false, PC );
}

#define vm 		this
#define VMCOUNT ( vm->count )
#define MELT { VMSP = vm->sp; VMVP = vm->vp; VMLINK = vm->link; }
#define FREEZE { vm->sp = VMSP; vm->vp = VMVP; vm->link = VMLINK; }
#define VMLINKFUNC 	( vm->func_of_link )
#define VMPCFUNC	( vm->func_of_program_counter )
typedef void *Special;
#define RETURN( e )     { pc = ( e ); goto **pc; }

#include "machine3.cpp.auto"
