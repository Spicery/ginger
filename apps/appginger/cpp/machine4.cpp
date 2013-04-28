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

#ifndef MACHINE4_EXCLUDED

#include <iostream>
//#include <cstdio>
#include <cstdlib>

#include "machine4.hpp"
#include "enginefactory.hpp"

#include "instructions_context.hpp"

using namespace Ginger;
using namespace std;

#define FREEZE
#define MELT
#define vm 		this
#define VMLINK 	( vm->link )
#define VMSP	( vm->sp )
#define VMVP	( vm->vp )
#define VMCOUNT	( vm->count )
#define VMLINKFUNC 	( vm->func_of_link )
#define VMPCFUNC	( vm->func_of_program_counter )
#define RETURN( e ) 	{ pc = e; goto execute_loop; }

typedef Instruction Special;

Machine4::Machine4( AppContext * g ) : MachineClass( g ) {}

void Machine4::execute( Ref r, const bool clear_stack ) {
	Ref * pc = this->setUpPC( r, clear_stack );
	execute_loop: {
		Special code = *reinterpret_cast< Special * >( pc );
		switch ( code ) {
			#include "machine4.cpp.auto"
			default: throw SystemError( "Invalid instruction" );
		}
		throw SystemError( "Instructions may not fall thru" );
	}
}

class Engine4Factory : public Ginger::EngineFactory {
public:
	MachineClass * newEngine( ::AppContext * cxt ) {
		return new Machine4( cxt );
	}
public:
	Engine4Factory() :
		EngineFactory( 
			"4", 
			"bytecoded", 
			"Instructions are small integers that are used as the argument to a large switch in the main interpreter loop"
		)
	{}

	virtual ~Engine4Factory() {}
};
Ginger::EngineFactoryRegistration engine4factory ( 
	new Engine4Factory()
);

#endif