#include "instruction_set.hpp"
#include "mishap.hpp"


void enter_error( int B, int A ) {
	throw Mishap( "Wrong number of arguments" ).culprit( "Found", "" + B ).culprit( "Wanted", "" + A );
}

void call_error( Ref r ) {
	throw Mishap( "Function needed - got something else" );
}


#include "instruction_set.cpp.auto"
