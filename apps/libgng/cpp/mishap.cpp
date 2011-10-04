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


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "mishap.hpp"

namespace Ginger {
using namespace std;

Problem & Problem::culprit( const std::string reason, const std::string arg ) {
	this->culprits.push_back( pair< const string, const string >( reason, arg ) );
	return *this;
}

Problem & Problem::culprit( const char * reason, const std::string arg ) {
	this->culprits.push_back( pair< const string, const string >( reason, arg ) );
	return *this;
}

Problem & Problem::culprit( const char * reason, const char * arg ) {
	this->culprits.push_back( pair< const string, const string >( reason, arg ) );
	return *this;
}

Problem & Problem::culprit( const std::string reason, const long N ) {
	std::ostringstream s;
	s << N;
	const std::string result( s.str() );
	this->culprits.push_back( pair< const string, const string >( reason, result ) );
	return *this;
}

Problem & Problem::culprit( const std::string reason, const char N ) {
	std::ostringstream s;
	s << N;
	const std::string result( s.str() );
	this->culprits.push_back( pair< const string, const string >( reason, result ) );
	return *this;
}

Problem & Problem::culprit( const std::string arg ) {
	this->culprits.push_back( std::pair< const string, const string >( "Argument", arg ) );
	return *this;
}

void Problem::report() {
	cerr << "Problem: " << this->message << endl;
	for ( 	
		vector< pair< string, string > >::iterator it = this->culprits.begin();
		it != this->culprits.end();
		++it
	) {
		cerr << it->first << " : " << it->second << endl;
	}
}

void Problem::gnxReport() {
	cout << "<Problem message=\"" << this->message << "\">";
	for ( 	
		vector< pair< string, string > >::iterator it = this->culprits.begin();
		it != this->culprits.end();
		++it
	) {
		cout << "<culprit name=\"" << it->first << "\" value=\"" << it->second << "\" />";
	}
	cout << "</Problem>" << endl;
	
}

std::string Problem::getMessage() {
	return this->message;
}

std::pair< std::string, std::string > & Problem::getCulprit( int n ) {
	return this->culprits[ n ];
}

int Problem::getCount() {
	return this->culprits.size();
}

Unreachable::Unreachable( const char * file, int line ) : 
	SystemError( "Unreachable" ) 
{
	stringstream s;
	s << line;
	this->culprit( "File", file );
	this->culprit( "Line", s.str() );
}

} // namespace Ginger
