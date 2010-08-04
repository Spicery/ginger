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

#include "makesysfn.hpp"
#include "sys.hpp"
#include "vmi.hpp"
#include "mishap.hpp"

Ref makeSysFn( Plant plant, std::string fn_name ) {

	SysMap::iterator smit = sysMap.find( fn_name );
	if ( smit == sysMap.end() ) {
		throw Mishap( "No such system call" ).culprit( "Name", fn_name );
	}
	SysInfo & info = smit->second;
	
	Ref x = info.coreFunctionObject;
	if ( x != NULL ) return x;

	vmiFUNCTION( plant, info.in_arity.count(), info.out_arity.count() );
	vmiSYS_CALL( plant, info.syscall );	
	vmiSYS_RETURN( plant );
	Ref r = vmiENDFUNCTION( plant, false );
	
	info.coreFunctionObject = r;
	return r;
	
}
