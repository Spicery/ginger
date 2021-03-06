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

#include <stdarg.h>
#include <stdlib.h>

#include "mnx.hpp"
#include "mishap.hpp"
#include "gngversion.hpp"

namespace Ginger {
using namespace std;

static constexpr const char * const HINT = "hint";

#define PREFIX "    "
#define ORIGIN ( PREFIX "Cause" )

Mishap & Mishap::cause( Mishap & problem ) {
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

Mishap & Mishap::hint( const std::string & hint_string ) {
	this->culprit( HINT, hint_string );
	return *this;
}

Mishap & Mishap::culprit( const std::string reason, const std::string arg ) {
	this->culprits.push_back( pair< const string, const string >( reason, arg ) );
	return *this;
}

Mishap & Mishap::culprit( const char * reason, const std::string arg ) {
	this->culprits.push_back( pair< const string, const string >( reason, arg ) );
	return *this;
}

Mishap & Mishap::culprit( const char * reason, const char * arg ) {
	this->culprits.push_back( pair< const string, const string >( reason, arg ) );
	return *this;
}

Mishap & Mishap::culprit( const std::string reason, const signed long N ) {
	std::ostringstream s;
	s << N;
	const std::string result( s.str() );
	this->culprits.push_back( pair< const string, const string >( reason, result ) );
	return *this;
}

Mishap & Mishap::culprit( const std::string reason, const unsigned long N ) {
	std::ostringstream s;
	s << N;
	const std::string result( s.str() );
	this->culprits.push_back( pair< const string, const string >( reason, result ) );
	return *this;
}

Mishap & Mishap::culprit( const std::string reason, const signed int N ) {
	std::ostringstream s;
	s << N;
	const std::string result( s.str() );
	this->culprits.push_back( pair< const string, const string >( reason, result ) );
	return *this;
}

Mishap & Mishap::culprit( const std::string reason, const unsigned int N ) {
	std::ostringstream s;
	s << N;
	const std::string result( s.str() );
	this->culprits.push_back( pair< const string, const string >( reason, result ) );
	return *this;
}

Mishap & Mishap::culprit( const std::string reason, const double N ) {
	std::ostringstream s;
	s << N;
	const std::string result( s.str() );
	this->culprits.push_back( pair< const string, const string >( reason, result ) );
	return *this;
}

Mishap & Mishap::culprit( const std::string reason, const signed char N ) {
	std::ostringstream s;
	s << N;
	const std::string result( s.str() );
	this->culprits.push_back( pair< const string, const string >( reason, result ) );
	return *this;
}

Mishap & Mishap::culprit( const std::string reason, const unsigned char N ) {
	std::ostringstream s;
	s << N;
	const std::string result( s.str() );
	this->culprits.push_back( pair< const string, const string >( reason, result ) );
	return *this;
}

Mishap & Mishap::culprit( const std::string arg ) {
	this->culprits.push_back( std::pair< const string, const string >( "Argument", arg ) );
	return *this;
}

std::string Mishap::category() const {
	switch ( this->mishap_category[0] ) {
		case 'R':
			return "Mishap";
		case 'S':
			return "Panic";
		case 'C':
			return "Compilation Error";
		default:
			return "Error";
	}
}


//static const char * ERR_MSG = "Error";


void Mishap::report() {

	size_t width = this->category().size();
	for ( 	
		vector< pair< string, string > >::iterator it = this->culprits.begin();
		it != this->culprits.end();
		++it
	) {
		size_t n = it->first.size();
		if ( n > width ) width = n;
	}
	

	cerr << endl;
	cerr << SYS_MSG_PREFIX << left << setw( width ) << this->category() << " : " << this->message << endl;
	for ( 	
		vector< pair< string, string > >::iterator it = this->culprits.begin();
		it != this->culprits.end();
		++it
	) {
		cerr << SYS_MSG_PREFIX << left << setw( width ) << it->first << " : " << it->second << endl;
	}
	cerr << endl;
}

void Mishap::gnxReport() {
	cout << "<problem message=\"";
	mnxRenderText( cout, this->message );
	cout << "\" category=\"";
	mnxRenderText( cout, this->mishap_category );
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

std::string Mishap::getMessage() {
	return this->message;
}

std::pair< std::string, std::string > & Mishap::getCulprit( int n ) {
	return this->culprits[ n ];
}

int Mishap::getCount() {
	return this->culprits.size();
}

} // namespace Ginger
