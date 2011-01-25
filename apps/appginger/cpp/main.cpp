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
#include <sstream>

#include <unistd.h>
#include <getopt.h>
#include <syslog.h>

#include "appcontext.hpp"
#include "rcep.hpp"
#include "term.hpp"
#include "mishap.hpp"
#include "sys.hpp"
#include "machine1.hpp"
#include "machine2.hpp"
#include "machine3.hpp"
//#include "database.hpp"

using namespace std;

#define APPGINGER		"appginger"
#define LICENSE_FILE	( INSTALL_LIB "/LICENSE.txt" )

/*
#define GNGMETAINFO		( INSTALL_BIN "/gngmetainfo" )
*/

class Main {
private:
	AppContext context;

private:
	int printLicense( const char * arg ) const;
	void runAsCgi();
	void mainLoop();

public:
	int run( int argc, char **argv, char **envp );
	void printGPL( const char * start, const char * end ) const;
};

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
        { "cgi",            no_argument,            0, 'C' },
        { "interactive",    no_argument,            0, 'I' },
        { "batch",          no_argument,            0, 'B' },
        { "help",           optional_argument,      0, 'h' },
        { "metainfo",		no_argument,			0, 'M' },
        { "machine",        required_argument,      0, 'm' },
        { "version",        no_argument,            0, 'v' },
        { "debug",          required_argument,      0, 'd' },
        { "license",        optional_argument,      0, 'l' },
        { "project",		required_argument,		0, 'j' },
        { 0, 0, 0, 0 }
    };

void Main::printGPL( const char * start, const char * end ) const {
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
	cout << "Usage :  " << APPGINGER << " [options] [files]" << endl << endl;
	cout << "OPTION                SUMMARY" << endl;
	cout << "-B, --batch           run in batch mode" << endl;
	cout << "-C, --cgi             run as CGI script" << endl;
	cout << "-I, --interactive     run interactively" << endl;
	cout << "-T, --terminate       stop on mishap" << endl;
	cout << "-d, --debug           add debug option (see --help=debug)" << endl;
	cout << "-h, --help            print out this help info (see --help=help)" << endl;
	cout << "-f, --projectfolder   add a project folder to the search path" << endl;
	cout << "-l, --license         print out license information and exit" << endl;
	cout << "-m<n>                 run using machine #n" << endl;
	cout << "-v, --version         print out version information and exit" << endl;
	cout << endl;
}	

static void printHelpOptions() {
	cout << "--help=debug          help on the debugging options available" << endl;
	cout << "--help=help           this short help" << endl;
	cout << "--help=licence        help on displaying license information" << endl;
}

static void printHelpDebug() {
	cout << "--debug=showcode      Causes the generated instructions to be displayed." << endl;
	cout << "--debug=notrap        Prevents mishaps being caught, for use with gdb." << endl;
	cout << "--debug=gctrace       Causes the garbage collector to emit debugging statistics." << endl;
}

static void printHelpLicense() {
	cout << "Displays key sections of the GNU Public License." << endl;
	cout << "--license=warranty    Shows warranty." << endl;
	cout << "--license=conditions  Shows terms and conditions." << endl;
}

static void printHelpHex() {
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

int Main::printLicense( const char * arg ) const {
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

static void printWelcomeMessage() {
	cout << "AppGinger " << VERSION << ", Copyright (c) 2010  Stephen Leach" << endl;
	cout << "  +----------------------------------------------------------------------+" << endl;
	cout << "  | This program comes with ABSOLUTELY NO WARRANTY. It is free software, |" << endl;
	cout << "  | and you are welcome to redistribute it under certain conditions.     |" << endl;
	cout << "  | Use option --help=license for details.                               |" << endl;
	cout << "  +----------------------------------------------------------------------+" << endl;
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
		cout << "/>" << endl;
	}
	cout << "  </std>" << endl;
}

static void printBuildInfo() {
	cout << "  <release>" << endl;
	cout << "    <version "; printAttr( "number", VERSION ); cout << "/>" << endl;
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


/*
	This is a stub that is left in as a reminder to be expanded later.
*/
void Main::runAsCgi() {
	cout << "Content-type: text/html\r\n\r\n";
	cout << "<html><head><title>AppGinger</title></head><body>\n";
	cout << "<H1>AppGinger Version " << VERSION << "</H1>\n";
	cout << "</body></html>\n";
}

void Main::mainLoop() {
	MachineClass * vm = this->context.newMachine();
	Package * interactive_pkg = this->context.initInteractivePackage( vm );
 
	#ifdef DBG_APPCONTEXT
		clog << "RCEP ..." << endl;
	#endif

	RCEP rcep( interactive_pkg );
	if ( this->context.isTrappingMishap() ) {
		while ( rcep.read_comp_exec_print( std::cin, std::cout ) ) {};
	} else {
		while ( rcep.unsafe_read_comp_exec_print( std::cin, std::cout ) ) {};
	}
}

int Main::run( int argc, char **argv, char **envp ) {
	bool meta_info_needed = false;
    for(;;) {
        int option_index = 0;
        int c = getopt_long( argc, argv, "CIBMhm:vd:lf:", long_options, &option_index );
        if ( c == -1 ) break;
        switch ( c ) {
            case 'C': {
                this->context.setCgiMode();
                break;
            }
            case 'I': {
                this->context.setInteractiveMode();
                break;
            }
            case 'B': {
                this->context.setBatchMode();
                break;
            }
            case 'd': {
                //std::string option( optarg );
                if ( std::string( optarg ) == std::string( "showcode" ) ) {
                    this->context.setShowCode();
                } else if ( std::string( optarg ) == std::string( "notrap" ) ) {
                    this->context.setTrappingMishap( false );
                } else if ( std::string( optarg ) == std::string( "gctrace" ) ) {
                    this->context.setGCTrace( true );
                } else {
                    std::cerr << "Invalid debug option: " << optarg << std::endl;
                    return EXIT_FAILURE;
                }
                break;
            }
            case 'h': {
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
                return EXIT_SUCCESS;   //  Is that right?
            }
            case 'M' : {
            	meta_info_needed = true;
            	break;
            }
            case 'm' : {
                this->context.setMachineImplNum( atoi( optarg ) );
                //printf( "Machine #%d (%s)\n", machine_impl_num, optarg );
                break;
            }
            case 'v': {
                cout << "appginger: version " << this->context.version() << " (" << __DATE__ << " " << __TIME__ << ")" << endl;
                return EXIT_SUCCESS;
            }
            case 'l': {
            	return printLicense( optarg );
            }
            case 'f': {
            	this->context.addProjectFolder( optarg );
            	break;
            }
            case '?': {
                break;
            }
            default: {
                printf( "?? getopt returned character code 0%x ??\n", static_cast< int >( c ) );
            }
        }
    }

	if ( optind < argc ) {
		 cout << "non-option ARGV-elements: ";
		 while ( optind < argc ) {
		   cout << argv[ optind++ ] << " ";
		 }
		 cout << endl;
	}
	
	if ( meta_info_needed ) {
		printMetaInfo();
		return EXIT_SUCCESS;
	}


    if ( this->context.isInteractiveMode() ) {
    	printWelcomeMessage();
    }

    if ( this->context.isInteractiveMode() || this->context.isBatchMode() ) {
        this->mainLoop();
    } else if ( this->context.isCgiMode() ) {
    	this->runAsCgi();
    } else {
        fprintf( stderr, "Invalid execute mode" );
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


int main( int argc, char **argv, char **envp ) {
	openlog( APPGINGER, 0, LOG_LOCAL2 );
	setlogmask( LOG_UPTO( LOG_INFO ) );
	try {
		Main main;
		return main.run( argc, argv, envp );
	} catch ( SystemError & e ) {
		e.report();
		return EXIT_FAILURE;
	}
}
