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

IdentClass::IdentClass( const std::string & nm, const FacetSet * facets ) :
	is_local( false ),
	name( nm ),
	//facet( facet ),
	facets( facets ),
	slot( -1 ),
	value_of( new Valof() ),
	level( -1 )
{
}

const std::string & IdentClass::getNameString() {
	return this->name;
}

bool IdentClass::isSame( IdentClass * other ) {
	return this->name == other->name;
}

Ident ident_new_local( const std::string & nm ) {
	IdentClass * id = new IdentClass( nm, NULL );
	id->setLocal();
	return shared< IdentClass >( id );
}

Ident ident_new_tmp( const int n ) {
	IdentClass * id = new IdentClass( std::string( "tmpvar" ), NULL );
	id->setLocal();
	id->slot = n;
	return shared< IdentClass >( id );
}

Ident ident_new_global( const std::string & nm, /*const Facet * facet,*/ const FacetSet * facets ) {
	IdentClass * id = new IdentClass( nm, /*facet,*/ facets );
	id->setGlobal();
	return shared< IdentClass >( id );
}
