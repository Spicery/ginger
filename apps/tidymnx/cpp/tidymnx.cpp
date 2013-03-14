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

#include <string>
#include <fstream>

#include <stddef.h>
#include <stdlib.h>
#include <getopt.h>

#include "gngversion.hpp"
#include "mnx.hpp"
#include "mishap.hpp"

using namespace Ginger;
using namespace std;


#define TIDYMNX "tidymnx"

extern char * optarg;
static struct option long_options[] =
    {
    	{ "indent",			required_argument,		0, 'i' },
        { "help",			optional_argument,		0, 'H' },
        { "license",        optional_argument,      0, 'L' },
        { "version",        no_argument,            0, 'V' },
        { 0, 0, 0, 0 }
    };

class Main {
private:
	bool use_stdin;
	string input_file_name;
	string indent;

public:
	string version() {
		return "0.1";
	}

	void printGPL( const char * start, const char * end ) {
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


	void parseArgs( int argc, char **argv, char **envp ) {
		this->use_stdin = true;
		this->indent = "    ";
		
		for(;;) {
			int option_index = 0;
			int c = getopt_long( argc, argv, "i:H::L::V", long_options, &option_index );
			if ( c == -1 ) break;
			switch ( c ) {
				case 'H': {
					//  Eventually we will have a "home" for our auxillary
					//  files and this will simply go there. Or run a web
					//  browser pointed there.
					if ( optarg == NULL ) {
						cout << "Usage:  " << TIDYMNX << " OPTIONS < MNX_IN > MNX_OUT" << endl;
						cout << "OPTIONS" << endl;
						cout << "-i, --indent=STRING   sets the indentation string." << endl;
						cout << "-H, --help[=TOPIC]    help info on optional topic (see --help=help)" << endl;
						cout << "-V, --version         print out version information and exit" << endl;
						cout << "-L, --license[=PART]  print out license information and exit (see --help=license)" << endl;
						cout << endl;
					} else if ( std::string( optarg ) == "help" ) {
						cout << "--help=help           this short help" << endl;
						cout << "--help=licence        help on displaying license information" << endl;
					} else if ( std::string( optarg ) == "license" ) {
						cout << "Displays key sections of the GNU Public License." << endl;
						cout << "--license=warranty    Shows warranty." << endl;
						cout << "--license=conditions  Shows terms and conditions." << endl;
					} else {
						cout << "Unknown help topic " << optarg << endl;
					}
					exit( EXIT_SUCCESS );   //  Is that right?
				}
				case 'i': {
					this->indent = optarg;
					break;
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
					cout << TIDYMNX << ": version " << this->version() << " (" << __DATE__ << " " << __TIME__ << ") part of " << PACKAGE_NAME << " version " << PACKAGE_VERSION << endl;
					exit( EXIT_SUCCESS );   //  Is that right?
				}
				case '?': {
					break;
				}
				default: {
					cout << "?? getopt returned character code 0x" << hex << static_cast< int >( c ) << dec << endl;
				}
			}
        }
        
		argc -= optind;
		argv += optind;
		
		if ( argc > 0 ) {
			use_stdin = false;
			input_file_name = argv[ argc - 1 ];
			argc -= 1;
			argv += 1;
		}
		
		if ( argc > 0 ) {
			throw Mishap( "Unused trailing argument" ).culprit( "Unused", argv[ argc - 1 ] );
		}

	}
	
public:
	void run( std::istream & in ) {
		MnxReader reader( in );
		SharedMnx g( reader.readMnx() );
		g->prettyPrint( this->indent );
	}

	int run() {
		try {
			if ( this->use_stdin ) {
				run( std::cin );
			} else {
				ifstream in( input_file_name.c_str() );
				if ( in.bad() ) throw Ginger::Mishap( "Cannot open file" ).culprit( "Filename", input_file_name );
				run( in );
			}
		} catch ( Ginger::Mishap & m ) {
			m.gnxReport();
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	}
};

int main( int argc, char **argv, char **envp ) {
	try {
		Main main;
		main.parseArgs( argc, argv, envp );
		return main.run();
	} catch ( Ginger::Mishap & p ) {
		p.culprit( "Detected by", TIDYMNX );
		p.gnxReport();
		return EXIT_FAILURE;
	}
}
