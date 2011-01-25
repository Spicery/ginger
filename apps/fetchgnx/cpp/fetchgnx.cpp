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
		-	Fetch the GNX for a definition in the project.
		
	Optionally it will also recursively fetch definitions that are referenced
	by a definition, which will improve the initial load time.
<pre>
    fetchgnx --prj <PATH> <OPTIONS>
</pre>

	OPTIONS
	-	--prj <URL>, sets the location of the origin project
	-	--prj1 <URL>, first load for the project 
	-	--pkg <name>, sets a package as the context
	-	--pkg1 <name>, first load for the package
	-	--load, generates the initialisation for current package
	-	--def <name>, generates the GNX for the named definition of the current package
 */ 
 
#define FETCHGNX "fetchgnx"

#include <iostream>
#include <string>
#include <utility>
#include <fstream>
#include <vector>
#include <cstdlib>

#include <getopt.h>
#include <syslog.h>

//#include "database.hpp"
#include "mishap.hpp"
#include "defn.hpp"
#include "search.hpp"

using namespace std;

#define STANDARD_LIBRARY	"standard_library"

enum Task {
	FETCH_DEFINITION,
	RESOLVE_QUALIFIED,
	RESOLVE_UNQUALIFIED,
	LOAD_PACKAGE
};

class Main {
private:
	Task						task;
	vector< string >			project_folders;
	string 						package_name;
	string						alias_name;
	string						variable_name;
	std::vector< std::string > 	packages_to_load;
	std::vector< Defn > 		definitions;
	std::string					sqlite_db_file;
	
public:
	void parseArgs( int argc, char **argv, char **envp );
	void summary();
	void init();
	void run();
	void printGPL( const char * start, const char * end );
	std::string version();
	
public:
	Main() : task( FETCH_DEFINITION ) {}
};

std::string Main::version() {
	return "0.3";
}

//  struct option {
//      const char *name;   // option name
//      int has_arg;        // 0 = no arg, 1 = mandatory arg, 2 = optional arg
//      int *flag;          // variable to return result or NULL
//      int val;            // code to return (when flag is null)
//                          //  typically short option code

extern char * optarg;
static struct option long_options[] =
    {
        { "alias",			required_argument,		0, 'a' },
        { "define",         no_argument,            0, 'D' },
        { "help",			optional_argument,		0, 'H' },
        { "initial",        no_argument,            0, 'I' },
        { "license",        optional_argument,      0, 'L' },
        { "projectfolder",  required_argument,      0, 'f' },
        { "package",        required_argument,      0, 'p' },
		{ "resolve",        no_argument,            0, 'R' },
        { "qualified",		required_argument,		0, 'q' },
    	{ "sqlite",			required_argument,		0, 's' }, 	//	or --cache???
        { "unqualified",	required_argument,		0, 'u' },
        { "variable",       required_argument,      0, 'v' },
        { "version",        no_argument,            0, 'V' },
        { 0, 0, 0, 0 }
    };

void Main::parseArgs( int argc, char **argv, char **envp ) {
	bool qualified = false;
    for(;;) {
        int option_index = 0;
        int c = getopt_long( argc, argv, "RDH:IVL:f:p:a:v:", long_options, &option_index );
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
            case 'f': {
				this->project_folders.push_back( optarg );
                break;
            }
            case 'H': {
                //  Eventually we will have a "home" for our auxillary
                //  files and this will simply go there. Or run a web
                //  browser pointed there.
                if ( optarg == NULL ) {
                    printf( "Usage:  fetchgnx MODE_OPTION -s DATABASE -j PROJECT -p PACKAGE [-a ALIAS] -v VARIABLE\n" );
                    printf( "MODE OPTIONS\n" );
                    printf( "-R, --resolve         find the orgin package::variable of a reference" );
                    printf( "-D, --definition      find the definition of a package::variable" );
                    printf( "-H, --help[=TOPIC]    help info on optional topic (see --help=help)\n" );
                    printf( "-I, --initial         fetch initialisation code for a package\n" );
                    printf( "-V, --version         print out version information and exit\n" );
                    printf( "-L, --license[=PART]  print out license information and exit (see --help=license)\n" );
                    printf( "ARGUMENTS FOR -R AND -D\n" );
                    printf( "-j, --project=PATH    defines project folder, there may be more than one\n" );
                    printf( "-p, --package=NAME    sets the package name\n" );
                    printf( "-a, --alias=NAME      sets the alias name, optional\n" );
                    printf( "-v, --variable=NAME   sets the variable name\n" );
                    printf( "\n" );
                } else if ( std::string( optarg ) == "help" ) {
                    cout << "--help=help           this short help" << endl;
                    cout << "--help=licence        help on displaying license information" << endl;
                    cout << "--help=std            print out variables in std" << endl;
                } else if ( std::string( optarg ) == "license" ) {
                    cout << "Displays key sections of the GNU Public License." << endl;
                    cout << "--license=warranty    Shows warranty." << endl;
                    cout << "--license=conditions  Shows terms and conditions." << endl;
                } else {
                    printf( "Unknown help topic %s\n", optarg );
                }
                exit( EXIT_SUCCESS );   //  Is that right?
            }
            case 'I' : {
            	task = LOAD_PACKAGE;
            	break;
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
            case 'p': {
				package_name = std::string( optarg );
				break;
            }
            case 'R' : {
            	task = RESOLVE_UNQUALIFIED;
            	break;
            }
            case 's': {
            	this->sqlite_db_file = string( optarg );
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
            case '?': {
                break;
            }
            default: {
                printf( "?? getopt returned character code 0%x ??\n", static_cast< int >( c ) );
            }
        }
    }
    if ( qualified && this->task == RESOLVE_UNQUALIFIED ) {
		this->task = RESOLVE_QUALIFIED;
	}
}
    

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

#ifdef DBG_FETCHGNX
void Main::summary() {
	cout << FETCHGNX << " Summary" << endl;
	for (
		auto it = this->project_folders.begin();
		it != this->project_folders.end();
		++it
	) {
		cout << "  Project " << *it << endl;
	}
	cout << "    Needs loading? " << ( this->project_needs_loading ? "true" : "false" ) << endl;
	cout << "  Packages to load (" << this->packages_to_load.size() << ")" << endl;
	for ( 
		vector< string >::iterator it = this->packages_to_load.begin(); 
		it != this->packages_to_load.end();
		++it
	) {
		cout << "    " << *it << endl;
	}
	cout << "  Definitions to load (" << this->definitions.size() << ")" << endl;
	for (
		vector< Defn >::iterator it = this->definitions.begin();
		it != this->definitions.end();
		++it
	) {
		cout << "    " << it->pkg << ", " << it->var << endl;
	}
}
#endif

void Main::init() {
	//cout << ( INSTALL_LIB "/" STANDARD_LIBRARY ) << endl;
	this->project_folders.push_back( INSTALL_LIB "/" STANDARD_LIBRARY );
}

void Main::run() {
	Search search( this->project_folders );
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
	}
}

/*
	Search search( this->sqlite_db_file, this->project_folders );
	for (
		std::vector< std::string >::iterator it = this->packages_to_load.begin();
		it != this->packages_to_load.end();
		++it
	) {
		syslog( LOG_INFO, "Loading package %s", it->c_str() );
		search.loadPackage( *it );
	}
	for ( 
		vector< Defn >::iterator it = this->definitions.begin();
		it != this->definitions.end();
		++it
	) {
		#if DBG_FETCHGNX
			cout << it->pkg << "::" << it->var << endl;
		#endif
		try {
			syslog( LOG_INFO, "Loading definition %s from package %s", it->var.c_str(), it->pkg.c_str() );
			search.findDefinition( *it );
		} catch ( Mishap &m ) {
			//	NOTE: This is not correct because the values of package & variable must be escaped!
			cout << "<mishap message=\"" << m.getMessage() << "\">";
			int n = m.getCount();
			for ( int i = 0; i < n; i++ ) {
				pair< string, string > & p = m.getCulprit( i );
				cout << "<culprit name=\"" << p.first << "\" value=\"" << p.second << "\"/>";
			}
			cout << "</mishap>" << endl;
		}
	}
}*/
			


int main( int argc, char ** argv, char **envp ) {
	openlog( FETCHGNX, 0, LOG_LOCAL2 );
	setlogmask( LOG_UPTO( LOG_INFO ) );
	try {
		Main main;
		main.parseArgs( argc, argv, envp );
		#ifdef DBG_FETCHGNX
			main.summary();
		#endif
		main.init();
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
