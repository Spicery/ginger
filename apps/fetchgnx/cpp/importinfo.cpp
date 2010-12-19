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

#include <fstream>
#include <iostream>
#include <utility>

#include "importinfo.hpp"
#include "mishap.hpp"

using namespace std;

#define FROM 				"from"
#define MATCH 				"match"
#define MATCH_SIZE	 		( sizeof( MATCH ) - 1 )

typedef std::map< std::string, std::string > Dict;

ImportInfo::ImportInfo( Dict & attrs ) : 
	attrs( attrs ) 
{
	Dict::iterator it = attrs.find( FROM );
	if ( it == attrs.end() ) {
		throw Mishap( "Missing attribute in import" ).culprit( "Attribute", FROM );
	}
	this->from = it->second;
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
			//cout << "Comparing {" << value << "} with {" << tag << "}" << endl;
			if ( value == tag ) return true;
		}
	}
	return false;
}

