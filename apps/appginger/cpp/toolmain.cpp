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
#include <vector>
#include <string>

#include <cstdio>
#include <cstdlib>
#include <sstream>

#include <ext/stdio_filebuf.h> // __gnu_cxx::stdio_filebuf

#include <unistd.h>
#include <getopt.h>
#include <syslog.h>

#include "gngversion.hpp"
#include "mnx.hpp"

#include "appcontext.hpp"
#include "rcep.hpp"
#include "mishap.hpp"
#include "sys.hpp"
#include "machine1.hpp"
#include "machine2.hpp"
#include "machine3.hpp"


#include "toolmain.hpp"

using namespace std;

#define LICENSE_FILE	( INSTALL_LIB "/COPYING" )

#define SIMPLIFYGNX		( INSTALL_TOOL "/simplifygnx" )
#define COMMON2GNX		( INSTALL_TOOL "/common2gnx" )
#define LISP2GNX		( INSTALL_TOOL "/lisp2gnx" )
#define GSON2GNX		( INSTALL_TOOL "/gson2gnx" )
#define TAIL			"/usr/bin/tail"


/*
	This is the structure of struct option, which does not seem to be
	especially well documented. Included for handy reference.
	struct option {
		const char *name;   // option name
		int has_arg;        // 0 = no arg, 1 = mandatory arg, 2 = optional arg
		int *flag;          // variable to return result or NULL
		int val;            // code to return (when flag is null)
							//  typically short option code
*/

extern char * optarg;
static struct option long_options[] =
    {
        { "engine",         required_argument,      0, 'E' },
        { "help",           optional_argument,      0, 'H' },
        { "machine",        required_argument,      0, 'm' },
        { "version",        no_argument,            0, 'V' },
        { "debug",          required_argument,      0, 'd' },
        { "grammar",		required_argument,		0, 'g' },
        { "license",        optional_argument,      0, 'L' },
        { "project",		required_argument,		0, 'j' },
        { "stdin",			no_argument,			0, 'i' },
        { "quiet",          no_argument,            0, 'q' },
        { "results",		required_argument,		0, 'r' },
        { 0, 0, 0, 0 }
    };

void ToolMain::printGPL( const char * start, const char * end ) const {
    bool printing = false;
    ifstream license( LICENSE_FILE );
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

static void printUsage() {
	cout << "Usage :  " << PACKAGE_NAME << " [options] [files]" << endl << endl;
	cout << "OPTION                SUMMARY" << endl;
	cout << "-d, --debug=OPTION    add debug option (see --help=debug)" << endl;
	cout << "-e, --engine=<N>      run using engine #n" << endl;
	cout << "-g, --grammar=LANG    select front-end syntax" << endl;
	cout << "-H, --help            print out this help info (see --help=help)" << endl;
	cout << "-i, --stdin           compile from stdin" << endl;
	cout << "-j, --project=PATH    add a project folder to the search path" << endl;
	cout << "-L, --license         print out license information and exit" << endl;
	//cout << "-M, --metainfo        dump meta-info XML file to stdout" << endl;
	cout << "-q, --quiet           no welcome banner" << endl;
	cout << "-r, --results=LEVEL   set results level to 1 or 2" << endl;
	cout << "-V, --version         print out version information and exit" << endl;
	cout << endl;
}	

static void printHelpOptions() {
	cout << "--help=debug          help on the debugging options available" << endl;
	cout << "--help=help           this short help" << endl;
	cout << "--help=licence        help on displaying license information" << endl;
}

static void printHelpDebug() {
	cout << "--debug=showcode      Causes the generated instructions to be displayed." << endl;
	cout << "--debug=gctrace       Causes the garbage collector to emit debugging statistics." << endl;
}

static void printHelpLicense() {
	cout << "Displays key sections of the GNU Public License." << endl;
	cout << "--license=warranty    Shows warranty." << endl;
	cout << "--license=conditions  Shows terms and conditions." << endl;
}


int ToolMain::printLicense( const char * arg ) const {
	if ( arg == NULL || std::string( arg ) == std::string( "all" ) ) {
		this->printGPL( NULL, NULL );
	} else if ( std::string( arg ) == std::string( "warranty" ) ) {
		this->printGPL( "Disclaimer of Warranty.", "Limitation of Liability." );                 
	} else if ( std::string( arg ) == std::string( "conditions" ) ) {
		this->printGPL( "TERMS AND CONDITIONS", "END OF TERMS AND CONDITIONS" );
	} else {
		std::cerr << "Unknown license option: " << arg << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


string ToolMain::shellSafeName( const string & filename ) {
	string safe;
	for ( string::const_iterator it = filename.begin(); it != filename.end(); ++it ) {
		const char ch = *it;
		if ( not isalnum( ch ) ) {
			safe.push_back( '\\' );
		}
		safe.push_back( ch );
	}
	return safe;
}

//	We will get to use this later, when we add the -x,--xargs option.
template < class ContainerT >
void tokenize(
    const std::string& str, 
    ContainerT & tokens,
    const std::string& delimiters = " ", 
    const bool trimEmpty = false
) {
    std::string::size_type pos, lastPos = 0;
    for (;;) {
        pos = str.find_first_of( delimiters, lastPos );
        if ( pos == std::string::npos) {
            pos = str.length();
            if ( pos != lastPos || not trimEmpty ) {
                tokens.push_back(
                	typename ContainerT::value_type(
                		str.data() + lastPos,
                  		pos - lastPos 
                  	)
            	);
            }

            break;
        }

        if ( pos != lastPos || not trimEmpty ) {
            tokens.push_back(
            	typename ContainerT::value_type(
            		str.data() + lastPos,
              		pos - lastPos 
              	)
        	);
        }

        lastPos = pos + 1;
    }
};

// Return true for an early exit, false to continue processing.
bool ToolMain::parseArgs( int argc, char **argv, char **envp ) {
	if ( envp != NULL ) this->context.setEnvironmentVariables( envp );
	//bool meta_info_needed = false;
    for(;;) {
        int option_index = 0;
        //int c = getopt_long( argc, argv, "d:e:g:H::ij:L::Mm:qr:V", long_options, &option_index );
        int c = getopt_long( argc, argv, "d:e:g:H::ij:L::m:qr:V", long_options, &option_index );
        //cerr << "Got c = " << c << endl;
        if ( c == -1 ) break;
        switch ( c ) {
            case 'd': {
                //std::string option( optarg );
                if ( std::string( optarg ) == std::string( "showcode" ) ) {
                    this->context.setShowCode();
                } else if ( std::string( optarg ) == std::string( "gctrace" ) ) {
                    this->context.setGCTrace( true );
                } else {
                    std::cerr << "Invalid debug option: " << optarg << std::endl;
                    return false;
                }
                break;
            }
            case 'e':
            case 'm' : {
                this->context.setMachineImplNum( atoi( optarg ) );
                break;
            }
            case 'g': {
            	this->context.setSyntax( optarg );
            	break;
            }
            case 'H': {
                //  Eventually we will have a "home" for our auxillary
                //  files and this will simply go there. Or run a web
                //  browser pointed there.
                if ( optarg == NULL ) {
                	printUsage();
                } else if ( std::string( optarg ) == "help" ) {
                	printHelpOptions();
                } else if ( std::string( optarg ) == "debug" ) {
                	printHelpDebug();
                } else if ( std::string( optarg ) == "license" ) {
                	printHelpLicense();
                } else {
                    printf( "Unknown help topic %s\n", optarg );
                }
				return false;
            }
            case 'i': {
            	this->context.useStdin() = true;
            	break;
            }
            case 'j': {
            	this->context.addProjectFolder( optarg );
            	break;
            }
            case 'L': {
            	return printLicense( optarg );
            }
            case 'q': {
            	this->context.welcomeBanner() = false;
            	break;
            }
            case 'r': {
            	int level = atoi( optarg );
            	this->context.printDetailLevel() = level;
            	break;
            }
            case 'V': {
                cout << this->appName() << ": version " << this->context.version() << " (" << __DATE__ << " " << __TIME__ << ")" << endl;
                return false;
            }
            case '?': {
            	//	Invalid option: exit.
                return false;
            }
            default: {
            	//	This should not happen. It indicates that the option string 
            	//	does not conform to the cases of this switch statement.
            	throw Ginger::SystemError( "Unrecognised option" ).culprit( "Option code", static_cast< long >( c ) );
            }
        }
    }

	//	Aggregate the remaining arguments, which are effectively filenames (paths).
	if ( optind < argc ) {
		 while ( optind < argc ) {
		   	this->context.addArgument( argv[ optind++ ] );
		 }
	}
	
	return true;
}
