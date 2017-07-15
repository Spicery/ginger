/******************************************************************************\
	Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
	of the GNU General Public License. This file is part of Ginger.

    Ginger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Ginger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Ginger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

#include <fstream>
#include <iostream>
#include <utility>
#include <string>
#include <set>

#include "importinfo.hpp"
#include "mishap.hpp"

using namespace std;

#define FROM 				"from"
#define MATCH 				"match"
#define MATCH_SIZE	 		( sizeof( MATCH ) - 1 )
#define INTO				"into"
#define INTO_SIZE			( sizeof( INTO ) - 1 )
#define ALIAS				"alias"

typedef std::map< std::string, std::string > Dict;

ImportInfo::ImportInfo( Dict & attrs ) : 
	attrs( attrs )
{
	Dict::iterator it = attrs.find( FROM );
	if ( it == attrs.end() ) {
		throw Mishap( "Missing attribute in import" ).culprit( "Attribute", FROM );
	}
	this->from = it->second;
	
	set< string > matches;
	set< string > intos;
	for (
		Dict::iterator it = attrs.begin();
		it != attrs.end();
		++it
	) {
		if ( it->first.compare( 0, MATCH_SIZE, MATCH ) == 0 ) {
			matches.insert( it->second );
		} else if ( it->first.compare( 0, INTO_SIZE, INTO ) == 0 ) {
			intos.insert( it->second );
		}
	}
	
	this->match_tags = fetchFacetSet( matches );
	this->into_tags = fetchFacetSet( intos );
}

bool ImportInfo::hasAlias( const string & alias ) {
	Dict::iterator it = attrs.find( ALIAS );
	if ( it == attrs.end() ) {
		return false;
	} else {
		return alias == it->second;
	}
}

void ImportInfo::printInfo() {
	for ( 
		Dict::iterator jt = this->attrs.begin();
		jt != this->attrs.end();
		++jt
	) {
		cout << jt->first << " -> " << jt->second << endl;
	}	
}

const string & ImportInfo::getFrom() {
	return this->from;
}

bool ImportInfo::matches( const string & tag ) {
	for ( 
		std::map< std::string, std::string >::iterator it = this->attrs.begin();
		it != attrs.end();
		++it
	) {
		const string & key = it->first;
		
		//	The following is the idiom for being a prefix.
		if ( key.compare( 0, MATCH_SIZE, MATCH ) == 0 ) {	
			const string & value = it->second;
			//	cout << "Comparing {" << value << "} with {" << tag << "}" << endl;
			if ( value == tag ) return true;
		}
	}
	return false;
}

