#include "common.hpp"
#include "item_factory.hpp"
#include "read_expr.hpp"
#include "plant.hpp"
#include "machine.hpp"
#include "print.hpp"
#include "rcep.hpp"
#include "vmi.hpp"
#include "mishap.hpp"
#include "lift.hpp"

#include <setjmp.h>
#include <time.h>

//#define DBG_RCEP


bool read_comp_exec_print( Machine vm, ItemFactory ifact ) {
    Plant plant;
    Ref r;
    Term term;
	volatile clock_t start, finish;
	Item it;
	ReadStateClass read_state( ifact );

#ifdef DBG_RCEP
	printf( "Entering Read-Compile-Eval-Print loop\n" );
	fflush( stdout );
#endif

    it = item_factory_peek( ifact );
	
	if ( it->role == EofRole ) return false;

	if ( setjmp( mishap_jump_buffer ) == 0 ) {
        term = read_state.read_expr();

        #ifdef DBG_RCEP
			fprintf( stderr, "Before lifting\n" );
        	term_print( term );
			fprintf( stderr, "Lifting ...\n" );
			fflush( stderr );
        #endif

		term = lift_term( vm->dict(), term );

        #ifdef DBG_RCEP
			fprintf( stderr, "After lifting\n" );
        	term_print( term );
			fprintf( stderr, "---\n" );
			fflush( stderr );
        #endif

	    plant = vm->plant();
	    vmiFUNCTION( plant, 0, 0 );
	    vmiENTER( plant );
        plant_term( plant, term );
	    vmiRETURN( plant );
	    r = vmiENDFUNCTION( plant );
	    start = clock();
    	vm->execute( r );
	} else {
		item_factory_reset( ifact );
		vm->resetMachine();
	}

	finish = clock();
    print_results( vm, ( ( float )( finish - start ) ) / CLOCKS_PER_SEC );
    fflush( stdout );
	fflush( stderr );
	
	return true;
}

