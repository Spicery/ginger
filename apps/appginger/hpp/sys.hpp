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

#ifndef GNG_SYS_HPP
#define GNG_SYS_HPP

#include <map>
#include <iostream>

#include "arity.hpp"

#include "common.hpp"
#include "cmp.hpp"
#include "mishap.hpp"

class MachineClass;

extern Ref * sysExplode( Ref *pc, class MachineClass * vm );
extern Ref * sysUpdaterOfExplode( Ref *pc, class MachineClass * vm );
extern Ref * sysIndex( Ref *pc, class MachineClass * vm );
extern Ref * sysUpdaterOfIndex( Ref *pc, class MachineClass * vm );
extern Ref * sysLength( Ref *pc, class MachineClass * vm );
extern Ref * sysHash( Ref *pc, class MachineClass * vm );
extern Ref * sysAppend( Ref *pc, class MachineClass * vm );
extern Ref * sysFastGetFastIterator( Ref * pc, class MachineClass * vm );
extern Ref * sysShowMeRuntimeInfo( Ref * pc, class MachineClass * vm );

#include "datatypes.hpp.auto"

enum InfoFlavour {
	VM_OP_FLAVOUR,
	CMP_OP_FLAVOUR,
	SYS_CALL_FLAVOUR
};

/**
 * 	SysNames is anticipating the need for language-specific bindings
 * 	for built-in functions. At the moment it is simply a name.
 */
class SysNames {
private:
	const char * def_name;

public:
	SysNames() : def_name( NULL ) {}
	SysNames( const char * _name ) : def_name( _name ) {}

public:
	const char * name() {
		if ( this->def_name == NULL ) throw Ginger::Unreachable( __FILE__, __LINE__ );
		return this->def_name;
	}	
};

struct SysInfo { 
	SysNames names;
	InfoFlavour flavour;		//	Is this system routine implemented as a VM instruction, comparison operator or as a system-call.
	Instruction instruction;	//	Populated if VM_OP_FLAVOUR (else vmc_halt).
	CMP_OP cmp_op;				//	Populated if CMP_OP_FLAVOUR (else CMP_EQ)
	Ginger::Arity in_arity;
	Ginger::Arity out_arity; 
	SysCall * syscall; 			//	Populated if SYS_CALL_FLAVOUR (else NULL).
	const char * docstring;
	Ref coreFunctionObject;		//	Memo cache.
	
	bool isSysCall() const { return this->flavour == SYS_CALL_FLAVOUR; }
	bool isCmpOp() const { return this->flavour == CMP_OP_FLAVOUR; }
	bool isVMOp() const { return this->flavour == VM_OP_FLAVOUR; }

	const char * name() {
		return this->names.name();
	}
	
	SysInfo( Instruction ins, Ginger::Arity in, Ginger::Arity out, const char * ds ) :
		flavour( VM_OP_FLAVOUR ),
		instruction( ins ),
		cmp_op( CMP_EQ ),
		in_arity( in ),
		out_arity( out ),
		syscall( NULL ),
		docstring( ds ),
		coreFunctionObject( NULL )
	{
	}
	
	SysInfo( Ginger::Arity in, Ginger::Arity out, SysCall * s, const char * ds ) :
		flavour( SYS_CALL_FLAVOUR ),
		instruction( vmc_halt ),
		cmp_op( CMP_EQ ),
		in_arity( in ),
		out_arity( out ),
		syscall( s ),
		docstring( ds ),
		coreFunctionObject( NULL )
	{
	}

	SysInfo( SysNames _names, Ginger::Arity in, Ginger::Arity out, SysCall * s, const char * ds ) :
		names( _names ),
		flavour( SYS_CALL_FLAVOUR ),
		instruction( vmc_halt ),
		cmp_op( CMP_EQ ),
		in_arity( in ),
		out_arity( out ),
		syscall( s ),
		docstring( ds ),
		coreFunctionObject( NULL )
	{
	}

	SysInfo( const CMP_OP cmp_op, Ginger::Arity in, Ginger::Arity out, const char * ds ) :
		flavour( CMP_OP_FLAVOUR ),
		instruction( vmc_halt ),
		cmp_op( cmp_op ),
		in_arity( in ),
		out_arity( out ),
		syscall( NULL ),
		docstring( ds ),
		coreFunctionObject( NULL )
	{
	}
	
};
typedef std::map< std::string, SysInfo > SysMap;
extern SysMap sysMap;

#endif

