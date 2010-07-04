#include "instruction_set.hpp"
#include "mishap.hpp"

/*void results_error( int d ) {
	throw Mishap( "Wrong number of results (needed 1)" ).culprit( "#results", "" + d );
}

void arith_error( int x, int y ) {
	throw Mishap( "Arithmetic error" ).culprit( "Argument1", "" + x ).culprit( "Argument2", "" + y );
}

void arith_error( int x ) {
	throw Mishap( "Arithmetic error" ).culprit( "Argument", "" + x );
}

void normal_exit() {
	throw NormalExit();
}*/

void enter_error( int B, int A ) {
	throw Mishap( "Wrong number of arguments" ).culprit( "Found", "" + B ).culprit( "Wanted", "" + A );
}

void call_error( Ref r ) {
	throw Mishap( "Function needed - got something else" );
}


#include "instruction_set.cpp.auto"