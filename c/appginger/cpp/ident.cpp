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

IdentClass::IdentClass( const std::string & nm ) :
	name( nm ),
	is_local( false ),
	slot( -1 ),
	valof( NULL ),
	level( -1 ),
	next( NULL )
{
}

const std::string & IdentClass::getNameString() {
	return this->name;
}

bool IdentClass::isSame( IdentClass * other ) {
	return this->name == other->name;
}

Ident ident_new_local( const std::string & nm ) {
	IdentClass * id = new IdentClass( nm );
	id->is_local = true;
	return shared< IdentClass >( id );
}

Ident ident_new_tmp( const int n ) {
	IdentClass * id = new IdentClass( std::string( "tmpvar" ) );
	id->is_local = true;
	id->slot = n;
	return shared< IdentClass >( id );
}

Ident ident_new_global( const std::string & nm ) {
	IdentClass * id = new IdentClass( nm );
	id->is_local = false;
	return shared< IdentClass >( id );
}