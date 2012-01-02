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
//#include "read_xml.hpp"
//#include "codegen.hpp"
#include "machine.hpp"
#include "rcep.hpp"
//#include "vmi.hpp"
#include "mishap.hpp"
//#include "lift.hpp"
//#include "resolve.hpp"

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

void RCEP::execGnx( shared< Ginger::Mnx > mnx, std::ostream & output ) {
	Machine vm = this->getMachine();
    CodeGen codegen;
    Ref r;
    
	try {
	    codegen = vm->codegen();
	    codegen->vmiFUNCTION( 0, 0 );
	    codegen->vmiENTER();
	    LabelClass retn( codegen, true );
        codegen->compileGnx( mnx, &retn );
        retn.labelSet();
	    codegen->vmiRETURN();				//	TODO: We might be able to eliminate this.
	    r = codegen->vmiENDFUNCTION();
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
		//	Do nothing! Just exit nicely.	
	}
    output.flush();
}

bool RCEP::unsafe_read_comp_exec_print( istream & input, std::ostream & output ) {
	Machine vm = this->getMachine();
    CodeGen codegen;
    Ref r;
    //Term term;
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
		
		//	DEBUG.
		#ifdef DBG_RCEP
			cerr << "RCEP EXPRESSION: ";
			mnx->render();
			cout << endl;
		#endif
		
		#ifdef DBG_RCEP
			cerr << "Planting" << endl;
	    #endif

	    codegen = vm->codegen();
	    codegen->vmiFUNCTION( 0, 0 );
	    codegen->vmiENTER();
	    LabelClass retn( codegen, true );
        codegen->compileGnx( mnx, &retn );
        retn.labelSet();
	    codegen->vmiRETURN();				//	TODO: We might be able to eliminate this.
	    r = codegen->vmiENDFUNCTION();
	    start = clock();
	    
		#ifdef DBG_RCEP
			cerr << "Queuing up" << endl;
		#endif
	    
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
			Machine vm = this->getMachine();
			vm->resetMachine();
		}
	}
}

void RCEP::printResults( std::ostream & output, float time_taken ) {
	this->getMachine()->printResults( output, time_taken );
}
