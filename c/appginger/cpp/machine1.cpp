#include "machine1.hpp"

//	Now source the auto-generated C++ file. The 
//	header files are provided here (for no very
//	good reason.)
#include "machine.hpp"
#include "key.hpp"
#include "sys.hpp"
#include "mishap.hpp"
#include "appginger.hpp"


typedef Ref *SpecialFn( Ref *pc, Machine vm );
typedef SpecialFn *Special;

Machine1::Machine1( AppGinger & g ) :
	MachineClass( g )
{
}

void Machine1::execute( Ref r ) {
	Ref * PC = this->setUpPC( r );

	//	printf( "Should return to %s\n", special_name( *(Special **)(*VM->sp) ) );
	for (;;) {
		Special fn = (Special)( *PC );
		#ifdef DBG_MACHINE
			special_show( PC );
		#endif
		PC = fn( PC, this );
	}
}

#include "machine1.cpp.auto"