#include "instruction_set.hpp"
#include "mishap.hpp"

void enter_error( int B, int A ) {
	reset( "Wrong number of arguments (found %d wanted %d)", B, A );
}

void call_error( Ref r ) {
	reset( "Function needed - got something else" );
}

#include "../automatic/instruction_set.cpp.auto"