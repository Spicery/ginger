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

class VirtualMachine {
friend class VirtualMachineFactory;
private:
    shared< ::AppContext > app_context;
    ::MachineClass * machine;
private:
    VirtualMachine( shared< ::AppContext > a, ::MachineClass * m ) : 
        app_context( a ),
        machine( m ) 
    {}
public:
    ~VirtualMachine() {
        delete this->machine;
    }
public: //  Garbage collection
    void garbageCollect( Ginger::GCData & gcdata );
public: //  Eval operations.
    void execGnx( shared< Ginger::Mnx > gnx );
public: //  Stack operations.
    ptrdiff_t stackLength();
    Cell peek( int n );
    std::vector< Cell > stack();
};

class VirtualMachineFactory {
private:
    shared< ::AppContext > app_context;
public:
    VirtualMachineFactory() :
        app_context( new AppContext() )
    {
        this->app_context->initScript();
    }
public:
    VirtualMachine * newVirtualMachine();
};


} // namespace Ginger
#endif