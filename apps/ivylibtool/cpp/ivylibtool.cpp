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

//  Usage: IVYLIBTOOL [OPTIONS] < QUERY > RESPONSE
//      -H, --help              print out help summary and exit
//      -V, --version           print out version information and exit
//      -L, --license[=PART]    print out license information and exit (see --help=license)




//  Local debugging switch for conditional code compilation.
//#define DBG_IVYLIBTOOL 1

 
#define IVYLIBTOOL "ivylibtool"

#include <iostream>
#include <string>
#include <utility>
#include <fstream>
#include <vector>
#include <cstdlib>

#include <stddef.h>
#include <getopt.h>
#include <syslog.h>


#include "printgpl.hpp"
#include "mnx.hpp"
#include "mishap.hpp"

using namespace std;

#define STANDARD_LIBRARY    "standard_library"

class Task {
protected:
    vector< string > arguments;
public:
    virtual void run() = 0;
public:
    virtual void addArgument( const string arg ) {
        this->arguments.push_back( arg );
    }
    virtual const string folder() const {
        if ( arguments.empty() ) {
            throw Ginger::Mishap( "No project specified" );
        }
        return arguments[0];
    }
    virtual ~Task() {}
};

/*
   * <fetch.index/>
   * <fetch.var.gnx def.pkg=${DEFPKG} var.name=${VAR} />
   * <fetch.init.gnx def.pkg=${DEFPKG} />
*/
class ExecuteTask : public Task {
private:
    Ginger::SharedMnx query;

private:
    void fetchIndex() {
        cout << "fetchIndex" << endl;
        cout << this->folder() << endl;
    }
    void fetchVarGnx() {
        cout << "fetchVarGnx" << endl;
    }
    void fetchInitGnx() {
        cout << "fetchInitGnx" << endl;

    }
public:
    virtual void run() {
        Ginger::MnxReader reader;  
        this->query = reader.readMnx();
        if ( ! this->query ) {
        	throw Ginger::Mishap( "No MinXML query on input" );
        }
        if ( query->hasName( "fetch.index" ) ) {
            this->fetchIndex();
        } else if ( query->hasName( "fetch.var.gnx" ) ) {
            this->fetchVarGnx();
        } else if ( query->hasName( "fetch.init.gnx" ) ) {
            this->fetchInitGnx();
        } else {
            throw Ginger::Mishap( "Unrecognised query" ).culprit( "Query", this->query->toString() );
        }
    }
};

class Main {
private:
    unique_ptr< Task >          task;
    string                      project_folder;
    
public:
    void parseArgs( int argc, char **argv, char **envp );
    void summary();
    void run();
    std::string version();
    
public:
    Main() : 
        task( nullptr )
    {
    }
};

std::string Main::version() {
    //  ToDo: not very happy with the way I have got versioning structured.
    return "0.1";
}

extern char * optarg;
static struct option long_options[] =
    {
        { "help",           optional_argument,      0, 'H' },
        { "license",        optional_argument,      0, 'L' },
        { "query",          no_argument,            0, 'Q' },
        { "version",        no_argument,            0, 'V' },
        { 0, 0, 0, 0 }
    };

static void printUsage() {
    cout << "Usage: " << IVYLIBTOOL << " [MODE OPTION] <PATHNAME>" << endl;
    cout << "MODE OPTIONS" << endl;
    cout << "-Q, --query           execute a MinXML query read from stdin (default mode)" << endl;
    cout << "-H, --help[=TOPIC]    help info on optional topic (see --help=help)" << endl;
    cout << "-L, --license[=PART]  print out license information (see --help=license)" << endl;
    cout << "-V, --version         print out version information" << endl;
    cout << endl;
}

static void printHelpOptionUsage() {
    cout << "--help=help           this short help" << endl;
    cout << "--help=licence        help on displaying license information" << endl;
    cout << "--help=std            print out variables in std" << endl;
}

static void printLicenseOptionUsage() {
    cout << "Displays key sections of the GNU Public License." << endl;
    cout << "--license=warranty    Shows warranty." << endl;
    cout << "--license=conditions  Shows terms and conditions." << endl;
}

void Main::parseArgs( int argc, char **argv, char **envp ) {
    this->task.reset( new ExecuteTask() );
    for(;;) {
        int option_index = 0;
        int c = getopt_long( argc, argv, "H::L::QV", long_options, &option_index );
        if ( c == -1 ) break;
        switch ( c ) {
            case 'H': {
                //  Eventually we will have a "home" for our auxillary
                //  files and this will simply go there. Or run a web
                //  browser pointed there.
                if ( optarg == NULL ) {
                    printUsage();
                } else if ( std::string( optarg ) == "help" ) {
                    printHelpOptionUsage();
                } else if ( std::string( optarg ) == "license" ) {
                    printLicenseOptionUsage();
                } else {
                    cout << "Unknown help topic " << optarg << endl;
                }
                exit( EXIT_SUCCESS );   //  Is that right?
            }
            case 'L': {
                Ginger::optionPrintGPL( optarg );
                exit( EXIT_SUCCESS );   //  Is that right?              
            }
            case 'Q': {
                //  Default case. No action needed.
                break;
            }
            case 'V': {
                cout << IVYLIBTOOL << ": version " << this->version() << " (" << __DATE__ << " " << __TIME__ << ")" << endl;
                exit( EXIT_SUCCESS );   //  Is that right?
            }
            case '?': {
                exit( EXIT_FAILURE );
            }
            default: {
                cout << "?? getopt returned character code " << hex << static_cast< int >( c ) << dec << endl;
            }
        }
    }
    //  PROCESS OTHER ARGS GOES HERE
    //      Aggregate the remaining arguments, which are effectively filenames (paths).
    if ( optind < argc ) {
        while ( optind < argc ) {
            this->task->addArgument( argv[ optind++ ] );
        }
    }
}

void Main::run() {
    this->task->run();
}

int main( int argc, char ** argv, char **envp ) {
    openlog( IVYLIBTOOL, 0, LOG_LOCAL2 );
    setlogmask( LOG_UPTO( LOG_INFO ) );
    try {
        Main main;
        main.parseArgs( argc, argv, envp );
        main.run();
        return EXIT_SUCCESS;
    } catch ( Ginger::Mishap & p ) {
        p.culprit( "Detected by", IVYLIBTOOL );
        p.report();
        return EXIT_FAILURE;
    }
}
