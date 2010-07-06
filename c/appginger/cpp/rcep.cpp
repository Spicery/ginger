#include <iostream>
#include <fstream>
using namespace std;

#include "common.hpp"
#include "read_xml.hpp"
#include "plant.hpp"
#include "machine.hpp"
#include "rcep.hpp"
#include "vmi.hpp"
#include "mishap.hpp"
#include "lift.hpp"

#include <setjmp.h>
#include <time.h>

#include <cstdio>

//#define DBG_RCEP
#define DBG_CRAWL

#ifdef DBG_CRAWL

#include "heapcrawl.hpp"
#include "cagecrawl.hpp"
#include "sys.hpp"
#include "key.hpp"
	
static void crawl( Machine vm ) {
	ofstream out( "crawl.log" );
	out << "Heap Crawl" << endl;
	
	HeapCrawl hcrawl( vm->heap() );
	for (;;) {
		CageClass * cage = hcrawl.next();
		if ( not cage ) break; 
		out << "  Cage at " << (unsigned long)cage << " with " << cage->ncells() << " cells" << endl;

		CageCrawl ccrawl( *cage );
		for (;;) {
			Ref * key = ccrawl.next();
			if ( not key ) break;
			out << hex << "    @" << (unsigned long) key << " : " << keyName( *key ) << endl;
			//out << "Value = ";
			//refPtrPrint( out, key );
			//out << endl;
		}
		
		out.flush();
	}
	out.close();
}
	
#endif

bool read_comp_exec_print( Machine vm, istream & input ) {
    Plant plant;
    Ref r;
    Term term;
	volatile clock_t start, finish;
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
     	#ifdef DBG_CRAWL
    		cout << "Crawling the heap ... ";
    		cout.flush();
    		crawl( vm );
    		cout << "done" << endl;
    	#endif
	//} catch ( Mishap & m ) {
		//m.report();
		//throw;
	} catch ( NormalExit ) {
     	#ifdef DBG_CRAWL
    		cout << "Crawling the heap ... ";
    		cout.flush();
    		crawl( vm );
    		cout << "done" << endl;
    	#endif
	}

	finish = clock();
    vm->print_results( static_cast<float>( finish - start ) / CLOCKS_PER_SEC );
    fflush( stdout );
	fflush( stderr );
	
	return true;
}

