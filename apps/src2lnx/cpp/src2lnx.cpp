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

//	STL
#include <iostream>
#include <fstream>
#include <cstdlib>

//	C libraries
#include <stddef.h>
#include <getopt.h>

//	Part of libgng
#include "printgpl.hpp"
#include "mishap.hpp"
#include "gngversion.hpp"
#include "mnx.hpp"

//	This application modules.
#include "grammar.hpp"

using namespace std;
using namespace Ginger;
using namespace SRC2LNX_NS;


#define SRC2LNX_APP "src2lnx"

extern char * optarg;
static struct option long_options[] =
    {
    	{ "grammar",		required_argument,		0, 'g' },
        { "help",			optional_argument,		0, 'H' },
        { "license",        optional_argument,      0, 'L' },
        { "version",        no_argument,            0, 'V' },
        { 0, 0, 0, 0 }
    };

class Main {
private:
	bool use_stdin;
	string input_file_name;
	string grammar_file;

public:
	string version() {
		return "0.1";
	}

	void printHelp() {
		//  Eventually we will have a "home" for our auxillary
		//  files and this will simply go there. Or run a web
		//  browser pointed there.
		if ( optarg == NULL ) {
			printf( "Usage:  %s OPTIONS < XSON_IN > GNX_OUT\n", SRC2LNX_APP );
			printf( "OPTIONS\n" );
			printf( "-g, --grammar=CONTROL_FILE the tokeniser's control file\n" );
			printf( "-H, --help[=TOPIC]         help info on optional topic (see --help=help)\n" );
			printf( "-V, --version              print out version information and exit\n" );
			printf( "-L, --license[=PART]       print out license information and exit (see --help=license)\n" );
			printf( "\n" );
		} else if ( std::string( optarg ) == "help" ) {
			cout << "--help=help           this short help" << endl;
			cout << "--help=license        help on displaying license information" << endl;
		} else if ( std::string( optarg ) == "license" ) {
			cout << "Displays key sections of the GNU Public License." << endl;
			cout << "--license=warranty    Shows warranty." << endl;
			cout << "--license=conditions  Shows terms and conditions." << endl;
		} else {
			printf( "Unknown help topic %s\n", optarg );
		}
	}

	void parseArgs( int argc, char **argv, char **envp ) {
		this->use_stdin = true;
		for(;;) {
			int option_index = 0;
			int c = getopt_long( argc, argv, "g:H::L::V", long_options, &option_index );
			if ( c == -1 ) break;
			switch ( c ) {
				case 'g': {
					this->grammar_file = optarg;
					break;
				}
				case 'H': {
					this->printHelp();
					exit( EXIT_SUCCESS );   //  Is that right?
				}
				case 'L': {
					exit( optionPrintGPL( optarg ) );
				}
				case 'V': {
					cout << SRC2LNX_APP << ": version " << this->version() << " (" << __DATE__ << " " << __TIME__ << ") part of " << PACKAGE_NAME << " version " << PACKAGE_VERSION << endl;
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
        
		argc -= optind;
		argv += optind;
		
		if ( argc > 0 ) {
			throw Mishap( "Unused trailing argument" ).culprit( "Unused", argv[ argc - 1 ] );
		}

	}
	
public:
	void run() {
		ifstream g( this->grammar_file.c_str() );
		MnxReader reader( g );
		shared< Mnx > grammar_description( reader.readMnx() );
		
		State state;
		Grammar grammar( &state, grammar_description );
		
		grammar.start();
		char ch;
		cin >> noskipws;
		state.lineno = 1;
		while ( cin >> ch ) {
			do {
				state.ok = true;
				grammar.processChar( ch ); 
			} while ( !state.ok );
			if ( ch == '\n' ) {
				state.lineno += 1;
			}
		}
		grammar.processEof();
		grammar.finish();
		
	}
};



int main( int argc, char **argv, char **envp ) {
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



