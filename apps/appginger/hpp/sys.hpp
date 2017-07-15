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

#ifndef GNG_SYS_HPP
#define GNG_SYS_HPP

#include <map>
#include <iostream>
#include <list>
#include <utility>

#include "arity.hpp"

#include "common.hpp"
#include "cmp.hpp"
#include "mishap.hpp"

namespace Ginger {

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
 * 	Synonyms anticipate the need for language-specific bindings
 * 	for built-in functions. At the moment it is simply a base-name and
 * 	synonyms for it.
 *
 *	In general we need
 *		- base-name: the global system function name used in GNX.
 *		- for each entry in a standard library
 *			+ name of package (currently only ginger.library)
 *			+ variable name
 *	
 * 	The next feature I want to accomodate is a system function
 *	that isn't exposed in a library but only as a <sysapp name=NAME/>.
 */

class FullName {
private:
	const char * pkg_name;
	const char * base_name;

public:
	FullName() : pkg_name( nullptr ), base_name( nullptr ) {}
	FullName( const char * _base_name ) : pkg_name( "ginger.library" ), base_name( _base_name ) {}
	FullName( const char * _pkg_name, const char * _base_name ) : pkg_name( _pkg_name ), base_name( _base_name ) {}

public:
	const char * baseName() const { return this->base_name; }
	const char * pkgName() const { return this->pkg_name; }
};

typedef std::list< FullName > SynonymList;

class SysInfo;

//	This is a point for future expansion.
class SysModule {
private:
	const char * module_name;
public:
	const char * moduleName() const { return this->module_name; }
	void registerSysFunction( SysInfo * info );
public:
	SysModule( const char * _module_name ) : module_name( _module_name ) {}
};


class SysInfo { 
public:
	SysModule * sys_module;
	FullName full_name;
	SynonymList synonym_list;
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

	const char * baseName() {
		return this->full_name.baseName();
	}

	const char * pkgName() {
		return this->full_name.pkgName();
	}

	void addSynonym( const char * _pkg_name, const char * _name ) {
		this->synonym_list.push_back( FullName( _pkg_name, _name ) );
	}

	SynonymList & synonyms() {
		return this->synonym_list;
	}

	//	Only declared to allow adding to std::map, which requires a
	//	nullary constructor.
	SysInfo() {}
	
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

	//	This is the one that causes self-registration.
	SysInfo( FullName _name, Ginger::Arity in, Ginger::Arity out, SysCall * s, const char * ds );

	SysInfo( SysModule * _module, FullName _name, Ginger::Arity in, Ginger::Arity out, SysCall * s, const char * ds );

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

class SysMap : public std::map< std::string, SysInfo > {
	typedef const std::map< std::string, SysInfo >::value_type * iter;
private:
	SysMap( iter start, iter end ) : std::map< std::string, SysInfo >( start, end ) {}
	//SysMap( const SysMap::value_type * start, const SysMap::value_type * end ) : std::map< std::string, SysInfo >( start, end ) {}

public:
	static SysMap & systemFunctionsMap(); 
};

class RegisterSysAltName {
public:
	RegisterSysAltName( SysInfo & info, const char * _pkg_name, const char * _base_name );
};

} // namespace Ginger

#endif

