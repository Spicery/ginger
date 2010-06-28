#include <iostream>
using namespace std;

#include "common.hpp"
#include "read_xml.hpp"
#include "plant.hpp"
#include "machine.hpp"
#include "print.hpp"
#include "rcep.hpp"
#include "vmi.hpp"
#include "mishap.hpp"
#include "lift.hpp"

#include <setjmp.h>
#include <time.h>

#include <cstdio>

//#define DBG_RCEP


bool read_comp_exec_print( Machine vm, istream & input ) {
    Plant plant;
    Ref r;
    Term term;
	volatile clock_t start, finish;
	//Item it;
	//ReadStateClass read_state( ifact );
	
	// Sort out ifact later
	ReadXmlClass read_xml( input );

#ifdef DBG_RCEP
	printf( "Entering Read-Compile-Eval-Print loop\n" );
	fflush( stdout );
#endif

	try {
        term = read_xml.readElement();
        
        if ( not term ) return false;

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
        plant->compileTerm( term );
	    vmiRETURN( plant );
	    r = vmiENDFUNCTION( plant );
	    start = clock();
    	vm->execute( r );
	//} catch ( Mishap & m ) {
		//m.report();
		//throw;
	} catch ( NormalExit ) {
	}

	finish = clock();
    print_results( vm, ( ( float )( finish - start ) ) / CLOCKS_PER_SEC );
    fflush( stdout );
	fflush( stderr );
	
	return true;
}

