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

#include <iostream>
#include <fstream>

#include <cstdlib>

#include <syslog.h>
#include <stddef.h>
#include <getopt.h>

#include "gngversion.hpp"

#include "printgpl.hpp"
#include "mishap.hpp"
#include "toolmain.hpp"

using namespace std;

#define LOG_FACILITY	LOG_LOCAL2

extern char * optarg;
static struct option long_options[] =
    {
    	{ "help",			optional_argument,		0, 'H' },
        { "license",        optional_argument,      0, 'L' },
        { "version",        no_argument,            0, 'V' },
        { 0, 0, 0, 0 }
    };

void ToolMain::parseArgs( int argc, char **argv, char **envp ) {
	this->use_stdin = true;

	for(;;) {
		int option_index = 0;
		int c = getopt_long( argc, argv, "H::L::V", long_options, &option_index );
		if ( c == -1 ) break;
		switch ( c ) {
			case 'H': {
				//  Eventually we will have a "home" for our auxillary
				//  files and this will simply go there. Or run a web
				//  browser pointed there.
				if ( optarg == NULL ) {
					cout << "Usage:  " << this->app_title << " OPTIONS < LNX_IN > CSV_OUT" << endl;
					cout << "OPTIONS" << endl;
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
			case 'L': {
				exit( Ginger::optionPrintGPL( optarg ) );              
			}
			case 'V': {
				cout << this->app_title << ": version " << this->version() << " (" << __DATE__ << " " << __TIME__ << ") part of " << PACKAGE_NAME << " version " << PACKAGE_VERSION << endl;
				exit( EXIT_SUCCESS );   //  Is that right?
			}
			case '?': {
				break;
			}
			default: {
				cout << "?? getopt returned character code 0%x ??" << static_cast< int >( c ) << endl;
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
		throw Ginger::Mishap( "Unused trailing argument" ).culprit( "Unused", argv[ argc - 1 ] );
	}

}


void ToolMain::run( FILE * in ) {
	// TO BE CONTINUED ...
}

int ToolMain::run() {
	openlog( this->app_title, 0, LOG_FACILITY );
	setlogmask( LOG_UPTO( LOG_INFO ) );
	
	try {
		if ( this->use_stdin ) {
			this->run( stdin );
		} else {
			const char * fname = input_file_name.c_str();
			FILE * in = fopen( fname, "r" ); 
			if ( in == NULL ) throw Ginger::Mishap( "Cannot open file" ).culprit( "Filename", fname );
			run( in );
		}
	} catch ( Ginger::Mishap & m ) {
		m.gnxReport();
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}

