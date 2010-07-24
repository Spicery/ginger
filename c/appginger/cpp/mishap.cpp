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
using namespace std;


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "mishap.hpp"

//jmp_buf mishap_jump_buffer;

/*void mishap( const char *msg, ... ) {
    va_list args;
    va_start( args, msg );
    fprintf( stderr, "Mishap: " );
    vfprintf( stderr, msg, args );
    fprintf( stderr, "\n" );
    va_end( args );
    exit( EXIT_FAILURE );
}

void warning( const char *msg, ... ) {
    va_list args;
    va_start( args, msg );
    fprintf( stderr, "Warning: " );
    vfprintf( stderr, msg, args );
    fprintf( stderr, "\n" );
    va_end( args );
}*/

/*void reset( const char *msg, ... ) {
    va_list args;
    va_start( args, msg );
    fprintf( stderr, "\nRESET: " );
    vfprintf( stderr, msg, args );
    fprintf( stderr, "\n\n" );
    va_end( args );
	longjmp( mishap_jump_buffer, -1 );
}*/

void this_never_happens() {
	throw Mishap( "This never happens" );
}

void to_be_done( charseq msg ) {
	throw Mishap( "To be done" ).culprit( "Message", msg );
}

Problem & Problem::culprit( const std::string reason, const std::string arg ) {
	this->culprits.push_back( pair< const string, const string >( reason, arg ) );
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

