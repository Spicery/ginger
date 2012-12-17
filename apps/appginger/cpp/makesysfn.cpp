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

#include <stddef.h>

#include "makesysfn.hpp"
#include "sys.hpp"
#include "codegen.hpp"
#include "mishap.hpp"


Ref makeSysFn( CodeGen codegen, std::string fn_name, Ref default_value ) {

	SysMap::iterator smit = sysMap.find( fn_name );
	if ( smit == sysMap.end() ) {
		return default_value;
	}
	SysInfo & info = smit->second;
	
	Ref x = info.coreFunctionObject;
	if ( x != NULL ) return x;

	codegen->vmiFUNCTION( fn_name, info.in_arity.count(), info.out_arity.count() );
	
	//	We have two different kinds of system functions. Those that are
	//	implemented as native instructions and those that are implemented
	//	by hand-written functions.
	//
	//	The test that distinguishes them is unsatisfactory because it fails
	//	to distinguish an omitted something from a genuine choice.
	//	REFACTOR.
	//
	if ( info.isSysCall() ) {
		//	Hand-written function.
		codegen->vmiSYS_CALL( info.syscall );	
	} else if ( info.isVMOp() ) {
		//	Native instruction.
		codegen->vmiINSTRUCTION( info.instruction );
	} else {
		throw Ginger::SystemError( "Internal error" );
	}
	codegen->vmiSYS_RETURN();
	Ref r = codegen->vmiENDFUNCTION( false );
	
	info.coreFunctionObject = r;
	return r;
	
}

