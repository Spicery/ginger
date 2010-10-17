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

#include <cstdio>
#include <cstdlib>

#include <unistd.h>
#include <getopt.h>

using namespace std;

#include "appcontext.hpp"
#include "rcep.hpp"
#include "term.hpp"
#include "mishap.hpp"
#include "sys.hpp"
#include "machine1.hpp"
#include "machine2.hpp"
#include "machine3.hpp"

class Main {
public:
	int run( int argc, char **argv, char **envp );
	void printGPL( const char * start, const char * end );
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
        { "cgi",            no_argument,            0, 'C' },
        { "interactive",    no_argument,            0, 'I' },
        { "batch",          no_argument,            0, 'B' },
        { "help",           optional_argument,      0, 'h' },
        { "machine",        required_argument,      0, 'm' },
        { "version",        no_argument,            0, 'v' },
        { "debug",          required_argument,      0, 'd' },
        { "license",        optional_argument,      0, 'l' },
        { 0, 0, 0, 0 }
    };

void Main::printGPL( const char * start, const char * end ) {
    bool printing = false;
    ifstream license( "LICENSE.TXT" );
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

int Main::run( int argc, char **argv, char **envp ) {
	AppContext appg;
    for(;;) {
        int option_index = 0;
        int c = getopt_long( argc, argv, "CIBhm:vd:l", long_options, &option_index );
        if ( c == -1 ) break;
        switch ( c ) {
            case 'C': {
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
                //std::string option( optarg );
                if ( std::string( optarg ) == std::string( "showcode" ) ) {
                    appg.setShowCode();
                } else if ( std::string( optarg ) == std::string( "notrap" ) ) {
                    appg.setTrappingMishap( false );
                } else if ( std::string( optarg ) == std::string( "gctrace" ) ) {
                    appg.setGCTrace( true );
                } else {
                    std::cerr << "Invalid debug option: " << optarg << std::endl;
                    exit( EXIT_FAILURE );
                }
                break;
            }
            case 'h': {
                //  Eventually we will have a "home" for our auxillary
                //  files and this will simply go there. Or run a web
                //  browser pointed there.
                if ( optarg == NULL ) {
                    printf( "Usage :  appginger [options] [files]\n\n" );
                    printf( "OPTION                SUMMARY\n" );
                    printf( "-B, --batch           run in batch mode\n" );
                    printf( "-C, --cgi             run as CGI script\n" );
                    printf( "-I, --interactive     run interactively\n" );
                    printf( "-T, --terminate       stop on mishap\n" );
                    printf( "-d, --debug           add debug option (see --help=debug)\n" );
                    printf( "-h, --help            print out this help info (see --help=help)\n" );
                    printf( "-m<n>                 run using machine #n\n" );
                    printf( "-v, --version         print out version information and exit\n" );
                    printf( "-l, --license         print out license information and exit\n" );
                    printf( "\n" );
                } else if ( std::string( optarg ) == "help" ) {
                    cout << "--help=debug          help on the debugging options available" << endl;
                    cout << "--help=help           this short help" << endl;
                    cout << "--help=licence        help on displaying license information" << endl;
                    cout << "--help=std            print out variables in std" << endl;
                } else if ( std::string( optarg ) == "debug" ) {
                    cout << "--debug=showcode      Causes the generated instructions to be displayed." << endl;
                    cout << "--debug=notrap        Prevents mishaps being caught, for use with gdb." << endl;
                    cout << "--debug=gctrace       Causes the garbage collector to emit debugging statistics." << endl;
                } else if ( std::string( optarg ) == "license" ) {
                    cout << "Displays key sections of the GNU Public License." << endl;
                    cout << "--license=warranty    Shows warranty." << endl;
                    cout << "--license=conditions  Shows terms and conditions." << endl;
                } else if ( std::string( optarg ) == std::string( "std" ) ) {
                    for (
                        SysMap::iterator it = sysMap.begin();
                        it != sysMap.end();
                        ++it
                    ) {
                        std::cout << it->first;
                        for ( int i = it->first.size(); i < 15; i++ ) {
                            std::cout << " ";
                        }
                        std::cout << "\t";
                        if ( it->second.docstring != NULL ) {
                            std::cout << it->second.docstring << std::endl;
                        } else {
                            std::cout << "-" << std::endl;
                        }
                    }
                } else if ( std::string( optarg ) == std::string( "hex" ) ) {
                	cout << hex;
                    cout << "absent         :\t" << sys_absent << endl;
                    cout << "true           :\t" << sys_true << endl;
                    cout << "false          :\t" << sys_false << endl;
                    cout << "nil            :\t" << sys_nil << endl;
					cout << "termin         :\t" << sys_termin << endl;
					cout << "system_only    :\t" << sys_system_only << endl;
					cout << "undef          :\t" << sys_undef << endl;
					cout << "Function       :\t" << sysFunctionKey << endl;
                    cout << "CoreFunction   :\t" << sysCoreFunctionKey << endl;
					cout << "Method         :\t" << sysMethodKey << endl;
					cout << "AbsentKey      :\t" << sysAbsentKey << endl;
					cout << "BoolKey        :\t" << sysBoolKey << endl;
					cout << "KeyKey         :\t" << sysKeyKey << endl;
					cout << "TerminKey      :\t" << sysTerminKey << endl;
					cout << "NilKey         :\t" << sysNilKey << endl;
					cout << "PairKey        :\t" << sysPairKey << endl;
					cout << "VectorKey      :\t" << sysVectorKey << endl;
					cout << "StringKey      :\t" << sysStringKey << endl;
					cout << "SymbolKey      :\t" << sysSymbolKey << endl;
					cout << "SmallKey       :\t" << sysSmallKey << endl;
					cout << "FloatKey       :\t" << sysFloatKey << endl;
					cout << "UnicodeKey     :\t" << sysUnicodeKey << endl;
					cout << "CharKey        :\t" << sysCharKey << endl;
					cout << "MapletKey      :\t" << sysMapletKey << endl;
					cout << "MapKey         :\t" << sysMapKey << endl;
					cout << "AssocKey       :\t" << sysAssocKey << endl;
					cout << "IndirectionKey :\t" << sysIndirectionKey << endl;
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
                cout << "appginger: version " << appg.version() << " (" << __DATE__ << " " << __TIME__ << ")" << endl;
                exit( EXIT_SUCCESS );   //  Is that right?
            }
            case 'l': {
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
            case '?': {
                break;
            }
            default: {
                printf( "?? getopt returned character code 0%x ??\n", static_cast< int >( c ) );
            }
        }
    }

    if ( appg.isInteractiveMode() ) {
        //cout << "Welcome to AppGinger (" << VERSION << ")\n";
        cout << "AppGinger " << VERSION << ", Copyright (c) 2010  Stephen Leach" << endl;
        cout << "  +----------------------------------------------------------------------+" << endl;
        cout << "  | This program comes with ABSOLUTELY NO WARRANTY. It is free software, |" << endl;
        cout << "  | and you are welcome to redistribute it under certain conditions      |" << endl;
        cout << "  | it under certain conditions. Use option --help=license for details.  |" << endl;
        cout << "  +----------------------------------------------------------------------+" << endl;
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
        Package * interactive_pkg = appg.initInteractivePackage( vm );

 
#ifdef DBG_APPCONTEXT
        clog << "RCEP ..." << endl;
#endif
        {
            RCEP rcep( interactive_pkg );
            if ( appg.isTrappingMishap() ) {
                while ( rcep.read_comp_exec_print( std::cin, std::cout ) ) {};
            } else {
                while ( rcep.unsafe_read_comp_exec_print( std::cin, std::cout ) ) {};
            }
        }
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


int main( int argc, char **argv, char **envp ) {
	Main main;
	return main.run( argc, argv, envp );
}
