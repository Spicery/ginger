/******************************************************************************\
	Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
	of the GNU General Public License. This file is part of Ginger.

    Ginger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Ginger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Ginger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

#include "debug.hpp"

#include <iostream>
#include <fstream>

#include <setjmp.h>
#include <time.h>

#include "debug.hpp"

#include "mnx.hpp"

#include "common.hpp"
#include "machine.hpp"
#include "gnxconstants.hpp"
#include "rcep.hpp"
#include "mishap.hpp"
#include "simplify.hpp"
#include "compile.hpp"

namespace Ginger {
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
	    codegen->vmiFUNCTION( "Top Level Loop", 0, 0 );
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


Ref RCEP::compileTopLevel( shared< Ginger::Mnx > mnx ) {
	Machine vm = this->getMachine();
    CodeGen codegen = vm->codegen();
	if ( vm->getAppContext().getFn2Code() ) {
	    MnxBuilder b;
	    b.start( GNX_FN );
	    b.start( GNX_SEQ );
	    b.end();
	    b.add( mnx );
	    b.end();
	    shared< Mnx > top_level = b.build();
		Simplify simplifier( vm->getAppContext(), this->currentPackage() );
		top_level = simplifier.simplify( top_level );
		Compile compiler( vm->getAppContext() );
		top_level = compiler.compile( top_level );
		if ( top_level->name() == GNX_FN_CODE ) {
			return codegen->compileGnxFnCodeStandalone( top_level );
		}
	}

	//	Fallback code in case the prototype compiler went wrong.
	Simplify simplifier( vm->getAppContext(), this->currentPackage() );
	mnx = simplifier.simplify( mnx );
	if ( vm->getAppContext().getFn2Code() ) {
 		Compile compiler( vm->getAppContext() );
		mnx = compiler.compile( mnx );
	}
    codegen->vmiFUNCTION( "Top level loop", 0, 0 );
    codegen->vmiENTER();
    LabelClass retn( codegen, true );
    codegen->compileGnx( mnx, &retn );
    retn.labelSet();
    codegen->vmiRETURN();				//	TODO: We might be able to eliminate this.
	return codegen->vmiENDFUNCTION();
}

bool RCEP::mainloop( MnxRepeater & mnxrep, std::ostream & output ) {
	Machine vm = this->getMachine();
    // CodeGen codegen;
    //Term term;
	volatile clock_t start, finish;
	//ReadXmlClass read_xml( input );

	try {
		//	TODO: Fix the freezing in of the package!
		//	NOTE: If the current package changes then it is vital to
		//	replace the simplifier. Really this is frozen into the wrong place!!
		shared< Ginger::Mnx > mnx( mnxrep.nextMnx() );
		if ( not mnx ) return false;

		//	DEBUG.
		#ifdef DBG_RCEP
			cerr << "RCEP expression to compile" << endl;
			cerr << "  [[";
			mnx->render( cerr );
			cerr << "]]" << endl;
		#endif
		
		#ifdef DBG_RCEP
			cerr << "Planting" << endl;
	    #endif

	    // codegen = vm->codegen();
		Ref r = this->compileTopLevel( mnx );
		// if ( vm->getAppContext().getFn2Code() ) {
		//     MnxBuilder b;
		//     b.start( GNX_FN );
		//     b.start( GNX_SEQ );
		//     b.end();
		//     b.add( mnx );
		//     b.end();
		//     shared< Mnx > top_level = b.build();
		// 	Simplify simplifier( vm->getAppContext(), this->currentPackage() );
		// 	top_level = simplifier.simplify( top_level );
	 // 		Compile compiler( vm->getAppContext() );
		// 	top_level = compiler.compile( top_level );
	 // 		if ( top_level->name() == GNX_FN_CODE ) {
	 // 			r = codegen->compileGnxFnCodeStandalone( top_level );
	 // 		}
	 // 	}
	 // 	if ( r == 0 ) {
		// 	Simplify simplifier( vm->getAppContext(), this->currentPackage() );
		// 	mnx = simplifier.simplify( mnx );
		// 	if ( vm->getAppContext().getFn2Code() ) {
		//  		Compile compiler( vm->getAppContext() );
		// 		mnx = compiler.compile( mnx );
		// 	}
		//     codegen->vmiFUNCTION( "Top level loop", 0, 0 );
		//     codegen->vmiENTER();
		//     LabelClass retn( codegen, true );
	 //        codegen->compileGnx( mnx, &retn );
	 //        retn.labelSet();
		//     codegen->vmiRETURN();				//	TODO: We might be able to eliminate this.
		//     r = codegen->vmiENDFUNCTION();
		// }
	    start = clock();
	    
		#ifdef DBG_RCEP
			cerr << "Planting done - Queuing up" << endl;
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

bool RCEP::unsafe_read_comp_exec_print( istream & input, std::ostream & output ) {
	Ginger::MnxReader read_xml( input );
	return this->mainloop( read_xml, output );
}

bool RCEP::read_comp_exec_print( istream & input, std::ostream & output ) {
	Ginger::MnxReader read_xml( input );
	for (;;) {
		try {
			return this->mainloop( read_xml, output );
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

} // namespace Ginger
