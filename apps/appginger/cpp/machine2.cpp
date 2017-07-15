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

#ifndef MACHINE2_EXCLUDED

#include "machine2.hpp"
#include "enginefactory.hpp"


//	Now source the auto-generated C++ file. 
#include "instructions_context.hpp"

namespace Ginger {

Ref *pc;
Machine vm;


typedef void SpecialFn( void );
typedef SpecialFn *Special;

Machine2::Machine2( AppContext * g ) :
	MachineClass( g )
{
}


void Machine2::execute( Ref r, const bool clear_stack ) {
	pc = this->setUpPC( r, clear_stack );
	vm = this;
	for (;;) {		
		Special fn = (Special)( *pc );
		#ifdef DBG_MACHINE
			special_show( pc );
		#endif
		fn();
	}
}

#include "machine2.cpp.auto"

class Engine2Factory : public Ginger::EngineFactory {
public:
	MachineClass * newEngine( AppContext * cxt ) {
		return new Machine2( cxt );
	}
public:
	Engine2Factory() :
		EngineFactory( 
			"2", 
			"globfn", 
			"Instructions are pointers to functions that take their arguments from global register variables"
		)
	{}

	virtual ~Engine2Factory() {}
};
Ginger::EngineFactoryRegistration engine2factory ( 
	new Engine2Factory()
);

} // namespace Ginger

#endif
