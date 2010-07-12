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

#include <string>
#include <memory>

#include "common.hpp"


class IdentClass {
public:
	std::string		name;
	bool			is_local;
	int				slot; 		//	used for local idents
	Ref				valof;		//	used for global idents
	int 			level;		//	level of scope
	IdentClass		*next;      //	chain used for env linking

public:
	bool isSame( IdentClass * other );
	
public:
	IdentClass( const std::string & nm );
	const std::string & getNameString();
};

typedef shared< IdentClass > Ident;


Ident ident_new_local( const std::string & nm );
Ident ident_new_tmp( int n );
Ident ident_new_global( const std::string & nm );

#endif
