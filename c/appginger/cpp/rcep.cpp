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

#include "debug.hpp"

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
#include "resolve.hpp"

#include <setjmp.h>
#include <time.h>

#include <cstdio>

//#define DBG_RCEP


#ifdef DBG_CRAWL

#include "heapcrawl.hpp"
#include "cagecrawl.hpp"
#include "sys.hpp"
#include "key.hpp"
#include "garbagecollect.hpp"
	
static void crawl( Machine vm, const char * logfname ) {
	ofstream out( logfname );
	out << "Heap Crawl" << endl;
	
	HeapCrawl hcrawl( vm->heap() );
	for (;;) {
		CageClass * cage = hcrawl.next();
		if ( not cage ) break; 
		out << "  Cage[" << cage->serialNumber() << "] at " << (unsigned long)cage << " with " << cage->nboxesInUse() << "/" << cage->capacity() << " cells" << endl;

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

bool RCEP::unsafe_read_comp_exec_print( Machine vm, istream & input ) {
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

		bool needs_lifting;
		term = resolveTerm( this->current_package, term, needs_lifting );
		
		#ifdef DBG_LIFTING
			cerr << "Lifting needed? " << needs_lifting << endl;
		#endif
		if ( needs_lifting ) {
			term = liftTerm( this->current_package, term );	
		}

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
	} catch ( NormalExit ) {
	}
	#ifdef DBG_CRAWL
		crawl( vm, "before.log" );
		sysQuiescentGarbageCollect( vm );
		crawl( vm, "after.log" );
	#endif


	finish = clock();
    vm->print_results( static_cast<float>( finish - start ) / CLOCKS_PER_SEC );
    fflush( stdout );
	fflush( stderr );
	
	return true;
}

bool RCEP::read_comp_exec_print( Machine vm, istream & input ) {
	for (;;) {
		try {
			return unsafe_read_comp_exec_print( vm, input );
		} catch ( Mishap & m ) {
			m.report();
		} catch ( SystemError & m ) {
			m.report();
			throw;
		} 
	}
}


