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

//	Local debugging switch for conditional code compilation.
//#define DBG_FETCHGNX 1

/**
	The fetchgnx command is used to fetch the Ginger XML (GNX) contents of a 
	project. It can
	
		-	Fetch the GNX to initialise a project.
		-	Resolve a variable reference in package A to a definition in package B
		-	Fetch the GNX for a definition in the project.
		
 */ 
 
#define FETCHGNX "fetchgnx"

#include <iostream>
#include <string>
#include <utility>
#include <fstream>
#include <vector>
#include <cstdlib>

#include <stddef.h>
#include <getopt.h>
#include <syslog.h>

#include "printgpl.hpp"
#include "mnx.hpp"
#include "mishap.hpp"
#include "defn.hpp"
#include "search.hpp"

using namespace std;

#define STANDARD_LIBRARY	"standard_library"

//#define DBG_FETCHGNX

enum Task {
	FETCH_DEFINITION,
	RESOLVE_QUALIFIED,
	RESOLVE_UNQUALIFIED,
	LOAD_PACKAGE,
	LOAD_FILE,
	EXECUTE_COMMAND
};

class Main {
private:
	Task						task;
	vector< string >			project_folders;
	string 						load_file;
	string 						package_name;
	string						alias_name;
	string						variable_name;
	std::vector< std::string > 	packages_to_load;
	std::vector< Defn > 		definitions;
	bool						undefined_allowed;
	
public:
	void parseArgs( int argc, char **argv, char **envp );
	void summary();
	void init();
	void run();
	std::string version();
	
public:
	Main() : task( FETCH_DEFINITION ), undefined_allowed( false ) {}
};

std::string Main::version() {
	return "0.5";
}

extern char * optarg;
static struct option long_options[] =
    {
        { "alias",			required_argument,		0, 'a' },
        { "define",         no_argument,            0, 'D' },
        { "execute",		no_argument,            0, 'X' },
        { "help",			optional_argument,		0, 'H' },
        { "initial",        no_argument,            0, 'I' },
        { "license",        optional_argument,      0, 'L' },
        { "load",			required_argument,      0, 'l' },
        { "folder",         required_argument,  	0, 'f' },
        { "project",        required_argument,      0, 'j' },
        { "package",        required_argument,      0, 'p' },
		{ "resolve",        no_argument,            0, 'R' },
        { "undefined",		required_argument,		0, 'u' },
        { "variable",       required_argument,      0, 'v' },
        { "version",        no_argument,            0, 'V' },
        { 0, 0, 0, 0 }
    };

static void printUsage() {
	cout << "Usage:  fetchgnx MODE_OPTION -j PROJECT -p PACKAGE [-a ALIAS] -v VARIABLE" << endl;
	cout << "MODE OPTIONS" << endl;
	cout << "-R, --resolve         find the origin package::variable of a reference" << endl;
	cout << "-D, --definition      find the definition of a package::variable" << endl;
	cout << "-H, --help[=TOPIC]    help info on optional topic (see --help=help)" << endl;
	cout << "-I, --initial         fetch initialisation code for a package" << endl;
	cout << "-V, --version         print out version information and exit" << endl;
	cout << "-L, --license[=PART]  print out license information and exit (see --help=license)" << endl;
	cout << "ARGUMENTS FOR -R AND -D" << endl;
	cout << "-j, --project=PATH    defines project folder, there may be more than one" << endl;
	cout << "-f, --folder=PATH     alternative to --project option" << endl;
	cout << "-p, --package=NAME    sets the package name" << endl;
	cout << "-a, --alias=NAME      sets the alias name, optional" << endl;
	cout << "-v, --variable=NAME   sets the variable name" << endl;
	cout << "-u, --undefined       allow undefined variables" << endl;
	cout << endl;
}

static void printHelpOptionUsage() {
    cout << "--help=help           this short help" << endl;
    cout << "--help=licence        help on displaying license information" << endl;
    cout << "--help=std            print out variables in std" << endl;
}

static void printLicenseOptionUsage() {
	cout << "Displays key sections of the GNU Public License." << endl;
	cout << "--license=warranty    Shows warranty." << endl;
	cout << "--license=conditions  Shows terms and conditions." << endl;
}

void Main::parseArgs( int argc, char **argv, char **envp ) {
	bool qualified = false;
    for(;;) {
        int option_index = 0;
        int c = getopt_long( argc, argv, "uXRDH::IVL::l:j:f:p:a:v:", long_options, &option_index );
        if ( c == -1 ) break;
        switch ( c ) {
            case 'a' : {
            	alias_name = optarg;
            	qualified = true;
            	break;
            }
            case 'D': {
            	task = FETCH_DEFINITION;
            	break;
            }
            case 'j':
            case 'f': {
				this->project_folders.push_back( optarg );
                break;
            }
            case 'H': {
                //  Eventually we will have a "home" for our auxillary
                //  files and this will simply go there. Or run a web
                //  browser pointed there.
                if ( optarg == NULL ) {
                	printUsage();
                } else if ( std::string( optarg ) == "help" ) {
                	printHelpOptionUsage();
                } else if ( std::string( optarg ) == "license" ) {
                	printLicenseOptionUsage();
                } else {
                    cout << "Unknown help topic " << optarg << endl;
                }
                exit( EXIT_SUCCESS );   //  Is that right?
            }
            case 'I' : {
            	task = LOAD_PACKAGE;
            	break;
            }
            case 'L': {
            	exit( Ginger::optionPrintGPL( optarg ) );
            }
            case 'l': {
            	task = LOAD_FILE;
            	this->load_file = std::string( optarg );
            	break;
            }
            case 'p': {
				package_name = std::string( optarg );
				break;
            }
            case 'R' : {
            	task = RESOLVE_UNQUALIFIED;
            	break;
            }
            case 'u': {
            	this->undefined_allowed = true;
            	break;
            }
            case 'v': {
            	variable_name = string( optarg );
                break;
            }
            case 'V': {
                cout << FETCHGNX << ": version " << this->version() << " (" << __DATE__ << " " << __TIME__ << ")" << endl;
                exit( EXIT_SUCCESS );   //  Is that right?
            }
            case 'X': {
            	task = EXECUTE_COMMAND;
            	break;
            }
            case '?': {
                break;
            }
            default: {
                cout << "?? getopt returned character code " << hex << static_cast< int >( c ) << dec << endl;
            }
        }
    }
    if ( qualified && this->task == RESOLVE_UNQUALIFIED ) {
		this->task = RESOLVE_QUALIFIED;
	}
}
    

#ifdef DBG_FETCHGNX
void Main::summary() {
	cerr << FETCHGNX << " Summary" << endl;
	cerr << "  Mode: ";
	switch ( this->task ) {
		case RESOLVE_QUALIFIED: {
			cerr << "RESOLVE_QUALIFIED" << endl;
			cerr << "  Package name  : " << this->package_name;
			cerr << "  Alias name    : " << this->alias_name;
			cerr << "  Variable name : " << this->variable_name;
			break;
		}
		case RESOLVE_UNQUALIFIED: {
			cerr << "RESOLVE_UNQUALIFIED" << endl;
			cerr << "  Package name  : " << this->package_name << endl; 
			cerr << "  Variable name : " << this->variable_name << endl;
			break;
		}
		case FETCH_DEFINITION : {
			cerr << "FETCH_DEFINITION" << endl;
			break;
		}
		case LOAD_PACKAGE: {
			cerr << "LOAD_PACKAGE" << endl;
			cerr << "  Package name  : " << this->package_name << endl; 
			break;
		}
		case LOAD_FILE: {
			cerr << "LOAD_FILE" << endl;
			cerr << "  Package name  : " << this->package_name << endl; 
			cerr << "  Load file name  : " << this->load_file << endl; 
		}
		case EXECUTE_COMMAND: {
			cerr << "EXECUTE_COMMAND" << endl;
			break;
		}
		default:
			throw Mishap( "Invalid mode" );
	}
	cerr << "  Project folders (" << this->project_folders.size() << ")" << endl;
	for (
		vector< string >::iterator it = this->project_folders.begin();
		it != this->project_folders.end();
		++it
	) {
		cerr << "    Project " << *it << endl;
	}
	cerr << "  Packages to load (" << this->packages_to_load.size() << ")" << endl;
	for ( 
		vector< string >::iterator it = this->packages_to_load.begin(); 
		it != this->packages_to_load.end();
		++it
	) {
		cerr << "    " << *it << endl;
	}
	cerr << "  Definitions to load (" << this->definitions.size() << ")" << endl;
	for (
		vector< Defn >::iterator it = this->definitions.begin();
		it != this->definitions.end();
		++it
	) {
		cerr << "    " << it->pkg << ", " << it->var << endl;
	}
}
#endif

void Main::init() {
	//cout << ( INSTALL_LIB "/" STANDARD_LIBRARY ) << endl;
	this->project_folders.push_back( INSTALL_LIB "/" STANDARD_LIBRARY );
}

void Main::run() {
	Search search( this->project_folders, this->undefined_allowed );
	switch ( this->task ) {
		case RESOLVE_QUALIFIED: {
			search.resolveQualified( this->package_name, this->alias_name, this->variable_name );
			break;
		}
		case RESOLVE_UNQUALIFIED: {
			search.resolveUnqualified( this->package_name, this->variable_name );
			break;
		}
		case FETCH_DEFINITION : {
			search.fetchDefinition( this->package_name, this->variable_name );
			break;
		}
		case LOAD_PACKAGE: {
			search.loadPackage( this->package_name );
			break;
		}
		case LOAD_FILE: {
			search.loadFileFromPackage( this->package_name, this->load_file );
			break;
		}
		case EXECUTE_COMMAND: {
			#ifdef DBG_FETCHGNX
				cerr << "Entering Execute Command mode" << endl;
			#endif
			Ginger::MnxReader reader( std::cin );
			shared< Ginger::Mnx > mnx = reader.readMnx();
			#ifdef DBG_FETCHGNX
				cerr << "Read GNX" << endl;
			#endif
			if ( not mnx ) {
				#ifdef DBG_FETCHGNX
					cerr << "Nothing read" << endl;
				#endif
			} else if ( 
				mnx->hasName( "resolve.gnx" ) &&
				mnx->size() == 1
			) {
				search.resolveGnxInPlace( mnx->getFirstChild() );
			} else if ( 
				mnx->hasName( "resolve.qualified" ) && 
				mnx->hasAttribute( "pkg.name" ) &&
				mnx->hasAttribute( "alias.name" ) &&
				mnx->hasAttribute( "var.name" )
			) {
				search.resolveQualified( mnx->attribute( "pkg.name" ), mnx->attribute( "alias.name" ), mnx->attribute( "var.name" ) );	
			} else if ( 
				mnx->hasName( "resolve.unqualified" ) &&
				mnx->hasAttribute( "pkg.name" ) &&
				mnx->hasAttribute( "var.name" )
			) {
				#ifdef DBG_FETCHGNX
					cerr << "Resolve unqualified reference" << endl;
				#endif
				search.resolveUnqualified( mnx->attribute( "pkg.name" ), mnx->attribute( "var.name" ) );
			} else if ( 
				mnx->hasName( "fetch.definition" ) &&
				mnx->hasAttribute( "pkg.name" ) &&
				mnx->hasAttribute( "var.name" )
			) {
				search.fetchDefinition( mnx->attribute( "pkg.name" ), mnx->attribute( "var.name" ) );
			} else if ( 
				mnx->hasName( "fetch.pkg.init" ) &&
				mnx->hasAttribute( "pkg.name" )
			) {
				search.loadPackage( mnx->attribute( "pkg.name" ) );
			} else if (
				mnx->hasName( "fetch.load.file" ) &&
				mnx->hasAttribute( "pkg.name" ) &&
				mnx->hasAttribute( "load.file" )
			) {
				search.loadFileFromPackage( mnx->attribute( "pkg.name" ), mnx->attribute( "load.file" ) );
			} else if (
				mnx->hasName( "browse.packages" )
			) {
				search.browsePackages();
			} else {
				throw Ginger::Mishap( "Invalid request" ).culprit( mnx->toString() );
			}
			break;
		}
	}
}

int main( int argc, char ** argv, char **envp ) {
	openlog( FETCHGNX, 0, LOG_LOCAL2 );
	setlogmask( LOG_UPTO( LOG_INFO ) );
	try {
		Main main;
		main.parseArgs( argc, argv, envp );
		main.init();
		#ifdef DBG_FETCHGNX
			main.summary();
		#endif
		main.run();
	    return EXIT_SUCCESS;
	} catch ( Ginger::Mishap & p ) {
		p.culprit( "Detected by", FETCHGNX );
		p.gnxReport();
		return EXIT_FAILURE;
	}
}
