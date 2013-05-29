#include "instruction_set.hpp"
#include "mishap.hpp"

namespace Ginger {

void enter_error( int B, int A ) {
	throw Ginger::Mishap( "Wrong number of arguments" ).culprit( "Found", "" + B ).culprit( "Wanted", "" + A );
}

void call_error( Ref r ) {
	throw Ginger::Mishap( "Function needed - got something else" );
}

void invoke_error() {
	throw Ginger::Mishap( "Error in method dispatch" );
}


#include "instruction_set.cpp.auto"

bool InstructionSet::isByPassInstruction( Ref * pc ) const {
	return *pc == spc_bypass;
}

} // namespace Ginger