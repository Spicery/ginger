#include "machine2.hpp"


//	Now source the auto-generated C++ file. The 
//	header files are provided here (for no very
//	good reason.)
#include "machine.hpp"
#include "key.hpp"
#include "sys.hpp"
#include "mishap.hpp"

Ref *pc;
Machine vm;


typedef void SpecialFn( void );
typedef SpecialFn *Special;

void Machine2::execute( Ref r ) {
	pc = this->setUpPC( r );
	vm = this;

	//	printf( "Should return to %s\n", special_name( *(Special **)(*VM->sp) ) );
	for (;;) {		
		Special fn = (Special)( *pc );
		#ifdef DBG_MACHINE
			special_show( pc );
		#endif
		fn();
	}
}

#include "../automatic/machine2.cpp.auto"



