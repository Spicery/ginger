#include <iostream>

#include <cstdio>
#include <cstdlib>

#include <unistd.h>
#include <getopt.h>

#include "cgi.hpp"
#include "rcep.hpp"
#include "appginger.hpp"
#include "term.hpp"
#include "mishap.hpp"
#include "initialize.hpp"
#include "machine1.hpp"
#include "machine2.hpp"
#include "machine3.hpp"

using namespace std;


//  struct option {
//      const char *name;   // option name
//      int has_arg;        // 0 = no arg, 1 = mandatory arg, 2 = optional arg
//      int *flag;          // variable to return result or NULL
//      int val;            // code to return (when flag is null)
//                          //  typically short option code

extern char * optarg;
static struct option long_options[] =
    {
		{ "cgi", 			no_argument, 			0, 'C' },
		{ "interactive",	no_argument,			0, 'I' },
		{ "batch",			no_argument,			0, 'B' },
        { "help", 			optional_argument, 		0, 'h' },
        { "machine",		required_argument, 		0, 'm' },
        { "version", 		no_argument, 			0, 'v' },
        { "debug",			required_argument,		0, 'd' },
        { 0, 0, 0, 0 }
    };

int NFIB_ARG;

int main( int argc, char **argv, char **envp ) {
	AppGinger appg;

    initialize();

    for(;;) {
        int option_index = 0;
        int c = getopt_long( argc, argv, "CIBhm:v", long_options, &option_index );
        if ( c == -1 ) break;
        switch ( c ) {
			case 'C': {
                if ( !cgi_init() ) {
                  	fprintf( stderr, "cgi_init: %s\n", strerror( cgi_errno ) );
                  	exit( EXIT_FAILURE );
                }           	
                appg.setCgiMode();
				break;
			}
			case 'I': {
				appg.setInteractiveMode();
				break;
			}
			case 'B': {
				appg.setBatchMode();
				break;
			}
			case 'd': {
				if ( std::string( optarg ) == std::string( "showcode" ) ) {
					appg.setShowCode();
				}
				break;
			}
            case 'h': {
            	//	Eventually we will have a "home" for our auxillary
            	//	files and this will simply go there. Or run a web
            	//	browser pointed there.
            	if ( optarg == NULL ) {
					printf( "Usage :  appginger [options] [files]\n\n" );
					printf( "OPTION              SUMMARY\n" );
					printf( "-B, --batch         run in batch mode\n" );
					printf( "-C, --cgi           run as CGI script\n" );
					printf( "-I, --interactive   run interactively\n" );
					printf( "-d, --debug         add debug option (see --help=debug)\n" );
					printf( "-h, --help          print out this help info\n" );
					printf( "-m<n>               run using machine #n\n" );
					printf( "-v, --version       print out version information and exit\n" );
					printf( "\n" );
				} else if ( std::string( optarg ) == "debug" ) {
					printf( "--debug=showcode    Causes the generated instructions to be displayed.\n" );
                } else {
                	printf( "Unknown help topic %s\n", optarg );
                }
                exit( EXIT_SUCCESS );   //  Is that right?
            }
            case 'm' : {
            	appg.setMachineImplNum( atoi( optarg ) );
       			//printf( "Machine #%d (%s)\n", machine_impl_num, optarg );
            	break;
            }
            case 'v': {
            	cout << "appginger: version " << appg.version() << endl;
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

	if ( appg.isInteractiveMode() ) {
		cout << "Welcome to Ginger (" << VERSION << ")\n";
	}

	if ( appg.isInteractiveMode() || appg.isBatchMode() ) {
        if ( optind < argc ) {
             cout << "non-option ARGV-elements: ";
             while ( optind < argc ) {
               cout << argv[ optind++ ] << " ";
             }
             cout << endl;
        }
        
        MachineClass * vm = appg.newMachine();
 
#ifdef DBG_APPGINGER
		clog << "RCEP ..." << endl;
#endif
		while ( read_comp_exec_print( vm, std::cin ) ) {};
    } else if ( appg.isCgiMode() ) {
		cout << "Content-type: text/html\r\n\r\n";
		cout << "<html><head><title>AppGinger</title></head><body>\n";
		cout << "<H1>AppGinger Version " << VERSION << "</H1>\n";
		cout << "</body></html>\n";
	} else {
		fprintf( stderr, "Invalid execute mode" );
		exit( EXIT_FAILURE );
	}
    return EXIT_SUCCESS;
}

MachineClass * AppGinger::newMachine() {
   switch ( this->machine_impl_num ) {
		case 1: return new Machine1( *this );
		case 2: return new Machine2( *this );
		case 3: return new Machine3( *this );
		default: {
			warning( 
				"Invalid implementation (#%d), using implementation %d\n", this->machine_impl_num, 1 
			);
			return new Machine1( *this );
			break;
		}
	}
}