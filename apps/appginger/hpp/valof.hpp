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

#ifndef VALOF_HPP
#define VALOF_HPP

//	For Ref
#include "common.hpp"

//	For SYS_UNDEF
#include "key.hpp"

//	For std::string
#include <string>

namespace Ginger {

class Valof {
public:
	Ref					valof;
	
private:
	class Package * 	parent;
	const std::string	name_data;

private:	//	Flags.
	bool				prot;

public:
	const std::string & getNameString() const { return this->name_data; }
	bool isProtected() const { return this->prot; }
	
public:
	Valof( class Package * parent, const std::string & nm, Ref r = SYS_UNASSIGNED ) :
		valof( r ),
		parent( parent ),
		name_data( nm ),
		prot( false )
	{}

	Valof( class Package * parent, const std::string & nm, bool prot, Ref r = SYS_UNASSIGNED ) :
		valof( r ),
		parent( parent ),
		name_data( nm ),
		prot( prot )
	{}


};

} // namespace Ginger

#endif
