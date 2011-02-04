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
	defining package). Marks variables as globally/locally scoped.

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

--sysapp: Replace references to system calls with <sysfn>
	and <sysapp>. It relies on absolute references and will typically
	be combined with --absolute.
	
--tailcall: Marks tail-calls.
 */ 
 
#define SIMPLIFYGNX 	"simplifygnx"
#define ARITY 			"arity"
#define TAILCALL		"tailcall"

#include <iostream>
#include <fstream>
#include <sstream>

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
#include "metainfo.hpp"

using namespace std;

#define FETCHGNX ( INSTALL_BIN "/fetchgnx" )

/*class Arity {
private:
	int 	count;
	bool 	more;
	
public:
	void add( const Arity & that ) {
		this->count += that.count;
		this->more = this->more || that.more;
	}
	
	void unify( const Arity & that ) {
		this->count = min( this->count, that.count );
		this->more = this->more || that.more || ( this->count != that.count );
	}
	
	const string toString() {
		stringstream in;
		in << ( this->count );
		if ( this->more ) {
			in << '+';
		}
		return in.str();
	}
	
public:
	Arity( int n ) :
		count( n ),
		more( false )
	{
	}
	
	Arity( const string & s ) {
		int n;
		stringstream out( s );
		out >> n;
		this->count = n;
		this->more = s.find( '+' ) != string::npos;
	}
};*/


class Main {
private:
	vector< string > 	project_folders;
	
public:
	bool absolute_processing;
	bool arity_processing;
	bool lifetime_processing;
	bool lift_processing;
	bool sysapp_processing;
	bool tailcall_processing;

public:
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
		lift_processing( false ),
		sysapp_processing( false ),
		tailcall_processing( false )
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
        { "sysapp",			no_argument,			0, '5' },
        { "tailcall",		no_argument,			0, '6' },
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
        int c = getopt_long( argc, argv, "123456f:H::L::p:V", long_options, &option_index );
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
        	case '5': {
        		this->sysapp_processing = true;
        		break;
        	}
        	case '6': {
        		this->tailcall_processing = true;
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

/*

//	Template for a Tree-walking Transformation
class TransformOfSomeKind : public Ginger::GnxVisitor {
public:
	void startVisit( Ginger::Gnx & element ) {
	}
	
	void endVisit( Ginger::Gnx & element ) {
	}
	
public:
	virtual ~TransformOfSomeKind() {}
};
*/


class ArityMarker : public Ginger::GnxVisitor {
public:
	void startVisit( Ginger::Gnx & element ) {
		element.clearAttribute( ARITY );	//	Throw away any previous marking.
	}
	
	void endVisit( Ginger::Gnx & element ) {
		if ( element.hasAttribute( "value" ) ) {
			element.putAttribute( ARITY, "1" );
		} else {
			const string & x = element.name();
			if ( x == "id" || x == "fn" ) {
				element.putAttribute( ARITY, "1" );
			} else if ( x == "for" ) {
				if ( element.hasAttribute( ARITY ) && element.attribute( ARITY ) == "0" ) {
					element.putAttribute( ARITY, "0" );
				}
			} else if ( x == "set" || x == "bind" ) {
				element.putAttribute( ARITY, "0" );
			} else if ( x == "seq" ) {
				bool all_have_arity = true;
				for ( int i = 0; all_have_arity && i < element.size(); i++ ) {
					all_have_arity = element.child( i )->hasAttribute( ARITY );
				}
				if ( all_have_arity ) {
					Ginger::Arity sofar( 0 );
					for ( int i = 0; i < element.size(); i++ ) {
						Ginger::Arity kid( element.child( i )->attribute( ARITY ) );
						sofar = sofar.add( kid );
					}
					element.putAttribute( ARITY, sofar.toString() );
				}
			} else if ( x == "if" ) {
				bool has_odd_kids = ( element.size() % 2 ) == 1;
				cout << "SIZE = " << element.size() << ", %2 = " << ( element.size() % 2 ) << endl;
				bool all_have_arity = true;
				for ( int i = 1; all_have_arity && i < element.size(); i += 2 ) {
					//cout << "CHECK " << i << endl;
					all_have_arity = element.child( i )->hasAttribute( ARITY );
				}
				if ( all_have_arity && has_odd_kids ) {
					//cout << "CHECK LAST" << endl;
					all_have_arity = element.lastChild()->hasAttribute( ARITY );
				}
				cout << "All? = " << all_have_arity << endl;
				if ( all_have_arity ) {
					const int N = element.size();
					if ( N == 0 ) {
						element.putAttribute( ARITY, "0" );
					} else if ( N == 1 ) {
						element.putAttribute( ARITY, element.child( 0 )->attribute( ARITY ) );
					} else {
						Ginger::Arity sofar( element.child( 1 )->attribute( ARITY ) );
						for ( int i = 3; i < element.size(); i += 2 ) {
							//cout << "CALC " << i << endl;
							Ginger::Arity kid( element.child( i )->attribute( ARITY ) );
							sofar = sofar.unify( kid );
						}
						if ( has_odd_kids ) {
							//cout << "CALC LAST" << endl;
							sofar = sofar.unify( Ginger::Arity( element.lastChild()->attribute( ARITY ) ) );
						}
						element.putAttribute( ARITY, sofar.toString() );
					}
					
				}
			} else if ( x == "sysapp" ) {
				element.putAttribute( ARITY, Ginger::outArity( element.attribute( "name" ) ).toString() );
			}
		} 
	}
	
public:
	virtual ~ArityMarker() {}
};



#define TAIL_CALL_MASK 		0x1

class TailCall : public Ginger::GnxVisitor {

public:
	void startVisit( Ginger::Gnx & element ) {
		const string & x = element.name();
		element.clearAttribute( TAILCALL );	//	Throw away any previous marking.
		if ( x == "fn" ) {
			//cout << "Marking last child of fn" << endl;
			element.lastChild()->orFlags( TAIL_CALL_MASK );
		} else if ( element.hasAllFlags( TAIL_CALL_MASK ) ) {
			if ( x == "app" ) {
				element.putAttribute( TAILCALL, "true" );
			} else if ( x == "if" ) {
				bool has_odd_kids = ( x.size() % 2 ) == 1;
				for ( int i = 1; i < element.size(); i += 2 ) {
					element.child( i )->orFlags( TAIL_CALL_MASK );
				}
				if ( has_odd_kids ) {
					element.lastChild()->orFlags( TAIL_CALL_MASK );
				}
			} else if ( x == "seq" || x == "block" ) {
				if ( x.size() >= 1 ) {
					element.lastChild()->orFlags( TAIL_CALL_MASK );
				}
			} 
			//	else don't push the marker down. 
			//	This includes <for>, <sysapp>, any constant,
			//	<id>, <set>
		}
	}
	
	void endVisit( Ginger::Gnx & element ) {
		element.clearFlags( TAIL_CALL_MASK );
	}
	
	
public:
	TailCall() {}

	virtual ~TailCall() {}
};

struct VarInfo {
	const string *	name;
	size_t				dec_level;
	bool 			is_protected;
	bool			is_local;
	
	bool isGlobal() {
		return not this->is_local;
	}
	
	bool isLocal() {
		return this->is_local;
	}
	
	VarInfo() :
		name( NULL ),
		is_protected( false )
	{
	}
	
	VarInfo( const string * name, size_t dec_level, bool is_protected, bool is_local ) : 
		name( name ),
		dec_level( dec_level ),
		is_protected( is_protected ),
		is_local( is_local )
	{
	}
};



/*
	This pass does the following tasks. 
	
	[1] It finds all references global variables and find their
		originating package. It caches this using the def.pkg attribute.
	
	[1] It classifies variables into global and local and caches this
		using the scope attribute. 
		
	[3] It propagates the protected="true" maplet into all <id> elements.
	
*/
class Absolute : public Ginger::GnxVisitor {
private:
	vector< string > & project_folders;
	std::string package;
	
	vector< int > scopes;
	vector< VarInfo > vars;
	
private:
	bool isGlobalScope() {
		return this->scopes.empty();
	}
	
	bool isLocalId( const std::string & name, VarInfo ** var_info ) {
		for ( 
			vector< VarInfo >::iterator it = this->vars.begin();
			it != this->vars.end();
			++it
		) {
			if ( name == *(it->name) ) {
				*var_info = &*it;
				return it->is_local;
			}
		}
		return false;
	}
	
	VarInfo * findId( const std::string & name ) {
		for ( 
			vector< VarInfo >::iterator it = this->vars.begin();
			it != this->vars.end();
			++it
		) {
			if ( name == *(it->name) ) {
				if ( it->dec_level != this->vars.size() ) {
					cout << "We have detected an outer declaration: " << name << endl;
				}
				return &*it;
			}
		}
		return NULL;
	}
	
public:
	void startVisit( Ginger::Gnx & element ) {
		const string & x = element.name();
		if ( x == "id" ) {
			const string & name = element.attribute( "name" );
			VarInfo * v = this->findId( name );
			if ( v == NULL || v->isGlobal() ) {
				element.putAttribute( "scope", "global" );
				if ( not element.hasAttribute( "def.pkg" ) ) {
					const string & enc_pkg = element.hasAttribute( "enc.pkg" ) ? element.attribute( "enc.pkg" ) : this->package;
					if ( element.hasAttribute( "alias" ) ) {
						const string & alias = element.attribute( "alias" );
						//cout << "RESOLVE (QUALIFIED): name=" << name << ", alias=" << alias << ", enc.pkg=" << enc_pkg << endl;
						const string def_pkg( resolveQualified( this->project_folders, enc_pkg, alias, name ) );
						//cout << "   def = " << def_pkg << endl;
						element.putAttribute( "def.pkg", def_pkg );
					} else {
						//cout << "RESOLVE (UNQUALIFIED): name=" << name << ", enc.pkg=" << enc_pkg << endl;
						const string def_pkg( resolveUnqualified( this->project_folders, enc_pkg, name ) );
						//cout << "   def = " << def_pkg << endl;
						element.putAttribute( "def.pkg", def_pkg );						
					}
					cout << "ID: " << name << endl;
				}
			} else {
				element.putAttribute( "scope", "local" );
			}
			if ( v != NULL && v->is_protected ) {
				element.putAttribute( "protected", "true" );
			}
		} else if ( x == "var" ) {
			const string & name = element.attribute( "name" );
			const bool is_protected = element.hasAttribute( "protected" ) && element.attribute( "protected" ) == "true";
			//cout << "NAME " << name << " is protected? " << element.hasAttribute( "protected" ) << endl;
			const bool is_global = this->isGlobalScope();
			if ( is_global ) {
				element.putAttribute( "scope", "global" );
				if ( not element.hasAttribute( "def.pkg" ) ) {
					const string & enc_pkg = element.hasAttribute( "enc.pkg" ) ? element.attribute( "enc.pkg" ) : this->package;
					//cout << "RESOLVE (UNQUALIFIED): name=" << name << ", enc.pkg=" << enc_pkg << endl;
					//cout << "VAR: " << name << endl;
					element.putAttribute( "def.pkg", enc_pkg );						
				}
			} else {
				element.putAttribute( "scope", "local" );
			}
			this->vars.push_back( VarInfo( &name, this->vars.size(), is_protected, not is_global ) );
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

/*
	This pass does the following tasks.
	
	[1] It finds all references to the ginger.library, which are guaranteed
		to be built-in functions. It replaces these references with 
		<sysfn> constants.
		
		If this transformation is successful, it does not need to force a
		further pass. The action (#2) it needs to combine with happens on the
		way up.
		
	[2] It replaces all <app><sysfn .../>...</app> forms with the especially
		efficient <sysapp>...</sysapp> form. 
		
		If this transformation is successful it triggers a further pass 
		to permit flattening of nested seqs.
		
	[3] It flattens any superfluous nested uses of <seq>. That includes
		the body of <sysapp> elements. The purpose of this is to normalise
		the code to simplify pattern recognition. 
		
		This transformation is forced on the way down the tree, which is 
		arbitrary.
		
*/
class SysFold : public Ginger::GnxVisitor {
public:
	bool again;

public:
	void startVisit( Ginger::Gnx & element ) {
		const string & x = element.name();
		if ( x == "sysapp" || x == "seq" ) {
			for ( int i = 0; i < element.size(); i++ ) {
				if ( element.child( i )->name() == "seq" ) {
					element.flattenChild( i );
					i -= 1;
				}
			}
		}
	}

	void endVisit( Ginger::Gnx & element ) {
		const string & x = element.name();
		if ( x == "id" ) {
			if ( element.hasAttribute( "def.pkg" ) && element.attribute( "def.pkg" ) == "ginger.library" ) {
				const string name( element.attribute( "name" ) );
				element.clearAllAttributes();
				element.putAttribute( "name", name );
				element.name() = "sysfn";
			}
		} else if ( x == "app" ) {
			if ( element.size() == 2 && element.child( 0 )->name() == "sysfn" ) {
				const string name( element.child( 0 )->attribute( "name" ) );
				element.name() = "sysapp";
				element.clearAllAttributes();
				element.putAttribute( "name", name );
				element.popFrontChild();
				this->again = true;	
			}
		}
	}

public:
	SysFold() : again( false ) {}
	virtual ~SysFold() {}
};


void Main::run() {
	Ginger::GnxReader reader;
	
	for (;;) {
	
		shared< Ginger::Gnx > g( reader.readGnx() );
		if ( not g ) break;
	
		if ( absolute_processing ) {
			Absolute a( this->project_folders, this->package );
			g->visit( a );
		}
	
		if ( sysapp_processing ) {	
			for (;;) {
				SysFold s;
				g->visit( s );
				if ( not s.again ) break;
			}
		}
		
		if ( tailcall_processing ) {
			g->orFlags( TAIL_CALL_MASK );
			TailCall tc;
			g->visit( tc );
		}
		
		if ( arity_processing ) {
			ArityMarker a;
			g->visit( a );
		}
		
		g->render();
		cout << endl;
		
	}
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
