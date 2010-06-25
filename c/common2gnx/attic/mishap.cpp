#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "mishap.hpp"

jmp_buf mishap_jump_buffer;

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

void reset( const char *msg, ... ) {
    va_list args;
    va_start( args, msg );
    fprintf( stderr, "\nRESET: " );
    vfprintf( stderr, msg, args );
    fprintf( stderr, "\n\n" );
    va_end( args );
	longjmp( mishap_jump_buffer, -1 );
}

void this_never_happens() {
	mishap( "This never happens" );
}

void to_be_done( charseq msg ) {
	mishap( "To be done: %s", msg );
}
