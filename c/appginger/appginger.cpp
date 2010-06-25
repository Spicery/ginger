#include <iostream>

#include <stdio.h>
#include <stdlib.h>

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

enum Mode {
	InteractiveMode,
	CGIMode
};

//  struct option {
//      const char *name;   // option name
//      int has_arg;        // 0 = no arg, 1 = mandatory arg, 2 = optional arg
//      int *flag;          // variable to return result or NULL
//      int val;            // code to return (when flag is null)
//                          //  typically short option code

extern char * optarg;
static struct option long_options[] =
    {
		{ "cgi", 		no_argument, 			0, 'C' },
        { "help", 		no_argument, 			0, 'h' },
        { "machine",	required_argument, 		0, 'm' },
        { "version", 	no_argument, 			0, 'v' },
        { 0, 0, 0, 0 }
    };

int NFIB_ARG;

int main( int argc, char **argv, char **envp ) {
	enum Mode mode = InteractiveMode;
	int machine_impl_num = 1;

    initialize();

    for(;;) {
        int option_index = 0;
        int c = getopt_long( argc, argv, "Chm:v", long_options, &option_index );
        if ( c == -1 ) break;
        switch ( c ) {
			case 'C': {
                if ( !cgi_init() ) {
                  	fprintf( stderr, "cgi_init: %s\n", strerror( cgi_errno ) );
                  	exit( EXIT_FAILURE );
                }           	
				mode = CGIMode;
				break;
			}
            case 'h': {
                printf( "Usage :  appginger [options] [files]\n\n" );
                printf( "OPTION              SUMMARY\n" );
				printf( "-C, --cgi           run as CGI script\n" );
                printf( "-h, --help          print out this help info\n" );
                printf( "-m<n>               run using machine #n\n" );
                printf( "-v, --version       print out version information and exit\n" );
                printf( "\n" );
                exit( EXIT_SUCCESS );   //  Is that right?
            }
            case 'm' : {
            	machine_impl_num = atoi( optarg );
       			//printf( "Machine #%d (%s)\n", machine_impl_num, optarg );
            	break;
            }
            case 'v': {
                printf( "appginger: version " VERSION "\n" );
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

	if ( mode == InteractiveMode ) {
		
		printf( "Welcome to Ginger\n" );
		fflush( stdout );

        if ( optind < argc ) {
             printf( "non-option ARGV-elements: " );
             while ( optind < argc ) {
                printf ("%s ", argv[ optind++ ] );
             }
             printf ("\n");
        }

        //ItemFactoryClass ifact( stdin );
        
        MachineClass * vm;
        switch ( machine_impl_num ) {
        	case 1: vm = new Machine1(); break;
        	case 2: vm = new Machine2(); break;
        	case 3: vm = new Machine3(); break;
        	default:
				warning( 
					"Invalid implementation (#%d), using implementation %d\n", machine_impl_num, 1 
				);
				vm = new Machine1();
				break;
		}

#ifdef DBG_APPGINGER
		fprintf( stdout, "RCEP ...\n" );
		fflush( stdout );
#endif
		while ( read_comp_exec_print( vm, std::cin ) ) {};
    } else if ( mode == CGIMode ) {
		printf( "Content-type: text/html\r\n\r\n" );
		printf( "<html><head><title>AppGinger</title></head><body>\n" );
		printf( "<H1>AppGinger Version %s</H1>\n", VERSION );
		printf( "</body></html>\n" );
	} else {
		fprintf( stderr, "Invalid execute mode" );
		exit( EXIT_FAILURE );
	}
    return EXIT_SUCCESS;
}

