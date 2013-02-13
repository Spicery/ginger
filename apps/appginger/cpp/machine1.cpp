#include "machine1.hpp"

//	Now source the auto-generated C++ file. 
#include "instructions_context.hpp"


typedef Ref *SpecialFn( Ref *pc, Machine vm );
typedef SpecialFn *Special;

Machine1::Machine1( AppContext * g ) :
	MachineClass( g )
{
}

/*static void sanity_check( Ref * func_obj_K ) {
	if ( func_obj_K != 0 && *func_obj_K != sysFunctionKey ) throw;
}*/
	

void Machine1::execute( Ref r ) {
	Ref * PC = this->setUpPC( r );
	for (;;) {			
		//sanity_check( this->func_of_program_counter ); // debug
		Special fn = (Special)( *PC );
		#ifdef DBG_MACHINE
			special_show( PC );
		#endif
		PC = fn( PC, this );
	}
}

#include "machine1.cpp.auto"
