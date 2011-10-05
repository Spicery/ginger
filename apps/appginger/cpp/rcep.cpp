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

#include <setjmp.h>
#include <time.h>

#include <cstdio>

#include "mnx.hpp"

#include "common.hpp"
#include "read_xml.hpp"
#include "plant.hpp"
#include "machine.hpp"
#include "rcep.hpp"
#include "vmi.hpp"
#include "mishap.hpp"
#include "lift.hpp"
#include "resolve.hpp"

using namespace std;


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

int RCEP::level = 0;


bool RCEP::unsafe_read_comp_exec_print( istream & input, std::ostream & output ) {
	Machine vm = this->getMachine();
    Plant plant;
    Ref r;
    Term term;
	volatile clock_t start, finish;
	//ReadXmlClass read_xml( input );
	Ginger::MnxReader read_xml( input );

#ifdef DBG_RCEP
	printf( "Entering Read-Compile-Eval-Print loop\n" );
	fflush( stdout );
#endif

	try {
		shared< Ginger::Mnx > mnx( read_xml.readMnx() );
		if ( not mnx ) return false;
		term = mnxToTerm( mnx );
        //term = read_xml.readElement();
        
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
	    vm->addToQueue( r );
	    if ( this->isTopLevel() ) {
        	#ifdef DBG_RCEP
        		cerr << "About to execute queue" << endl;
        	#endif
			vm->executeQueue();
	    } else {
        	#ifdef DBG_RCEP
        		cerr << "Not top level" << endl;
        	#endif
	    }
	} catch ( Ginger::NormalExit ) {
	}
	#ifdef DBG_CRAWL
		crawl( vm, "before.log" );
		sysQuiescentGarbageCollect( vm );
		crawl( vm, "after.log" );
	#endif


	finish = clock();
	if ( this->printing ) {
	    vm->printResults( output, static_cast<float>( finish - start ) / CLOCKS_PER_SEC );
	}
    fflush( stdout );
	fflush( stderr );
	
	return true;
}

bool RCEP::read_comp_exec_print( istream & input, std::ostream & output ) {
	for (;;) {
		try {
			return unsafe_read_comp_exec_print( input, output );
		} catch ( Ginger::Mishap & m ) {
			m.report();
		} catch ( Ginger::SystemError & m ) {
			m.report();
			throw m;
		}
	}
}


