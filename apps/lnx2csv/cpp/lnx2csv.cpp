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

#include <stdlib.h>
#include <getopt.h>

#include "gngversion.hpp"
#include "lnxreader.hpp"
#include "mishap.hpp"

using namespace Ginger;
using namespace std;


#define APP_NAME "lnx2csv"

extern char * optarg;
static struct option long_options[] =
    {
    	{ "columns",		no_argument,			0, 'c' },
    	{ "help",			optional_argument,		0, 'H' },
        { "license",        optional_argument,      0, 'L' },
        { "version",        no_argument,            0, 'V' },
        { 0, 0, 0, 0 }
    };

class Main {
private:
	bool use_stdin;
	string input_file_name;
	bool columns;

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
		this->columns = false;
		
		for(;;) {
			int option_index = 0;
			int c = getopt_long( argc, argv, "cH::L::V", long_options, &option_index );
			if ( c == -1 ) break;
			switch ( c ) {
				case 'c': {
					this->columns = true;
					break;
				}
				case 'H': {
					//  Eventually we will have a "home" for our auxillary
					//  files and this will simply go there. Or run a web
					//  browser pointed there.
					if ( optarg == NULL ) {
						printf( "Usage:  %s OPTIONS < LNX_IN > CSV_OUT\n", APP_NAME );
						printf( "OPTIONS\n" );
						printf( "-c, --columns         first line of output is a CSV header\n" );
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
					cout << APP_NAME << ": version " << this->version() << " (" << __DATE__ << " " << __TIME__ << ") part of " << PACKAGE_NAME << " version " << PACKAGE_VERSION << endl;
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
			use_stdin = false;
			input_file_name = argv[ argc - 1 ];
			argc -= 1;
			argv += 1;
		}
		
		if ( argc > 0 ) {
			throw Mishap( "Unused trailing argument" ).culprit( "Unused", argv[ argc - 1 ] );
		}

	}

private:
	static void csvout( std::string & text ) {
		cout << '"';
		for ( 
			string::iterator it = text.begin();
			it != text.end();
			++it
		) {
			char ch = *it;
			if ( ch == '"' ) {
				cout << "\"\"";
			} else {
				cout << ch;
			}
		}
		cout << '"';
	}

	void run( istream & in ) {
		LnxReader reader( in );
		
		if ( this->columns ) {
			const char * gap = "";
			for ( int i = 0; i < reader.propertyCount(); i++ ) {
				cout << gap;
				
				//	Note that column names do NOT need quoting. This is because
				//	they MUST obey the XML name format which (by inspection) does
				//	not require quoting.
				cout << "\"" << reader.propertyKey( i ) << "\"";
				
				gap = ",";
				
			}
			cout << endl;
		}
		
		int N = reader.propertyCount();
		for (;;) {
			LnxItem * item = reader.read();
			if ( item == NULL ) break;
			const char * gap = "";
			for ( int i = 0; i < N; i++ ) {
				cout << gap;
				csvout( item->propertyValue( i ) );
				gap = ",";				
			}
			cout << endl;
		}
	}

public:
	void run() {
		if ( this->use_stdin ) {
			run( cin );
		} else {
			ifstream input( input_file_name.c_str() );
			run( input );
		}

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
