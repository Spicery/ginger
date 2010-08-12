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

#include "dict.hpp"

Ident DictClass::lookup( const std::string & s ) {
	std::map< std::string, Ident >::iterator it = this->table.find( s );
	return it == this->table.end() ? shared< IdentClass >() : it->second;
}

Ident DictClass::add( const std::string & s, /*const Facet * facet,*/ const FacetSet * facets ) {
	Ident id = ident_new_global( s, /*facet,*/ facets );
    return this->table[ s ] = id;
}

Ident DictClass::lookup_or_add( const std::string & c, /*const Facet * facet,*/ const FacetSet * facets ) {
    Ident id = this->lookup( c );
	if ( not id ) {
    	return this->add( c, /*facet,*/ facets );
    } else {
    	return id;
    }
}