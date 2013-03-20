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

class Result {
public:
    virtual ~Result() {}
public:
    virtual void longValue( const long n ) = 0;
};

class TextResult : public Result {
    void longValue( const long n ) {
        cout << n << endl;
    }
};

class Tracer : public HeapTracer {
private:
    long num;
public:
    virtual void startCage() {
        this->num = 0;
        cout << "Start Cage" << endl;
    }

    virtual void serialNumberCage( long serial_number ) {
        cout << "    Serial : " << serial_number << endl;
    }

    virtual void usedCage( long cells_used ) {
        cout << "    Used   : " << cells_used << endl;
    }

    virtual void capacityCage( long capacity_in_cells ) {
        cout << "    Capacity: " << capacity_in_cells << endl;
    }

    virtual void endCage() {
        cout << "    #Objects: " << this->num << endl;
        cout << "End Cage" << endl;        
    }

    virtual void atObject( HeapObject heap_object ) {
        this->num += 1;
    }
};

class Cmd {
private:
    VirtualMachine * vm;
    shared< Mnx > command;
    Result & result;

public:
    Cmd( VirtualMachine * vm, shared< Mnx > command, Result & result ) : 
        vm( vm ), 
        command( command ),
        result( result )
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

    void doStackLength() {
        this->result.longValue( this->vm->stackLength() );
    }

    void doGarbageCollect() {
        GCData gcdata;
        this->vm->garbageCollect( gcdata );
        gcdata.show();
    }

    void doPeek() {
        if ( this->command->hasAttribute( "n" ) ) {
            Ginger::Cell c( this->vm->peek( this->command->attributeToLong( "n" ) ) );
            c.println( std::cout, true );
        } else {
            Ginger::Cell c( this->vm->peek() );
            c.println( std::cout, true );            
        }
    }

    void doCode() {
        this->vm->execCode( this->command );
    }

    void doStackClear() {
        this->vm->clearStack();
    }

    void doHelp() {
        const std::string topic = this->command->attribute( "topic", "" );
        if ( topic == "" ) {
            cout << "Stack Commands" << endl;
            cout << "--------------" << endl;
            cout << "<peek/>                          <help topic=\"peek\"/>" << endl;
            cout << "<stack.clear/>                   <help topic=\"stack.clear\"/>" << endl;
            cout << "<stack.length/>                  <help topic=\"stack.length\"/>" << endl;
            cout << "<stack/>                         <help topic=\"stack\"/>" << endl;
            cout << endl;
            cout << "Heap Commands" << endl;
            cout << "<heap.crawl/>                    <help topic=\"heap.crawl\"/>" << endl;
            cout << "<gc/>                            <help topic=\"gc\"/>" << endl;
            cout << endl;
            cout << "Execute Commands" << endl;
            cout << "----------------" << endl;
            cout << "<compile> GNX </compile>         <help topic=\"compile\"/>" << endl;
            cout << "<code> INSTRUCTION* </code>      <help topic=\"code\"/>" << endl;
        } else {
            cout << "Help not implemented for topic: " << topic << endl;
        }
    }

    void doHeapCrawl() {
        Tracer tracer;
        this->vm->crawlHeap( tracer );
    }

    void doCommand() {
        const string & name = command->name();
        if ( name == "help" ) {
            this->doHelp();
        } else if ( name == "stack" ) {
            this->doStack();
        } else if ( name == "stack.length" ) {
            this->doStackLength();
        } else if ( name == "stack.clear" ) {
            this->doStackClear();
        } else if ( name == "peek" ) {
            this->doPeek();
        } else if ( name == "gc" ) {
            this->doGarbageCollect();
        } else if ( name == "heap.crawl" ) {
            this->doHeapCrawl();
        } else if ( name == "code" ) {
            this->doCode();
        } else if ( name == "compile" ) {
            this->doCompile();
        } else {
            cerr << "Unrecognised command: ";
            this->command->render( cerr );
            cerr << endl;
        }
    }

};


int main( int argc, char ** argv ) {
    VirtualMachineFactory f;
    f.setShowCode( true );
    VirtualMachine * vm = f.newVirtualMachine();

    for (;;) {
        try {
            MnxReader read_xml( std::cin );
            shared< Mnx > mnx( read_xml.readMnx() );
            if ( not mnx ) break;
            TextResult result;
            Cmd( vm, mnx, result ).doCommand();
        } catch ( Ginger::Mishap m ) {
            m.report();
       }
    }

    return EXIT_SUCCESS;
}