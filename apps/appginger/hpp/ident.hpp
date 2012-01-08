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

#ifndef IDENT_HPP
#define IDENT_HPP

#include "shared.hpp"
#include "valof.hpp"

#include <string>
#include <memory>

#include "common.hpp"


class IdentClass {
private:
	const std::string	name_data;

public:
	Valof * 			value_of;		//	used for global idents

public:
	const std::string & getNameString() const { return this->name_data; }
	
public:
	IdentClass( const std::string & nm ); 
};

typedef shared< IdentClass > Ident;

Ident identNewGlobal( const std::string nm );

#endif

