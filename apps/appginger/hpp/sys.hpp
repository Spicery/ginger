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

#ifndef SYS_HPP
#define SYS_HPP

#include <map>
#include <iostream>

#include "common.hpp"
#include "arity.hpp"

class MachineClass;

extern Ref * sysExplode( Ref *pc, class MachineClass * vm );
extern Ref * sysIndex( Ref *pc, class MachineClass * vm );
extern Ref * sysLength( Ref *pc, class MachineClass * vm );
extern Ref * sysHash( Ref *pc, class MachineClass * vm );
extern Ref * sysAppend( Ref *pc, class MachineClass * vm );
extern Ref * sysFastGetFastIterator( Ref * pc, class MachineClass * vm );

#include "datatypes.hpp.auto"

struct SysInfo { 
	Functor functor; 
	Arity in_arity;
	Arity out_arity; 
	SysCall * syscall; 
	const char * docstring;
	Ref coreFunctionObject;
	
	SysInfo( Functor f, Arity in, Arity out, SysCall * s, const char * ds ) :
		functor( f ),
		in_arity( in ),
		out_arity( out ),
		syscall( s ),
		docstring( ds ),
		coreFunctionObject( NULL )
	{
	}
	
	
};
typedef std::map< std::string, SysInfo > SysMap;
extern SysMap sysMap;

#endif

