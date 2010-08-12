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

#ifndef FACET_HPP
#define FACET_HPP

#include <string>
#include <set>
#include <iostream>

class Facet {
friend class FacetSet;
private:
	const std::string name_data;
	
public:
	const std::string & name() const {
		return this->name_data;
	}

public:
	Facet( const std::string & nm ) : name_data( nm ) {}
};

extern const Facet * fetchFacet( const std::string & string );

class FacetSet {
private:
	std::set< std::string > names_set;
	int id;

public:
	bool contains( const Facet * c ) const;
	
/*public:
	void debug() const {
		std::cout << "Debug facetset (" << id << "," << names_set.size() << "): ";
		for ( 
			std::set< std::string >::iterator it = names_set.begin();
			it != names_set.end();
			++it
		) {
			std::cout << " " << *it;
		}
		std::cout << std::endl;
	}*/
	
public:
	FacetSet( std::set< std::string > & names );
};

extern const FacetSet * fetchFacetSet( std::set< std::string > & names );
extern const FacetSet * fetchFacetSet( std::string & name );
extern const FacetSet * fetchFacetSet( const char * name );

#endif
