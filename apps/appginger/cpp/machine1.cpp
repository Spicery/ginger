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

#ifndef MACHINE1_EXCLUDED

#include "machine1.hpp"
#include "enginefactory.hpp"

//	Now source the auto-generated C++ file. 
#include "instructions_context.hpp"

namespace Ginger {

typedef Ref *SpecialFn( Ref *pc, Machine vm );
typedef SpecialFn *Special;

Machine1::Machine1( AppContext * g ) :
	MachineClass( g )
{
}

/*static void sanity_check( Ref * func_obj_K ) {
	if ( func_obj_K != 0 && *func_obj_K != sysFunctionKey ) throw;
}*/
	

void Machine1::execute( Ref r, const bool clear_stack ) {
	Ref * PC = this->setUpPC( r, clear_stack );
	for (;;) {			
		//sanity_check( this->func_of_program_counter ); // debug
		Special fn = (Special)( *PC );
		#ifdef DBG_MACHINE
			special_show( PC );
		#endif
		PC = fn( PC, this );
	}
}

#include "machine1.cpp.auto"

class Engine1Factory : public Ginger::EngineFactory {
public:
	MachineClass * newEngine( AppContext * cxt ) {
		return new Machine1( cxt );
	}
public:
	Engine1Factory() :
		EngineFactory( 
			"1", 
			"sysfn", 
			"Instructions are pointers to functions that are passed the virtual program counter and engine pointer"
		)
	{}

	virtual ~Engine1Factory() {}
};
Ginger::EngineFactoryRegistration engine1factory ( 
	new Engine1Factory()
);

} // namespace Ginger

#endif