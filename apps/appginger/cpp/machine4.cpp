#include "machine4.hpp"
#include "instructions_context.hpp"

using namespace Ginger;

#define FREEZE
#define MELT
#define vm 		this
#define VMLINK 	( vm->link )
#define VMSP	( vm->sp )
#define VMVP	( vm->vp )
#define VMCOUNT	( vm->count )
#define VMLINKFUNC 	( vm->func_of_link )
#define VMPCFUNC	( vm->func_of_program_counter )
#define RETURN( e ) 	{ pc = e; goto execute_loop; }

typedef Instruction Special;

Machine4::Machine4( AppContext & g ) : MachineClass( g ) {}

void Machine4::execute( Ref r ) {
	Ref * pc = this->setUpPC( r );
	execute_loop: {
		Special code = *reinterpret_cast< Special * >( pc );
		switch ( code ) {
			#include "machine4.cpp.auto"
			default: throw SystemError( "Invalid instruction" );
		}
		throw SystemError( "Instructions may not fall thru" );
	}
}


