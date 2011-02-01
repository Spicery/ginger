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
//#define DBG_SIMPLIFYGNX 1

/**
The simplifygnx command is used to simplify the Ginger XML (GNX) fed
to it as a stream.
	
--absolute : replace all qualified/unqualified references to global
	variables with absolute references to their package of origin (i.e.
	defining package).

--arity : performs a basic arity analysis, tagging expressions with
	their proven arity and flagging bindings and system calls as safe.

--lifetime : tags the last use of a local variable, so the compiler can
	safely nullify. Also tags each local variable with an explicit position
	in the stack frame, the parent function with the max position number,
	and whether or not the position will be reused before the end of the
	function (which informs nullification).

--lift: performs the generalised lift transformation that ensures all
	local variables are strictly local. It will use partApply to explicitly
	replace closures, invent extra local variables, passing in explicitly
	created reference objects.
 */ 
 
#define SIMPLIFYGNX "simplifygnx"

#include <iostream>
#include <fstream>

#include <getopt.h>
#include <syslog.h>
#include <stdlib.h>
#include <list>
#include <vector>
#include <sstream>

#include "mishap.hpp"
#include "sax.hpp"
#include "gnx.hpp"
#include "command.hpp"

using namespace std;

#define FETCHGNX ( INSTALL_BIN "/fetchgnx" )

class Main {
private:
	vector< string > 	project_folders;
	
public:
	bool absolute_processing;
	bool arity_processing;
	bool lifetime_processing;
	bool lift_processing;
	std::string package;

public:
	void parseArgs( int argc, char **argv, char **envp );
	void run();
	void printGPL( const char * start, const char * end );
	std::string version();
	
public:
	Main() :
		absolute_processing( false ),
		arity_processing( false ),
		lifetime_processing( false ),
		lift_processing( false )
	{}
};

std::string Main::version() {
	return "0.1";
}


extern char * optarg;
static struct option long_options[] =
    {
		{ "absolute",		no_argument,			0, '1' },
		{ "arity",			no_argument,			0, '2' },
        { "lifetime", 		no_argument,			0, '3' },
        { "lift",			no_argument,			0, '4' },
		{ "projectfolder",	required_argument,		0, 'f' },
        { "help",			optional_argument,		0, 'H' },
        { "license",        optional_argument,      0, 'L' },
        { "package",		required_argument,		0, 'p' },
        { "version",        no_argument,            0, 'V' },
        { 0, 0, 0, 0 }
    };

void Main::parseArgs( int argc, char **argv, char **envp ) {
    for(;;) {
        int option_index = 0;
        int c = getopt_long( argc, argv, "1234f:H::L::p:V", long_options, &option_index );
        if ( c == -1 ) break;
        switch ( c ) {
        	case '1': {
        		this->absolute_processing = true;
        		break;
        	}
        	case '2': {
        		this->arity_processing = true;
        		break;
        	}
        	case '3': {
        		this->lifetime_processing = true;
        		break;
        	}
        	case '4': {
        		this->lift_processing = true;
        		break;
        	}        
        	case 'f' : {
        		this->project_folders.push_back( optarg );
        		break;
        	}
            case 'H': {
                //  Eventually we will have a "home" for our auxillary
                //  files and this will simply go there. Or run a web
                //  browser pointed there.
                if ( optarg == NULL ) {
                    printf( "Usage:  simplifygnx -p PACKJAGE OPTIONS < GNX_IN > GNX_OUT\n" );
                    printf( "OPTIONS\n" );
                    printf( "-p, --package=PACKAGE defines the enclosing package\n" );
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
            case 'p': {
            	this->package = optarg;
            	break;
            }
            case 'V': {
                cout << SIMPLIFYGNX << ": version " << this->version() << " (" << __DATE__ << " " << __TIME__ << ")" << endl;
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

class ResolveHandler : public Ginger::SaxHandler {
public:
	Ginger::Mishap mishap;
	//string enc_pkg_name;
	//string alias_name;
	string def_pkg_name;
	//bool qualified;	
        
public:
	void startTag( std::string & name, std::map< std::string, std::string > & attrs ) {
		if ( name == "resolve" ) {
			//qualified = attrs.find( "alias" ) != attrs.end();
			//this->enc_pkg_name = attrs[ "enc.pkg" ];
			//this->alias_name = attrs[ "alias" ];
			this->def_pkg_name = attrs[ "def.pkg" ];
		} else if ( name == "mishap" ) {
			this->mishap.setMessage( attrs[ "message" ] );
		} else if ( name == "culprit" ) {
			mishap.culprit( attrs[ "name" ], attrs[ "value" ] );
		} else {
			throw Ginger::Mishap( "Unexpected element in response" ).culprit( "Element name", name );
		}
	}
	
	void endTag( std::string & name ) {
		if ( name == "mishap" ) {
			throw mishap;
		}
	}

public:
	std::string defPkgName() {
		return this->def_pkg_name;
	}
	  
public:
	ResolveHandler() : mishap( "" ) {}
};

std::string resolveUnqualified( vector< string > & project_folders, const std::string & enc_pkg, const std::string & name ) {
	Ginger::Command cmd( FETCHGNX );
	cmd.addArg( "-R" );
	{
		for ( 
			vector< string >::iterator it = project_folders.begin();
			it != project_folders.end();
			++it
		) {
			cmd.addArg( "-f" );
			cmd.addArg( *it );
		}
	}
	cmd.addArg( "-p" );
	cmd.addArg( enc_pkg );
	cmd.addArg( "-v" );
	cmd.addArg( name );

	//cerr << "About to run the command" << endl;
	int fd = cmd.runWithOutput();		
	//cerr << "Command run " << endl;
	stringstream prog;
	for (;;) {
		static char buffer[ 1024 ];
		int n = read( fd, buffer, sizeof( buffer ) );
		if ( n == 0 ) break;
		prog.write( buffer, n );
	}
		
	#ifdef DBG_PACKAGE_AUTOLOAD
		cout << "[[" << prog.str() << "]]" << endl;
	#endif

	ResolveHandler resolve;
	Ginger::SaxParser parser( prog, resolve );
	parser.readElement();
	
	//resolve.report();
	
	return resolve.defPkgName();
}

std::string resolveQualified( 
	vector< string > & project_folders, 
	const std::string & enc_pkg, 
	const std::string & alias, 
	const std::string & name 
) {
	Ginger::Command cmd( FETCHGNX );
	cmd.addArg( "-R" );
	{
		for ( 
			vector< string >::iterator it = project_folders.begin();
			it != project_folders.end();
			++it
		) {
			cmd.addArg( "-f" );
			cmd.addArg( *it );
		}
	}
	cmd.addArg( "-p" );
	cmd.addArg( enc_pkg );
	cmd.addArg( "-a" );
	cmd.addArg( alias );
	cmd.addArg( "-v" );
	cmd.addArg( name );

	//cerr << "About to run the command" << endl;
	int fd = cmd.runWithOutput();		
	//cerr << "Command run " << endl;
	stringstream prog;
	for (;;) {
		static char buffer[ 1024 ];
		int n = read( fd, buffer, sizeof( buffer ) );
		if ( n == 0 ) break;
		prog.write( buffer, n );
	}
		
	#ifdef DBG_PACKAGE_AUTOLOAD
		cout << "[[" << prog.str() << "]]" << endl;
	#endif

	ResolveHandler resolve;
	Ginger::SaxParser parser( prog, resolve );
	parser.readElement();
	
	//resolve.report();
	
	return resolve.defPkgName();
}

class Absolute : public Ginger::GnxVisitor {
private:
	vector< string > & project_folders;
	std::string package;
	
	vector< int > scopes;
	vector< const string * > vars;
	
private:
	bool isGlobalScope() {
		return this->scopes.empty();
	}
	
	bool isLocalId( const std::string & name ) {
		for ( 
			vector< const string * >::iterator it = this->vars.begin();
			it != this->vars.end();
			++it
		) {
			if ( name == **it ) return true;
		}
		return false;
	}
	
public:
	void startVisit( Ginger::Gnx & element ) {
		const string & x = element.name();
		if ( x == "id" ) {
			const string & name = element.attribute( "name" );
			if ( !this->isLocalId( name ) ) {
				element.putAttribute( "scope", "global" );
				if ( not element.hasAttribute( "def.pkg" ) ) {
					const string & enc_pkg = element.hasAttribute( "enc.pkg" ) ? element.attribute( "enc.pkg" ) : this->package;
					if ( element.hasAttribute( "alias" ) ) {
						const string & alias = element.attribute( "alias" );
						cout << "RESOLVE (QUALIFIED): name=" << name << ", alias=" << alias << ", enc.pkg=" << enc_pkg << endl;
						const string def_pkg( resolveQualified( this->project_folders, enc_pkg, alias, name ) );
						cout << "   def = " << def_pkg << endl;
						element.putAttribute( "def.pkg", def_pkg );
					} else {
						cout << "RESOLVE (UNQUALIFIED): name=" << name << ", enc.pkg=" << enc_pkg << endl;
						const string def_pkg( resolveUnqualified( this->project_folders, enc_pkg, name ) );
						cout << "   def = " << def_pkg << endl;
						element.putAttribute( "def.pkg", def_pkg );						
					}
					cout << "ID: " << name << endl;
				}
			} else {
				element.putAttribute( "scope", "local" );
			}
		} else if ( x == "var" ) {
			if ( this->isGlobalScope() ) {
				element.putAttribute( "scope", "global" );
				if ( not element.hasAttribute( "def.pkg" ) ) {
					const string & name = element.attribute( "name" );
					const string & enc_pkg = element.hasAttribute( "enc.pkg" ) ? element.attribute( "enc.pkg" ) : this->package;
					cout << "RESOLVE (UNQUALIFIED): name=" << name << ", enc.pkg=" << enc_pkg << endl;
					cout << "VAR: " << name << endl;
					element.putAttribute( "def.pkg", enc_pkg );						
				}
			} else {
				element.putAttribute( "scope", "local" );
				const string & name = element.attribute( "name" );
				this->vars.push_back( &name );
			}
		} else if ( x == "fn" || x == "block" || x == "for" ) {
			this->scopes.push_back( this->vars.size() );
		}
	}
	
	void endVisit( Ginger::Gnx & element ) {
		const string & x = element.name();
		if ( x == "fn" || x == "block" || x == "for" ) {
			int n = this->scopes.back();
			this->scopes.pop_back();
			this->vars.resize( n );
		}
	}
public:
	Absolute( vector< string > & folders, const std::string & enc_pkg ) : 
		project_folders( folders ),
		package( enc_pkg )
	{
	}

	virtual ~Absolute() {}
};

void Main::run() {
	Ginger::GnxReader reader;
	shared< Ginger::Gnx > g( reader.readGnx() );
	
	//	We should walk the tree looking for: VAR & ID elements.
	Absolute a( this->project_folders, this->package );
	g->visit( a );
	
	g->render();
	cout << endl;
}

int main( int argc, char ** argv, char **envp ) {
	openlog( SIMPLIFYGNX, 0, LOG_LOCAL2 );
	setlogmask( LOG_UPTO( LOG_INFO ) );
	try {
		Main main;
		main.parseArgs( argc, argv, envp );
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
