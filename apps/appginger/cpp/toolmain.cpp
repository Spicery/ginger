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
#include "term.hpp"
#include "mishap.hpp"
#include "sys.hpp"
#include "machine1.hpp"
#include "machine2.hpp"
#include "machine3.hpp"
//#include "database.hpp"

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
        { "metainfo",		no_argument,			0, 'M' },
        { "machine",        required_argument,      0, 'm' },
        { "version",        no_argument,            0, 'V' },
        { "debug",          required_argument,      0, 'd' },
        { "license",        optional_argument,      0, 'L' },
        { "project",		required_argument,		0, 'j' },
        { "stdin",			no_argument,			0, 'i' },
        { "print",			no_argument,			0, 'p' },
        { "printmore",		no_argument,			0, 'P' },
        { "quiet",          no_argument,            0, 'q' },
        { "syntax",			required_argument,		0, 's' },
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
	cout << "-E<n>                 run using engine #n" << endl;
	cout << "-j, --project=PATH    add a project folder to the search path" << endl;
	cout << "-H, --help            print out this help info (see --help=help)" << endl;
	cout << "-i, --stdin           compile from stdin" << endl;
	cout << "-L, --license         print out license information and exit" << endl;
	cout << "-M, --metainfo        dump meta-info XML file to stdout" << endl;
	cout << "-p, -P                set the print level to 1 or 2" << endl;
	cout << "-q, --quiet           no welcome banner" << endl;
	cout << "-s, --syntax=LANG     select syntax" << endl;
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

static void printHelpHex() {
	cout << hex;
	cout << "absent         :\t" << SYS_ABSENT << endl;
	cout << "true           :\t" << SYS_TRUE << endl;
	cout << "false          :\t" << SYS_FALSE << endl;
	cout << "nil            :\t" << SYS_NIL << endl;
	cout << "termin         :\t" << SYS_TERMIN << endl;
	cout << "system_only    :\t" << SYS_SYSTEM_ONLY << endl;
	cout << "undef          :\t" << SYS_UNDEF << endl;
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
	cout << "HardEqMapKey   :\t" << sysHardEqMapKey << endl;
	cout << "HardIdMapKey   :\t" << sysHardIdMapKey << endl;
	cout << "WeakIdMapKey   :\t" << sysWeakIdMapKey << endl;
	cout << "CacheEqMapKey  :\t" << sysCacheEqMapKey << endl;
	cout << "AssocKey       :\t" << sysAssocKey << endl;
	cout << "IndirectionKey :\t" << sysIndirectionKey << endl;
	cout << "HardRefKey     :\t" << sysHardRefKey << endl;
	cout << "WeakRefKey     :\t" << sysWeakRefKey << endl;
	cout << "SoftRefKey     :\t" << sysSoftRefKey << endl;
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

static void renderText( std::ostream & out, const std::string & str ) {
	for ( std::string::const_iterator it = str.begin(); it != str.end(); ++it ) {
		const unsigned char ch = *it;
		if ( ch == '<' ) {
			out << "&lt;";
		} else if ( ch == '>' ) {
			out << "&gt;";
		} else if ( ch == '&' ) {
			out << "&amp;";
		} else if ( 32 <= ch && ch < 127 ) {
			out << ch;
		} else {
			out << "&#" << (int)ch << ";";
		}
	}
}

static void printAttr( const char * name, const std::string & val ) {
	cout << name << "=\"";
	renderText( cout, val );
	cout << "\" ";
}

static void printStdInfo() {
	cout << "  <std>" << endl;
	cout << "    <!-- Summary of the built-in functions -->" << endl;
	for (
		SysMap::iterator it = sysMap.begin();
		it != sysMap.end();
		++it
	) {
		cout << "    <sysfn ";
		printAttr( "name", it->first );
		printAttr( "docstring", ( it->second.docstring != NULL ? it->second.docstring : "-" ) );
		printAttr( "in", it->second.in_arity.toString() );
		printAttr( "out", it->second.out_arity.toString() );
		cout << "/>" << endl;
	}
	cout << "  </std>" << endl;
}

static void printBuildInfo() {
	cout << "  <release>" << endl;
	cout << "    <version "; printAttr( "number", APPGINGER_VERSION ); cout << "/>" << endl;
	cout << "    <build ";
	printAttr( "file", __FILE__ );
	printAttr( "date", __DATE__ ); 
	printAttr( "time", __TIME__ ); 
	cout << "/>" << endl;
	cout << "  </release>" << endl;
}

static void printLicenseInfo() {
	cout << "  <ipr>" << endl;
	cout << "    <!-- Intellectual Property Rights -->" << endl;
	cout << "    <license url=\"http://www.gnu.org/licenses/gpl-3.0.txt\" />" << endl;
	cout << "    <copyright notice=\"Copyright (c) 2010 Stephen Leach\" email=\"stephen.leach@steelypip.com\"/>" << endl;
	cout << "  </ipr>" << endl;
}

static void printCommunityInfo() {
	cout << "  <community>" << endl;
	cout << "    <!-- URLs for all the services for users & the devteam -->" << endl;
	cout << "    <!-- We are obviously missing a user website, forum and mailing list -->" << endl;
	cout << "    <repository type=\"subversion\" url=\"http://svn6.assembla.com/svn/ginger/\" />" << endl;
	cout << "    <issue_tracking type=\"trac\" url=\"http://trac6.assembla.com/ginger\" />" << endl;
	cout << "  </community>" << endl;
	
}

static void printMetaInfo() {
	cout << "<?xml version=\"1.0\"?>" << endl;
	cout << "<appginger>" << endl;
	cout << "  <!-- Information about the AppGinger executable, its toolchain or community -->" << endl;
	printBuildInfo();
	printLicenseInfo();
	printCommunityInfo();
	printStdInfo();
	cout << "</appginger>" << endl;
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

// Return true for an early exit, false to continue processing.
bool ToolMain::parseArgs( int argc, char **argv, char **envp ) {
	this->context.setEnvironmentVariables( envp );
	bool meta_info_needed = false;
    for(;;) {
        int option_index = 0;
        int c = getopt_long( argc, argv, "s:qpPiMH::m:E:Vd:L::j:", long_options, &option_index );
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
            case 'E':
            case 'm' : {
                this->context.setMachineImplNum( atoi( optarg ) );
                break;
            }
            case 'f': {
            	this->context.addProjectFolder( optarg );
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
                } else if ( std::string( optarg ) == std::string( "hex" ) ) {
                	printHelpHex();
                } else {
                    printf( "Unknown help topic %s\n", optarg );
                }
				return false;
            }
            case 'i': {
            	this->context.useStdin() = true;
            	break;
            }
            case 'L': {
            	return printLicense( optarg );
            }
            case 'M' : {
            	meta_info_needed = true;
            	break;
            }
            case 'P': {
            	this->context.printLevel() = 2;
            	break;
            }
            case 'p': {
             	this->context.printLevel() = 1;
            	break;
            }
            case 'q': {
            	this->context.welcomeBanner() = false;
            	break;
            }
            case 's': {
            	this->context.setSyntax( optarg );
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
	
	if ( meta_info_needed ) {
		printMetaInfo();
		return false;
	}
	
	return true;
}



