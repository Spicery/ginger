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

//  Standard C++ libraries.
#include <iostream>

//  Standard C Libraries.
#include <unistd.h>

//  Include libgng libraries.
#include "mnx.hpp"

//  Our own header file.
#include "gvm.hpp"

//  Ginger VM header files (required for heap crawling)    
#include "garbagecollect.hpp"
#include "machine.hpp"
#include "heapcrawl.hpp"
#include "cagecrawl.hpp"
//#include "sys.hpp"
#include "key.hpp"
#include "garbagecollect.hpp"


using namespace std;
using namespace Ginger;

void GCData::Count::show() {
    cout << "Number of objects copied             : " << this->objects << endl;
    cout << "Number of function objects copied    : " << this->fn_objs << endl;
    cout << "Number of vector objects copied      : " << this->vec_objs << endl;
    cout << "Number of record objects copied      : " << this->rec_objs << endl;
    cout << "Number of user objects copied        : " << this->user_objs << endl;
    cout << "Number of string objects copied      : " << this->string_objs << endl;
    cout << "Number of word-record objects copied : " << this->wrecord_objs << endl;           
}

void GCData::show() {
    double milli_secs = 1000.0 * double( this->end_time - this->start_time ) / double( CLOCKS_PER_SEC );
    std::cout.unsetf( std::ios::floatfield );           
    std::cout.precision( 3 );
    std::cout.setf( ios::showpoint );
    std::cout.setf( ios::fixed );
    cout << "Garbage collection time: " << milli_secs << "ms" << endl;
    std::cout.unsetf( std::ios::floatfield );           
    std::cout.unsetf( std::ios::showpoint );               
    std::cout.unsetf( std::ios::fixed );    

    this->counts.show();
}


class TrackGarbageCollection : public GCTracker {
    typedef void * Ref;
private:
    GCData & gcdata;

public:
    TrackGarbageCollection( GCData & _gcdata ) : gcdata( _gcdata ) {}

public:
    virtual void startGarbageCollection() {
        this->gcdata.start_time = clock();
    }

    virtual void endGarbageCollection() {
        this->gcdata.end_time = clock();
    }
     virtual void pickedObjectToCopy( Ref * obj_K ) {
        this->gcdata.counts.objects += 1;
    }
    virtual void startFnObj() {
        this->gcdata.counts.fn_objs += 1;
    }
    virtual void startVector( Ref * obj_K ) {
        this->gcdata.counts.vec_objs += 1;
    }
    virtual void startRecord( Ref * obj_K ) {
        this->gcdata.counts.rec_objs += 1;
    }
    virtual void startInstance( Ref * obj_K ) {
        this->gcdata.counts.user_objs += 1;
    }
    virtual void atString( Ref * obj_K ) {
        this->gcdata.counts.string_objs += 1;
    }
    virtual void atWRecord( Ref * obj_K ) {
        this->gcdata.counts.wrecord_objs += 1;
    }
};



VirtualMachine * VirtualMachineFactory::newVirtualMachine() {
    ::MachineClass * m = this->app_context->newMachine();
    if ( m == 0 ) {
        cerr << "Cannot allocate virtual machine" << endl;
        exit( EXIT_FAILURE );
    }
    return new VirtualMachine( this->app_context, m );
}

void VirtualMachine::garbageCollect( GCData & gcdata ) {
    TrackGarbageCollection tracker( gcdata );
    sysQuiescentGarbageCollect( this->machine, &tracker );
}


void VirtualMachine::execGnx( shared< Mnx > gnx ) {
    try {
        CodeGen codegen = this->machine->codegen();
        codegen->vmiFUNCTION( "Top Level Loop", 0, 0 );
        codegen->vmiENTER();
        LabelClass retn( codegen, true );
        codegen->compileGnx( gnx, &retn );
        retn.labelSet();
        codegen->vmiRETURN();                               //      TODO: We might be able to eliminate this.
        Ref r = codegen->vmiENDFUNCTION();
        this->machine->addToQueue( r );
        this->machine->executeQueue( false );
        //  We should never normally reach here.
        cerr << "Somehow the GVM returned rather than exited with a throw! Wrong!" << endl;
        exit( EXIT_FAILURE );   //  Probably need to force a continuation to permit debugging.

    } catch ( Ginger::NormalExit ) {
        //      Do nothing! Just exit nicely.
        //  TODO: ELiminate this message.
        cerr << "Note: Virtual machine stopped normally" << endl;
    }
 }

ptrdiff_t VirtualMachine::stackLength() {
    return this->machine->stackLength();
}

Cell VirtualMachine::peek( int n ) {
    return Cell( this->machine->fastPeek( n ) );
}

vector< Cell > VirtualMachine::stack() {
    vector< Cell > sofar;
    for ( int i = this->machine->stackLength() - 1; i >= 0; i -= 1 ) {
        sofar.push_back( this->peek( i ) );
    }
    return sofar;
}

void VirtualMachine::execCode( shared< Mnx > code ) {
    CodeGen codegen = this->machine->codegen();
    codegen->vmiFUNCTION( 0, 0 );
    MnxChildIterator it( code ); 
    while( it.hasNext() ) {
        shared< Mnx > & c = it.next();
        if ( c->hasAttribute( "name" ) ) {
            const std::string name = c->attribute( "name" );
            Instruction vmc = this->machine->instructionSet().findInstruction( name );
            codegen->vmiINSTRUCTION( vmc );
            MnxChildIterator args( c );
            while ( args.hasNext() ) {
                shared< Mnx > & arg = args.next();
                if ( arg->hasName( "constant" ) ) {
                    codegen->emitRef( codegen->calcConstant( arg ) );
                } else {
                    throw Ginger::Mishap( "Unexpected arg" );
                }
            }
        } else {
            throw Ginger::Mishap( "Instruction missing name" );
        }
    }
    codegen->vmiINSTRUCTION( vmc_reset );
    Ref r = codegen->vmiENDFUNCTION();
    this->machine->addToQueue( r );
    try {
        this->machine->executeQueue( false );
    } catch ( Ginger::NormalExit ) {
        cerr << "Note: VM stopped normally" << endl;
    } catch ( Ginger::Mishap m ) {
        m.report();
    }
}

void VirtualMachine::clearStack() {
    this->machine->clearStack();
}


void VirtualMachine::crawlHeap( HeapTracer & tracer ) {
    HeapCrawl hcrawl( this->machine->heap() );
    for (;;) {
        CageClass * cage = hcrawl.next();
        if ( not cage ) break; 
        tracer.startCage();

        tracer.serialNumberCage( cage->serialNumber() );
        tracer.usedCage( cage->nboxesInUse() );
        tracer.capacityCage( cage->capacity() );

        //out << "  Cage[" << cage->serialNumber() << "] at " << (unsigned long)cage << " with " << cage->nboxesInUse() << "/" << cage->capacity() << " cells" << endl;

        tracer.startCrawl();
        CageCrawl ccrawl( cage );
        for (;;) {
            Ref * key = ccrawl.next();
            if ( not key ) break;
            tracer.atObject( HeapObject( key ) );
            //out << hex << "    @" << (unsigned long) key << " : " << keyName( *key ) << endl;
            //out << "Value = ";
            //refPtrPrint( out, key );
            //out << endl;
        }
        tracer.endCrawl();

        tracer.endCage();
    }
}




