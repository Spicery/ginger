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

//	Local debugging switch for conditional code compilation.
//#define DBG_SIMPLIFYGNX 1

/**
The simplifygnx command is used to simplify the Ginger XML (GNX) fed
to it as a stream.
	
--absolute : replace all qualified/unqualified references to global
	variables with absolute references to their package of origin (i.e.
	defining package).

--arity : performs a basic arity analysis, tagging expressions with
	their proven arity and flagging bindings and system calls as safe.

--lifetime : tags the last use of a local variable, so the compiler can
	safely nullify. Also tags each local variable with an explicit position
	in the stack frame, the parent function with the max position number,
	and whether or not the position will be reused before the end of the
	function (which informs nullification).

--lift: performs the generalised lift transformation that ensures all
	local variables are strictly local. It will use partApply to explicitly
	replace closures, invent extra local variables, passing in explicitly
	created reference objects.
 */ 
 
#define SIMPLIFYGNX "simplifygnx"

#include <iostream>
#include <fstream>

#include <getopt.h>
#include <syslog.h>
#include <stdlib.h>

#include "mishap.hpp"


using namespace std;

class Main {
public:
	void parseArgs( int argc, char **argv, char **envp );
	void run();
	void printGPL( const char * start, const char * end );
	std::string version();
	
public:
	Main() {}
};

std::string Main::version() {
	return "0.1";
}


extern char * optarg;
static struct option long_options[] =
    {
		{ "absolute",		no_argument,			0, '1' },
		{ "arity",			no_argument,			0, '2' },
        { "help",			optional_argument,		0, 'H' },
        { "license",        optional_argument,      0, 'L' },
        { "lifetime", 		no_argument,			0, '3' },
        { "lift",			no_argument,			0, '4' },
        { "version",        no_argument,            0, 'V' },
        { 0, 0, 0, 0 }
    };

void Main::parseArgs( int argc, char **argv, char **envp ) {
    for(;;) {
        int option_index = 0;
        int c = getopt_long( argc, argv, "1234H::VL::", long_options, &option_index );
        if ( c == -1 ) break;
        switch ( c ) {
            case 'H': {
                //  Eventually we will have a "home" for our auxillary
                //  files and this will simply go there. Or run a web
                //  browser pointed there.
                if ( optarg == NULL ) {
                    printf( "Usage:  simplifygnx OPTIONS < GNX_IN > GNX_OUT\n" );
                    printf( "OPTIONS\n" );
                    printf( "-H, --help[=TOPIC]    help info on optional topic (see --help=help)\n" );
                    printf( "-V, --version         print out version information and exit\n" );
                    printf( "-L, --license[=PART]  print out license information and exit (see --help=license)\n" );
                    printf( "\n" );
                } else if ( std::string( optarg ) == "help" ) {
                    cout << "--help=help           this short help" << endl;
                    cout << "--help=licence        help on displaying license information" << endl;
                } else if ( std::string( optarg ) == "license" ) {
                    cout << "Displays key sections of the GNU Public License." << endl;
                    cout << "--license=warranty    Shows warranty." << endl;
                    cout << "--license=conditions  Shows terms and conditions." << endl;
                } else {
                    printf( "Unknown help topic %s\n", optarg );
                }
                exit( EXIT_SUCCESS );   //  Is that right?
            }
            case 'L': {
                if ( optarg == NULL || std::string( optarg ) == std::string( "all" ) ) {
                    this->printGPL( NULL, NULL );
                } else if ( std::string( optarg ) == std::string( "warranty" ) ) {
                    this->printGPL( "Disclaimer of Warranty.", "Limitation of Liability." );                 
                } else if ( std::string( optarg ) == std::string( "conditions" ) ) {
                    this->printGPL( "TERMS AND CONDITIONS", "END OF TERMS AND CONDITIONS" );
                } else {
                    std::cerr << "Unknown license option: " << optarg << std::endl;
                    exit( EXIT_FAILURE );
                }
                exit( EXIT_SUCCESS );   //  Is that right?              
            }
            case 'V': {
                cout << SIMPLIFYGNX << ": version " << this->version() << " (" << __DATE__ << " " << __TIME__ << ")" << endl;
                exit( EXIT_SUCCESS );   //  Is that right?
            }
            case '?': {
                break;
            }
            default: {
                printf( "?? getopt returned character code 0%x ??\n", static_cast< int >( c ) );
            }
        }
    }
}
    

void Main::printGPL( const char * start, const char * end ) {
    bool printing = false;
    ifstream license( INSTALL_LIB "/LICENSE.TXT" );
    std::string line;
    while ( getline( license, line ) )  {
        if ( !printing && ( start == NULL || line.find( start ) != string::npos ) ) {
            printing = true;
        } else if ( printing && end != NULL && line.find( end ) != string::npos ) {
            printing = false;
        }
        if ( printing ) {
            std::cout << line << std::endl;
        }
    }
}

void Main::run() {
	//	To be done.
}

int main( int argc, char ** argv, char **envp ) {
	openlog( SIMPLIFYGNX, 0, LOG_LOCAL2 );
	setlogmask( LOG_UPTO( LOG_INFO ) );
	try {
		Main main;
		main.parseArgs( argc, argv, envp );
		main.run();
	    return EXIT_SUCCESS;
	} catch ( Ginger::SystemError & p ) {
		p.report();
		return EXIT_FAILURE;
	} catch ( Ginger::Problem & p ) {
		p.report();
		return EXIT_FAILURE;
	}
}
