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

#include "facet.hpp"

#include <string>
#include <map>
using namespace std;

static std::map< std::string, Facet * > table;

const Facet * fetchFacet( const std::string & name ) {
	map< string, Facet * >::iterator it = table.find( name );
	if ( it != table.end() ) {
		return it->second;
	} else {
		table[ name ] = new Facet( name );
		return table[ name ];
	}
}


static map< set< string >, FacetSet * > table_set;

const FacetSet * fetchFacetSet( set< string > & names ) {
	map< set< string >, FacetSet * >::iterator it = table_set.find( names );
	if ( it != table_set.end() ) {
		return it->second;
	} else {
		return table_set[ names ] = new FacetSet( names );
	}
}

const FacetSet * fetchFacetSet( string & name ) {
	set< string > names;
	names.insert( name );
	return fetchFacetSet( names );
}

const FacetSet * fetchFacetSet( const char * name ) {
	set< string > names;
	names.insert( name );
	return fetchFacetSet( names );
}

FacetSet::FacetSet( std::set< std::string > & names ) :
	names_set( names )
{
	static int counter = 0;
	this->id = counter++;
}

bool FacetSet::contains( const Facet * c ) const {
	return c != NULL && this->names_set.find( c->name_data ) != this->names_set.end();
}

ostream& operator<<( ostream & out, const FacetSet & facets ) {	
	out << "{";
	const char * gap = "";
	for ( 
		std::set< std::string >::iterator it = facets.names_set.begin();
		it != facets.names_set.end();
		++it
	) {
		out << gap << "@" << *it;
		gap = ",";
	}
	out << "}";
	return out;
}

ostream& operator<<( ostream & out, const Facet & facet ) {	
	out << "@" << facet.name_data;
	return out;
}
