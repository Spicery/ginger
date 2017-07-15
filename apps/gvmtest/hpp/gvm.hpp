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

#ifndef GINGER_GVM_HPP
#define GINGER_GVM_HPP

#include <vector>

#include "shared.hpp"
#include "appcontext.hpp"
#include "machine.hpp"

//  Additional libraries we need to program the GVM.
#include "cell.hpp"
#include "gctracker.hpp"

namespace Ginger {

class GCData {
public:
    clock_t start_time;
    clock_t end_time;

    struct Count {
        long objects;
        long fn_objs;
        long vec_objs;
        long rec_objs;
        long user_objs;
        long string_objs;
        long wrecord_objs;

        Count() :
            objects( 0 ),
            fn_objs( 0 ),
            vec_objs( 0 ),
            rec_objs( 0 ),
            user_objs( 0 ),
            string_objs( 0 ),
            wrecord_objs( 0 )
        {}

        void show();
    } counts;

    void show();

};

class HeapTracer {
public:
    virtual ~HeapTracer() {}
public:
    virtual void startCage() {}
    virtual void serialNumberCage( long serial_number ) {}
    virtual void usedCage( long cells_used ) {}
    virtual void capacityCage( long capacity_in_cells ) {}
    virtual void endCage() {}

    virtual void startCrawl() {}
    virtual void atObject( HeapObject heap_object ) {}
    virtual void endCrawl() {}
};

class VirtualMachine {
friend class VirtualMachineFactory;
private:
    shared< AppContext > app_context;
    MachineClass * machine;
private:
    VirtualMachine( shared< AppContext > a, MachineClass * m ) : 
        app_context( a ),
        machine( m ) 
    {}
public:
    ~VirtualMachine() {
        delete this->machine;
    }
public: //  Garbage collection
    void garbageCollect( Ginger::GCData & gcdata );
    void crawlHeap( HeapTracer & tracer );
public: //  Eval operations.
    void execGnx( shared< Ginger::Mnx > gnx );
    void execCode( shared< Ginger::Mnx > code );
public: //  Stack operations.
    ptrdiff_t stackLength();
    void clearStack();
    Cell peek( int n = 0 );
    std::vector< Cell > stack();
public: //  Registers
    HeapObject funcRegister() { return Ginger::HeapObject( this->machine->func_of_program_counter ); }
    HeapObject funcLinkRegister() { return Ginger::HeapObject( this->machine->func_of_link ); }
    long countRegister() { return this->machine->count; }
    ptrdiff_t offsetProgramCounterRegister() { return this->machine->program_counter - this->machine->func_of_program_counter; }
};

class VirtualMachineFactory {
private:
    shared< AppContext > app_context;
public:
    VirtualMachineFactory() :
        app_context( new AppContext() )
    {
        this->app_context->initScript();
    }
public:
    VirtualMachine * newVirtualMachine();
    void setShowCode( const bool b ) { this->app_context->setShowCode( b ); }
};


} // namespace Ginger

#endif
