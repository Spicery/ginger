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
#include <fstream>

//  Standard C libraries.
#include <unistd.h>
#include <time.h>

//  libgng libraries
#include "mnx.hpp"

//  Local libraries
#include "gvm.hpp"

using namespace std;
using namespace Ginger;

class Reporter {
public:
    virtual ~Reporter() {}
public:
    virtual void indent( const int n = 1 ) = 0;
    virtual void binding( const std::string & key, const long n ) = 0;
    virtual void longValue( const long n ) = 0;
    virtual void binding( const std::string & key, HeapObject heap_object ) = 0;
};

class TextReporter : public Reporter {
private:
    int indent_level;
public:
    TextReporter() : indent_level( 0 ) {}
public:
    void indent( int n ) {
        this->indent_level += n;
    }
    void doIndent() {
         for ( int i = 0; i < this->indent_level; i++ ) {
            cout << "    ";
        }       
    }
    void binding( const std::string & key, const long n ) {
        for ( int i = 0; i < this->indent_level; i++ ) {
            cout << "    ";
        }
        cout << key << ": " << n << endl;
    }
    void binding( const std::string & key, HeapObject heap_object ) {
        this->doIndent();
        cout << key << ": " << heap_object.toPrintString() << endl;
    }
    void longValue( const long n ) {
        cout << n << endl;
    }
};

class Tracer : public HeapTracer {
public:
    Tracer( const bool _allow_zeros ) : allow_zeros( _allow_zeros ) {}
private:
    const bool allow_zeros;
private:
    struct Counts {
    private:
        long num_obj;
        long num_fn_obj;
        long num_core_fn_obj;
        long num_method_fn_obj;
        long num_inst_obj;
        long num_vec_obj;
        long num_mixed_obj;
        long num_rec_obj;
        long num_pair_obj;
        long num_map_obj;
        long num_wrec_obj;
        long num_atomicwrec_obj;
        long num_str_obj;
        long num_other_obj;
    public:
        Counts() :
            num_obj( 0 ),
            num_fn_obj( 0 ),
            num_core_fn_obj( 0 ),
            num_method_fn_obj( 0 ),
            num_inst_obj( 0 ),
            num_vec_obj( 0 ),
            num_mixed_obj( 0 ),
            num_rec_obj( 0 ),
            num_pair_obj( 0 ),
            num_map_obj( 0 ),
            num_wrec_obj( 0 ),
            num_atomicwrec_obj( 0 ),
            num_str_obj( 0 ),
            num_other_obj( 0 )
        {}
    private:
        void reportBinding( Reporter & reporter, const bool allow_zeros, const std::string & key, const long n ) {
            if ( allow_zeros || n > 0 ) {
                reporter.binding( key, n );
            }
        }
    public:
        void report( Reporter & reporter, const bool allow_zeros = true ) {
            reporter.indent( 1 );
            reportBinding( reporter, allow_zeros, "#Function", num_fn_obj );
            reportBinding( reporter, allow_zeros, "#Function/Core", num_core_fn_obj );
            reportBinding( reporter, allow_zeros, "#Function/Method", num_method_fn_obj );
            reportBinding( reporter, allow_zeros, "#Instance", num_inst_obj );
            reportBinding( reporter, allow_zeros, "#Vector", num_vec_obj );
            reportBinding( reporter, allow_zeros, "#Mixed", num_mixed_obj );
            reportBinding( reporter, allow_zeros, "#Record", num_rec_obj );
            reportBinding( reporter, allow_zeros, "#Pair", num_pair_obj );
            reportBinding( reporter, allow_zeros, "#Map", num_map_obj );
            reportBinding( reporter, allow_zeros, "#WRecord", num_wrec_obj );
            reportBinding( reporter, allow_zeros, "#AtomicWRecord", num_atomicwrec_obj );
            reportBinding( reporter, allow_zeros, "#String", num_str_obj );
            reportBinding( reporter, allow_zeros, "#OTHER", num_other_obj );
        }
        void bump( HeapObject h ) {
            this->num_obj += 1;
            if ( h.isFunctionObject() ) {
                this->num_fn_obj += 1;
                if ( h.isCoreFunctionObject() ) {
                    this->num_core_fn_obj += 1;
                } else if ( h.isMethodFunctionObject() ) {
                    this->num_method_fn_obj += 1;
                }
            } else if ( h.isInstanceObject() ) {
                this->num_inst_obj += 1;
            } else if ( h.isVectorObject() ) {
                this->num_vec_obj += 1;
            } else if ( h.isMixedObject() ) {
                this->num_mixed_obj += 1;
            } else if ( h.isRecordObject() ) {
                this->num_rec_obj += 1;
            } else if ( h.isPairObject() ) {
                this->num_pair_obj += 1;
            } else if ( h.isMapObject() ) {
                this->num_map_obj += 1;
            } else if ( h.isWRecordObject() ) {
                this->num_wrec_obj += 1;
            } else if ( h.isAtomicWRecordObject() ) {
                this->num_atomicwrec_obj += 1;
            } else if ( h.isStringObject() ) {
                this->num_str_obj += 1;
            } else {
                this->num_other_obj += 1;
            }
        }
    } counts;

public:
    virtual void startCage() {
        this->counts = Counts();
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
        TextReporter reporter;
        this->counts.report( reporter, this->allow_zeros );
        cout << "End Cage" << endl;        
    }

    virtual void atObject( HeapObject heap_object ) {
        this->counts.bump( heap_object );
    }
};

class Cmd {
private:
    VirtualMachine * vm;
    shared< Mnx > command;
    Reporter & reporter;

public:
    Cmd( VirtualMachine * vm, shared< Mnx > command, Reporter & reporter ) : 
        vm( vm ), 
        command( command ),
        reporter( reporter )
    {}

public:

    void doCompile() {
        for ( int i = 0; i < this->command->size(); i++ ) {
            this->vm->execGnx( this->command->child( i ) );
        }
    }

    void doStack() {
        vector< Cell > stack( this->vm->stack() );
        if ( this->command->hasAttribute( "order", "top-last" ) ) {
             for ( vector< Cell >::iterator it = stack.begin(); it != stack.end(); ++it ) {
                Cell & c = *it;
                c.println( std::cout, true );
            }
       } else {
            for ( vector< Cell >::reverse_iterator it = stack.rbegin(); it != stack.rend(); ++it ) {
                Cell & c = *it;
                c.println( std::cout, true );
            }
        }
    }

    void doStackLength() {
        this->reporter.longValue( this->vm->stackLength() );
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
            cout << "Register Commands" << endl;
            cout << "-----------------" << endl;
            cout << "<registers/>                     <help topic=\"registers\"/>" << endl;
            cout << endl;
            cout << "Stack Commands" << endl;
            cout << "--------------" << endl;
            cout << "<peek/>                          <help topic=\"peek\"/>" << endl;
            cout << "<stack.clear/>                   <help topic=\"stack.clear\"/>" << endl;
            cout << "<stack.length/>                  <help topic=\"stack.length\"/>" << endl;
            cout << "<stack/>                         <help topic=\"stack\"/>" << endl;
            cout << endl;
            cout << "Heap Commands" << endl;
            cout << "-------------" << endl;
            cout << "<heap.crawl/>                    <help topic=\"heap.crawl\"/>" << endl;
            cout << "<gc/>                            <help topic=\"gc\"/>" << endl;
            cout << endl;
            cout << "Execute Commands" << endl;
            cout << "----------------" << endl;
            cout << "<compile> GNX </compile>         <help topic=\"compile\"/>" << endl;
            cout << "<code> INSTRUCTION* </code>      <help topic=\"code\"/>" << endl;
        } else if ( topic == "gvm.instructions" ) {
            #define X( VMC, NAME, SIG ) cout << NAME << "\t" << SIG << endl;
            #include "instruction_set.xdef.auto"
            #undef X
        } else {
            std::string file_name( INSTALL_LIB );
            file_name += "/gvmtest/help/";
            file_name += topic;
            file_name += ".rst";
            if ( access( file_name.c_str(), F_OK ) == 0 ) {
                #ifndef NO_PAGER
                    pid_t pid = fork();
                    if ( pid == 0 ) {
                        // In the child process.
                        execl(
                            "/usr/bin/less",
                            "/usr/bin/less",
                            file_name.c_str(),
                            0
                        );
                        cerr << "Problem opening help file: " << file_name << endl;
                    } else {
                        // In the parent process.
                        wait( 0 );
                    }
                #else
                    ifstream file( file_name.c_str() );
                    if ( file.good() ) {
                        ifstream file( file_name.c_str() );
                        string line;
                        while( getline( file, line ) ) {  
                            cout << line << endl;
                        }
                    } else {
                        cerr << "Problem opening help file: " << file_name << endl;
                    }
                #endif
            } else {
                cerr << "Sorry, help not implemented for topic: " << topic << endl;
                cerr << "File required: " << file_name << endl;
            }
        }
    }

    void doHeapCrawl() {
        Tracer tracer( this->command->attribute( "zeros", "" ) == "true" );
        this->vm->crawlHeap( tracer );
    }

    void doRegisters() {
        /*  Scaffolding
            * PC    virtual program counter, pointer to currently executing instruction
            * FUNC  pointer to currently executing function
            * COUNT the number of items passed on the stack to the current call
            * LINK  the previous PC i.e. return address
            * FUNCLINK the previous value of FUNC
        */
        this->reporter.binding( "COUNT", this->vm->countRegister() );
        this->reporter.binding( "PC (offset from FUNC)", this->vm->offsetProgramCounterRegister() );
        this->reporter.binding( "FUNC", this->vm->funcRegister() );
        this->reporter.binding( "FUNCLINK", this->vm->funcLinkRegister() );
    }

    void doCommand() {
        const string & name = command->name();
        if ( name == "help" ) {
            this->doHelp();
        } else if ( name == "registers" ) {
            this->doRegisters();
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
            TextReporter reporter;
            Cmd( vm, mnx, reporter ).doCommand();
        } catch ( Ginger::Mishap m ) {
            m.report();
       }
    }

    return EXIT_SUCCESS;
}