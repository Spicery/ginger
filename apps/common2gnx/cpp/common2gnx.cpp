#include <iostream>
#include <fstream>

#include <cstdio>
#include <cstdlib>

#include <syslog.h>

#include <getopt.h>

#include "gngversion.hpp"

#include "mishap.hpp"
#include "item_factory.hpp"
#include "read_expr.hpp"

using namespace std;

#define APP_TITLE 		"common2gnx"
#define LOG_FACILITY	LOG_LOCAL2

/*int main( int argc, char **argv, char **envp ) {
	openlog( APP_TITLE, 0, LOG_FACILITY );
	setlogmask( LOG_UPTO( LOG_INFO ) );

	try {
		FILE * in;
		if ( argc == 2 ) {
			in = fopen( argv[1], "r" ); 
			if ( in == NULL ) throw Mishap( "Cannot open file" ).culprit( "Filename", argv[1] );
			syslog( LOG_INFO, "Converting %s", argv[1] );
		} else {
			in = stdin;
			syslog( LOG_INFO, "Converting standard input (%s)", argv[1] );
		}
		for (;;) {
			ItemFactoryClass ifact( in );
			ReadStateClass input( &ifact );
			for (;;) {
				Node n = input.read_opt_expr();
				if ( not n ) return EXIT_SUCCESS;
				n->render( cout );
				cout << endl;
				input.checkSemi();
			};
		} 
	} catch ( Mishap & m ) {
		m.gnxReport();
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}*/


extern char * optarg;
static struct option long_options[] =
    {
    	{ "help",			optional_argument,		0, 'H' },
        { "license",        optional_argument,      0, 'L' },
        { "tokenise",			no_argument,		0, 'T' },
        { "version",        no_argument,            0, 'V' },
        { 0, 0, 0, 0 }
    };

class Main {
private:
	bool use_stdin;
	string input_file_name;
	bool gen_lnx;

public:
	string version() {
		return "0.2";
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
		this->gen_lnx = false;
		this->use_stdin = true;
		
		for(;;) {
			int option_index = 0;
			int c = getopt_long( argc, argv, "TH::L::V", long_options, &option_index );
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
						printf( "Usage:  %s OPTIONS < LNX_IN > CSV_OUT\n", APP_TITLE );
						printf( "OPTIONS\n" );
						printf( "-T, --tokenise        tokenise only, generate LNX output instead of GNX\n" );
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
					cout << APP_TITLE << ": version " << this->version() << " (" << __DATE__ << " " << __TIME__ << ") part of AppGinger version " << APPGINGER_VERSION << endl;
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
	void tokenise( FILE *in ) {
		cout << "<item.stream V=\"\" R=\"\" T=\"\" P=\"\" L=\"\">" << endl;
		for (;;) {
			ItemFactoryClass ifact( in );
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

	void parse( FILE * in ) {
		for (;;) {
			ItemFactoryClass ifact( in );
			ReadStateClass input( &ifact );
			for (;;) {
				Node n = input.read_opt_expr();
				if ( not n ) return;
				n->render( cout );
				cout << endl;
				input.checkSemi();
			};
		} 
	}

	void run( FILE * in ) {
		if ( this->gen_lnx ) {
			this->tokenise( in );
		} else {
			this->parse( in );
		}
	}

public:
	int run() {
		openlog( APP_TITLE, 0, LOG_FACILITY );
		setlogmask( LOG_UPTO( LOG_INFO ) );
	
		try {
			if ( this->use_stdin ) {
				run( stdin );
			} else {
				const char * fname = input_file_name.c_str();
				FILE * in = fopen( fname, "r" ); 
				if ( in == NULL ) throw Mishap( "Cannot open file" ).culprit( "Filename", fname );
				run( in );
			}
	
		} catch ( Mishap & m ) {
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
	} catch ( Mishap & p ) {
		p.report();
		return EXIT_FAILURE;
	}
}
