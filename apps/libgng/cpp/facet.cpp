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

#include <stddef.h>


namespace Ginger {
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

const FacetSet * fetchEmptyFacetSet() {
	static set< string > empty;
	return fetchFacetSet( empty );
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

bool FacetSet::isntEmptyIntersection( const FacetSet * fs ) const {
	std::set< std::string >::iterator left = this->names_set.begin();
	std::set< std::string >::iterator left_end = this->names_set.end();
	std::set< std::string >::iterator right = fs->names_set.begin();
	std::set< std::string >::iterator right_end = fs->names_set.end();
	while ( left != left_end && right != right_end ) {
		if ( *left < *right ) { 
			++left; 
		} else if ( *right < *left ) {
			++right;
		} else {
			return true;
		}
	}
	return false;
}

bool FacetSet::isEmptyIntersection( const FacetSet * fs ) const {
	return !this->isntEmptyIntersection( fs );
}

bool FacetSet::isEmpty() const {
	return this->names_set.empty();
}



const FacetSet * FacetSet::add( const Facet * f ) const {
	set< string > s = this->names_set;
	s.insert( f->name() );
	return fetchFacetSet( s );
}

const FacetSet * FacetSet::add( const string & x ) const {
	set< string > s( this->names_set );
	s.insert( x );
	return fetchFacetSet( s );
}

const FacetSet * FacetSet::addSet( const FacetSet * fs ) const {
	set< string > s( this->names_set );
	for ( 
		set< string >::iterator it = fs->names_set.begin();
		it != fs->names_set.end();
		++it
	) {
		s.insert( *it );
	}
	return fetchFacetSet( s );
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



} // Namespace
