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

#include <iostream>
#include <fstream>

#include <cstdlib>

#include <syslog.h>
#include <stddef.h>
#include <getopt.h>

#include "gngversion.hpp"

#include "mishap.hpp"
#include "item_factory.hpp"
#include "read_expr.hpp"

#include "toolmain.hpp"

using namespace std;

#define LOG_FACILITY	LOG_LOCAL2

extern char * optarg;
static struct option long_options[] =
    {
    	{ "help",			optional_argument,		0, 'H' },
        { "license",        optional_argument,      0, 'L' },
        { "tokenise",		no_argument,			0, 'T' },
        { "version",        no_argument,            0, 'V' },
        { "nospan",         no_argument,            0, 's' },
        { 0, 0, 0, 0 }
    };


void ToolMain::printGPL( const char * start, const char * end ) {
	bool printing = false;
	ifstream license( INSTALL_LIB "/COPYING" );
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


void ToolMain::parseArgs( int argc, char **argv, char **envp ) {
	this->gen_lnx = false;
	this->use_stdin = true;

	for(;;) {
		int option_index = 0;
		int c = getopt_long( argc, argv, "TH::L::Vs", long_options, &option_index );
		if ( c == -1 ) break;
		switch ( c ) {
			case 'T': {
				this->gen_lnx = true;
				break;
			}
			case 'H': {
				//  Eventually we will have a "home" for our auxillary
				//  files and this will simply go there. Or run a web
				//  browser pointed there.
				if ( optarg == NULL ) {
					cout << "Usage:  " << this->app_title << " OPTIONS < LNX_IN > CSV_OUT" << endl;
					cout << "OPTIONS" << endl;
					cout << "-T, --tokenise        tokenise only, generate LNX output instead of GNX" << endl;
					cout << "-H, --help[=TOPIC]    help info on optional topic (see --help=help)" << endl;
					cout << "-V, --version         print out version information and exit" << endl;
					cout << "-L, --license[=PART]  print out license information and exit (see --help=license)" << endl;
					cout << "-s, --nospan          suppress span attribution" << endl;
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
			case 's': {
				this->no_span = true;
				break;
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

/*
	Properties
		V = value
		R = role
		T = type
		P = precedence
		L = line number (not generated)

	This method generates LNX output. The point of LNX is to enable
	the lnx2lnx parser, so the classification required is quite
	specific. All tokens are one of the following:
	
	1) 	Literals: R="literal" T=type of literal. 
		e.g. 3 is R="literal" T="int"
		e.g. "fred" is R="literal" T="string"
		
	2) 	Identifiers: R="name"
		e.g. foo is R="name"
	
	3)  Operators: R="operator" P=precedence
		e.g. + is R="operator" P="600"
	
	4)  Reserved: R="reserved"
		e.g. if is R="reserved"			
	
*/
void ToolMain::tokenise( FILE *in ) {
	cout << "<item.stream V=\"\" R=\"\" T=\"\" P=\"\" L=\"\">" << endl;
	for (;;) {
		ItemFactoryClass ifact( in, this->cstyle );
		Item item = ifact.read();
		if ( item->tok_type == tokty_eof ) break;
		cout << "<.";
		cout << " V=\"" << item->nameString() << "\"";
		if ( item->tok_type == tokty_charseq ) { 
			cout << " R=\"charseq\"";
		} else if ( item->tok_type == tokty_id ) { 
			cout << " R=\"name\"";
		} else if ( item->role.IsLiteral() ) {
			cout << " R=\"literal\"";
			if ( item->tok_type == tokty_int ) {
				cout << " T=\"int\"";
			} else if ( item->tok_type == tokty_double ) {
				cout << " T=\"double\"";
			} else if ( item->tok_type == tokty_string ) {
				cout << " T=\"string\"";
			}
		} else if ( item->role.compile_code == SysCode && item->precedence != 0 ) {
			cout << " R=\"operator\"";
			cout << " P=\"" << item->precedence << "\"";
		} else {
			cout << " R=\"reserved\"";
		}
		cout << "/>" << endl;
	}
	cout << "</item.stream>" << endl;		
}

void ToolMain::parse( FILE * in ) {
	ItemFactoryClass ifact( in, this->cstyle );
	ReadStateClass input( &ifact, not this->no_span );
	input.setCStyleMode( this->cstyle );
	while ( not input.isAtEndOfInput() ) {
		input.reset();
		//cerr << "reset ... " << input.isPostfixAllowed() << endl;
		//cerr << "about to read" << endl;
		Node n = input.readSingleStmnt( true );
		//cerr << "read expr" << endl;
		if ( not n ) return;
		//cerr << "about to render" << endl;
		
		if ( not this->use_stdin ) {
			n->putAttribute( "source.file", this->input_file_name );
		}
		
		n->render( cout );
		//cerr << "rendered" << endl;
		cout << endl;
	}
}

void ToolMain::run( FILE * in ) {
	if ( this->gen_lnx ) {
		this->tokenise( in );
	} else {
		this->parse( in );
	}
}

int ToolMain::run() {
	openlog( this->app_title, 0, LOG_FACILITY );
	setlogmask( LOG_UPTO( LOG_INFO ) );
	
	try {
		if ( this->use_stdin ) {
			run( stdin );
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

