/******************************************************************************\
	Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
	of the GNU General Public License. This file is part of AppGinger.

    AppGinger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AppGinger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AppGinger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

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
//#define DBG_CRAWL

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
		out << "  Cage at " << (unsigned long)cage << " with " << cage->nboxesInUse() << " cells" << endl;

		CageCrawl ccrawl( cage );
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

bool unsafe_read_comp_exec_print( Machine vm, istream & input ) {
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
    		//cout << "Crawling the heap ... ";
    		//cout.flush();
    		crawl( vm );
    		//cout << "done" << endl;
    	#endif
	} catch ( NormalExit ) {
     	#ifdef DBG_CRAWL
    		//cout << "Crawling the heap ... ";
    		//cout.flush();
    		crawl( vm );
    		//cout << "done" << endl;
    	#endif
	}

	finish = clock();
    vm->print_results( static_cast<float>( finish - start ) / CLOCKS_PER_SEC );
    fflush( stdout );
	fflush( stderr );
	
	return true;
}

bool read_comp_exec_print( Machine vm, istream & input ) {
	try {
		return unsafe_read_comp_exec_print( vm, input );
	} catch ( Mishap & m ) {
		m.report();
		throw;
	} 
}

