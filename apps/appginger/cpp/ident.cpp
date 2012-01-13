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

#include <string>

#include "ident.hpp"
#include "key.hpp"

IdentClass::IdentClass( class Package * parent, const std::string & nm ) : //, const FacetSet * facets ) :
	parent( parent ),
	name_data( nm )
{
}

Ident identNewGlobal( class Package * parent, const std::string nm ) {
	IdentClass * id = new IdentClass( parent, nm );
	return shared< IdentClass >( id );
}
