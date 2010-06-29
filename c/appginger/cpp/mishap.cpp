#include <iostream>
using namespace std;


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "mishap.hpp"

//jmp_buf mishap_jump_buffer;

void mishap( const char *msg, ... ) {
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
}

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

Mishap & Mishap::culprit( const std::string reason, const std::string arg ) {
	this->culprits.push_back( pair< const string, const string >( reason, arg ) );
	return *this;
}

Mishap & Mishap::culprit( const std::string arg ) {
	this->culprits.push_back( std::pair< const string, const string >( "Argument", arg ) );
	return *this;
}

void Mishap::report() {
	cerr << "MISHAP: " << this->message << endl;
	for ( 	
		vector< pair< string, string > >::iterator it = this->culprits.begin();
		it != this->culprits.end();
		++it
	) {
		cerr << it->first << " : " << it->second << endl;
	}
}
	
Mishap::Mishap( const std::string & msg ) : 
	message( msg ) 
{
}
