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
 
 

#include <iostream>
#include <string>
#include <utility>
#include <fstream>
#include <vector>

#include <stdlib.h>
#include <getopt.h>

#include "mishap.hpp"
#include "search.hpp"

using namespace std;

class Main {
private:
	string project;
	bool project_needs_loading;
	std::vector< std::string > packages_to_load;
	std::vector< std::pair< std::string, std::string > > definitions;
	
public:
	void parseArgs( int argc, char **argv, char **envp );
	void summary();
	void run();
	void printGPL( const char * start, const char * end );
	std::string version();
};

std::string Main::version() {
	return "0.1";
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
        { "project",        required_argument,      0, 'j' },
        { "project1",       required_argument,     	0, 'J' },
        { "package",        required_argument,      0, 'p' },
        { "package1",       required_argument,  	0, 'P' },
        { "variable",       required_argument,      0, 'v' },
        { "version",        no_argument,            0, 'V' },
        { "help",			optional_argument,		0, 'H' },
        { "license",        optional_argument,      0, 'L' },
        { 0, 0, 0, 0 }
    };

void Main::parseArgs( int argc, char **argv, char **envp ) {
	std::string current_package;
    for(;;) {
        int option_index = 0;
        int c = getopt_long( argc, argv, "j:J:p:P:v:VH:L:", long_options, &option_index );
        if ( c == -1 ) break;
        switch ( c ) {
        	case 'J': 
            case 'j': {
            	if ( this->project.size() != 0 ) {
            		throw Mishap( "Project being defined twice" );	
            	}
				this->project = std::string( optarg );
				this->project_needs_loading = ( c == 'P' );
                break;
            }
			case 'P':
            case 'p': {
				current_package = std::string( optarg );
				if ( c == 'P' ) {
					this->packages_to_load.push_back( current_package );
				}
				break;
            }
            case 'v': {
            	this->definitions.push_back( pair< string, string >( current_package, string( optarg ) ) );
                break;
            }
            case 'H': {
                //  Eventually we will have a "home" for our auxillary
                //  files and this will simply go there. Or run a web
                //  browser pointed there.
                if ( optarg == NULL ) {
                    printf( "Usage :  appginger [options] [files]\n\n" );
                    printf( "OPTION                SUMMARY\n" );
                    printf( "-h, --help            print out this help info (see --help=help)\n" );
                    printf( "-v, --version         print out version information and exit\n" );
                    printf( "-l, --license         print out license information and exit\n" );
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
            case 'V': {
                cout << "fetchgnx: version " << this->version() << " (" << __DATE__ << " " << __TIME__ << ")" << endl;
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
            case '?': {
                break;
            }
            default: {
                printf( "?? getopt returned character code 0%x ??\n", static_cast< int >( c ) );
            }
        }
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

void Main::summary() {
	cout << "FetchGNX Summary" << endl;
	cout << "  Project " << this->project << endl;
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
		vector< pair< string, string > >::iterator it = this->definitions.begin();
		it != this->definitions.end();
		++it
	) {
		cout << "    " << it->first << ", " << it->second << endl;
	}
}

void Main::run() {
	Search search( this->project );
	for ( 
		vector< pair< string, string > >::iterator it = this->definitions.begin();
		it != this->definitions.end();
		++it
	) {
		#if DBG_FETCHGNX
			cout << it->first << "::" << it->second << endl;
		#endif
		try {
			search.find_definition( it->first, it->second );
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
}
			


int main( int argc, char ** argv, char **envp ) {
	try {
		Main main;
		main.parseArgs( argc, argv, envp );
		#ifdef DBG_FETCHGNX
			main.summary();
		#endif
		main.run();
	    return EXIT_SUCCESS;
	} catch ( Problem & p ) {
		p.report();
		return EXIT_FAILURE;
	}
}
