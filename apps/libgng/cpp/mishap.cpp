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
#include <iomanip>
#include <cstring> 

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "mnx.hpp"
#include "mishap.hpp"
#include "gngversion.hpp"

namespace Ginger {
using namespace std;

#define PREFIX "    "
#define ORIGIN ( PREFIX "Cause" )

Problem & Problem::cause( Problem & problem ) {
	this->culprit( ORIGIN, problem.getMessage() );
	for ( 	
		vector< pair< string, string > >::iterator it = problem.culprits.begin();
		it != problem.culprits.end();
		++it
	) {
		std::string key( PREFIX );
		key.append( it->first );
		this->culprit( key, it->second );
	}
	return *this;
}

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

static const char * ERR_MSG = "Error";


void Problem::report() {

	size_t width = strlen( ERR_MSG );	
	for ( 	
		vector< pair< string, string > >::iterator it = this->culprits.begin();
		it != this->culprits.end();
		++it
	) {
		size_t n = it->first.size();
		if ( n > width ) width = n;
	}
	

	cerr << endl;
	cerr << SYS_MSG_PREFIX << left << setw( width ) << ERR_MSG << " : " << this->message << endl;
	for ( 	
		vector< pair< string, string > >::iterator it = this->culprits.begin();
		it != this->culprits.end();
		++it
	) {
		cerr << SYS_MSG_PREFIX << left << setw( width ) << it->first << " : " << it->second << endl;
	}
	cerr << endl;
}

void Problem::gnxReport() {
	cout << "<problem message=\"";
	mnxRenderText( cout, this->message );
	cout << "\" severity=\"";
	mnxRenderText( cout, this->severity() );
	cout << "\"" << ">";
	for ( 	
		vector< pair< string, string > >::iterator it = this->culprits.begin();
		it != this->culprits.end();
		++it
	) {
		cout << "<culprit name=\"";
		mnxRenderText( cout, it->first );
		cout << "\" value=\"";
		mnxRenderText( it->second );
		cout << "\" />";
	}
	cout << "</problem>" << endl;	
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
