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

#include <iostream>
#include <sstream>
using namespace std;


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "sysprint.hpp"
#include "mishap.hpp"

void this_never_happens() {
	throw Mishap( "This never happens" );
}

Problem & Problem::culprit( const std::string reason, const std::string arg ) {
	this->culprits.push_back( pair< const string, const string >( reason, arg ) );
	return *this;
}

Problem & Problem::culprit( const std::string reason, Ref ref ) {
	std::ostringstream s;
	refPrint( s, ref );
	const std::string result( s.str() );
	//this->culprits( reason, result );
	this->culprits.push_back( pair< const string, const string >( reason, result ) );
	return *this;
}

Problem & Problem::culprit( const std::string arg ) {
	this->culprits.push_back( std::pair< const string, const string >( "Argument", arg ) );
	return *this;
}

void Problem::report() {
	cerr << "MISHAP: " << this->message << endl;
	for ( 	
		vector< pair< string, string > >::iterator it = this->culprits.begin();
		it != this->culprits.end();
		++it
	) {
		cerr << it->first << " : " << it->second << endl;
	}
}


