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

//  Standard C++ libraries
#include <iostream>
#include <string>
#include <vector>    

//  Standard C libraries.
#include <unistd.h>
#include <time.h>

//  libgng libraries
#include "mnx.hpp"

//  Local libraries
#include "gvm.hpp"

using namespace std;
using namespace Ginger;



class Cmd {
private:
    VirtualMachine * vm;
    shared< Mnx > command;

public:
    Cmd( VirtualMachine * vm, shared< Mnx > command ) : 
        vm( vm ), 
        command( command ) 
    {}

public:

    void doCompile() {
        for ( int i = 0; i < this->command->size(); i++ ) {
            this->vm->execGnx( this->command->child( i ) );
        }
    }

    void doStack() {
        vector< Cell > stack( this->vm->stack() );
        for ( vector< Cell >::iterator it = stack.begin(); it != stack.end(); ++it ) {
            Cell & c = *it;
            c.println( std::cout, true );
        }
    }

    void doGarbageCollect() {
        GCData gcdata;
        this->vm->garbageCollect( gcdata );
        gcdata.show();
    }

    void doCommand() {
        const string & name = command->name();
        if ( name == "compile" ) {
            this->doCompile();
        } else if ( name == "stack" ) {
            this->doStack();
        } else if ( name == "gc" ) {
            this->doGarbageCollect();
        } else {
            cerr << "Unrecognised command: ";
            this->command->render( cerr );
            cerr << endl;
        }
    }

};




int main( int argc, char ** argv ) {
    VirtualMachineFactory f;
    VirtualMachine * vm = f.newVirtualMachine();

    for (;;) {
        MnxReader read_xml( std::cin );
        shared< Mnx > mnx( read_xml.readMnx() );
        if ( not mnx ) break;
        Cmd( vm, mnx ).doCommand();
    }

    return EXIT_SUCCESS;
}