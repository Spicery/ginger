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
	Propagates any protected markings.

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

//	These are attributes that are used to annotate the tree. Should
//	be reserved to SimplifyGNX.
#define ARITY 			"arity"
#define TAILCALL		"tailcall"
#define UID				"uid"
#define OUTER_LEVEL		"outer.level"
#define IS_OUTER		"is.outer"
#define SCOPE			"scope"
#define PROTECTED		"protected"

#define CONSTANT		"constant"
#define CONSTANT_TYPE	"type"
#define CONSTANT_VALUE	"value"

#define	ID				"id"
#define ID_NAME			"name"
#define ID_DEF_PKG		"def.pkg"

#define VAR				"var"
#define VAR_NAME		"name"

#define IF				"if"
#define SET				"set"
#define FOR				"for"
#define BIND			"bind"
#define SEQ				"seq"
#define	BLOCK			"block"

#define ASSERT			"assert"
#define ASSERT_N		"n"
#define ASSERT_TYPE		"type"

#define APP				"app"

#define SYSFN			"sysfn"
#define SYSFN_VALUE		"value"

#define SYSAPP			"sysapp"
#define SYSAPP_NAME		"name"

#define	FN				"fn"

#define PROBLEM			"problem"
#define CULPRIT			"culprit"
#define CULPRIT_NAME	"name"
#define CULPRIT_VALUE	"value"

#include <iostream>
#include <fstream>
#include <sstream>

#include <getopt.h>
#include <syslog.h>
#include <stdlib.h>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <algorithm>


#include "mishap.hpp"
#include "sax.hpp"
#include "mnx.hpp"
#include "command.hpp"
#include "metainfo.hpp"

using namespace std;

#define FETCHGNX ( INSTALL_TOOL "/fetchgnx" )

class Main {
private:
	vector< string > 	project_folders;
	
public:
	bool absolute_processing;
	bool arity_processing;
	bool flatten_processing;
	bool lifetime_processing;
	bool lift_processing;
	bool scope_processing;
	bool sysapp_processing;
	bool tailcall_processing;
	bool val_processing;

public:
	std::string package;
	bool undefined_allowed;

private:
	void simplify( shared< Ginger::Mnx > g );
	bool resimplify( shared< Ginger::Mnx > g );


public:
	void parseArgs( int argc, char **argv, char **envp );
	void run();
	void printGPL( const char * start, const char * end );
	std::string version();
	
public:
	Main() :
		absolute_processing( false ),
		arity_processing( false ),
		flatten_processing( false ),
		lifetime_processing( false ),
		lift_processing( false ),
		scope_processing( false ),
		sysapp_processing( false ),
		tailcall_processing( false ),
		val_processing( false ),
		undefined_allowed( false )
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
		{ "flatten",		no_argument,			0, '3' },
        { "lifetime", 		no_argument,			0, '4' },
        { "lift",			no_argument,			0, '5' },
        { "scope",			no_argument,			0, '6' },
        { "sysapp",			no_argument,			0, '7' },
        { "tailcall",		no_argument,			0, '8' },
        { "val",         	no_argument,			0, '9' },
		{ "project",		required_argument,		0, 'j' },
        { "help",			optional_argument,		0, 'H' },
        { "license",        optional_argument,      0, 'L' },
        { "package",		required_argument,		0, 'p' },
        { "standard",		no_argument,			0, 's' },
        { "undefined",      no_argument,            0, 'u' },
        { "version",        no_argument,            0, 'V' },
        { 0, 0, 0, 0 }
    };

void Main::parseArgs( int argc, char **argv, char **envp ) {
    for(;;) {
        int option_index = 0;
        int c = getopt_long( argc, argv, "123456789f:H::L::p:suV", long_options, &option_index );
        if ( c == -1 ) break;
        switch ( c ) {
        	case '1': {
        		this->absolute_processing = true;
        		this->scope_processing = true;		//	requires scope analysis.
        		break;
        	}
        	case '2': {
        		this->arity_processing = true;
        		break;
        	}
        	case '3': {
        		this->flatten_processing = true;
        		break;
        	}
        	case '4': {
        		this->lifetime_processing = true;
        		break;
        	}
        	case '5': {
        		this->lift_processing = true;
        		this->scope_processing = true;		//	lift needs scope analysis.
        		this->flatten_processing = true;	//	lift benefits from flattening.
        		break;
        	}
        	case '6': {
        		this->sysapp_processing = true;
        		this->flatten_processing = true;	//	introduces new flattening opportunities.
        		break;
        	}
        	case '7': {
        		this->scope_processing = true;
        		break;
        	}
        	case '8': {
        		this->tailcall_processing = true;
        		break;
        	}
        	case '9': {
        		this->val_processing = true;
        		break;
        	}
        	case 's' : {
        		//	A standard choice of options. Everything (unless it is
        		//	insanely expensive - only we don't have any of those yet)
        		this->absolute_processing = true;
        		this->arity_processing = true;
        		this->flatten_processing = true;
        		this->lifetime_processing = true;
        		this->lift_processing = true;
        		this->scope_processing = true;
        		this->sysapp_processing = true;
        		this->tailcall_processing = true;        		
        		this->val_processing = true;        		
        		break;
        	}
        	case 'j' : {
        		this->project_folders.push_back( optarg );
        		break;
        	}
            case 'H': {
                //  Eventually we will have a "home" for our auxillary
                //  files and this will simply go there. Or run a web
                //  browser pointed there.
                if ( optarg == NULL ) {
                    printf( "Usage:  simplifygnx -p PACKAGE OPTIONS < GNX_IN > GNX_OUT\n" );
                    printf( "OPTIONS\n" );
                    printf( "-j, --project=PATH    adds a project folder onto the search path" );
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
            case 'u': {
            	this->undefined_allowed = true;
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
	string def_pkg_name;
        
public:
	void startTag( std::string & name, std::map< std::string, std::string > & attrs ) {
		if ( name == "resolve" ) {
			//qualified = attrs.find( "alias" ) != attrs.end();
			//this->enc_pkg_name = attrs[ "enc.pkg" ];
			//this->alias_name = attrs[ "alias" ];
			this->def_pkg_name = attrs[ "def.pkg" ];
		} else if ( name == PROBLEM ) {
			this->mishap.setMessage( attrs[ "message" ] );
		} else if ( name == CULPRIT ) {
			mishap.culprit( attrs[ CULPRIT_NAME ], attrs[ CULPRIT_VALUE ] );
		} else {
			throw Ginger::Mishap( "Unexpected element in response" ).culprit( "Element name", name );
		}
	}
	
	void endTag( std::string & name ) {
		if ( name == PROBLEM ) {
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

std::string resolveUnqualified( vector< string > & project_folders, const std::string & enc_pkg, const std::string & name, const bool undefined_allowed ) {
	Ginger::Command cmd( FETCHGNX );
	cmd.addArg( "-R" );
	if ( undefined_allowed ) {
		cmd.addArg( "-u" );
	}
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

	//cerr << "About to run the command. Package: " << enc_pkg << endl;
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
	const std::string & name ,
	const bool undefined_allowed
) {
	Ginger::Command cmd( FETCHGNX );
	cmd.addArg( "-R" );
	if ( undefined_allowed ) {
		cmd.addArg( "-u" );
	}
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
class TransformOfSomeKind : public Ginger::MnxVisitor {
public:
	void startVisit( Ginger::Mnx & element ) {
	}
	
	void endVisit( Ginger::Mnx & element ) {
	}
	
public:
	virtual ~TransformOfSomeKind() {}
};
*/


class ArityMarker : public Ginger::MnxVisitor {
private:
	bool changed;
	
public:
	bool hasChanged() { return this->changed; }
	
public:
	void startVisit( Ginger::Mnx & element ) {
		element.clearAttribute( ARITY );	//	Throw away any previous marking.
	}
	
	void endVisit( Ginger::Mnx & element ) {
		const string & x = element.name();
		if ( x == CONSTANT ) {
			element.putAttribute( ARITY, "1" );
		} else if ( x == ID || x == FN ) {
			element.putAttribute( ARITY, "1" );
		} else if ( x == FOR ) {
			if ( element.hasAttribute( ARITY ) && element.attribute( ARITY ) == "0" ) {
				element.putAttribute( ARITY, "0" );
			}
		} else if ( x == SET || x == BIND ) {
			element.putAttribute( ARITY, "0" );
		} else if ( x == SEQ ) {
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
		} else if ( x == IF ) {
			bool has_odd_kids = ( element.size() % 2 ) == 1;
			bool all_have_arity = true;
			for ( int i = 1; all_have_arity && i < element.size(); i += 2 ) {
				all_have_arity = element.child( i )->hasAttribute( ARITY );
			}
			if ( all_have_arity && has_odd_kids ) {
				all_have_arity = element.lastChild()->hasAttribute( ARITY );
			}
			if ( all_have_arity ) {
				const int N = element.size();
				if ( N == 0 ) {
					element.putAttribute( ARITY, "0" );
				} else if ( N == 1 ) {
					element.putAttribute( ARITY, element.child( 0 )->attribute( ARITY ) );
				} else {
					Ginger::Arity sofar( element.child( 1 )->attribute( ARITY ) );
					for ( int i = 3; i < element.size(); i += 2 ) {
						Ginger::Arity kid( element.child( i )->attribute( ARITY ) );
						sofar = sofar.unify( kid );
					}
					if ( has_odd_kids ) {
						sofar = sofar.unify( Ginger::Arity( element.lastChild()->attribute( ARITY ) ) );
					}
					element.putAttribute( ARITY, sofar.toString() );
				}
				
			}
		} else if ( x == SYSAPP ) {
			element.putAttribute( ARITY, Ginger::outArity( element.attribute( SYSAPP_NAME ) ).toString() );
		} else if ( x == ASSERT && element.size() == 1 ) {
			if ( element.hasAttribute( ASSERT_N, "1" ) ) {
				if ( element.child( 0 )->hasAttribute( ARITY, "1" ) ) {
					//	As we can prove the child satisfies the condition we should 
					//	eliminate the parent. So we simply replace the contents of the 
					//	node with the contents of the child!
					shared< Ginger::Mnx > c = element.child( 0 );
					element.copyFrom( *c );
					//	Because we have changed something we should let the system know
					//	in case it would synergise with other optimsations.
					this->changed = true;
				} else {
					element.putAttribute( ARITY, "1" );
				}
			} else if ( element.hasAttribute( ASSERT_TYPE ) ) {
				shared< Ginger::Mnx > c = element.child( 0 );
				if ( c->hasName( CONSTANT ) && element.attribute( ASSERT_TYPE ) == c->attribute( CONSTANT_TYPE ) ) {
					element.copyFrom( *c );
					this->changed = true;
				} else {
					element.putAttribute( ARITY, "1" );
				}
			}
		} 
	}
	
public:
	ArityMarker() : changed( false ) {}
	virtual ~ArityMarker() {}
};



#define TAIL_CALL_MASK 		0x1

class TailCall : public Ginger::MnxVisitor {

public:
	void startVisit( Ginger::Mnx & element ) {
		const string & x = element.name();
		element.clearAttribute( TAILCALL );	//	Throw away any previous marking.
		if ( x == FN ) {
			shared< Ginger::Mnx > ch( element.lastChild() );
			ch->orFlags( TAIL_CALL_MASK );
		} else if ( element.hasAllFlags( TAIL_CALL_MASK ) ) {
			if ( x == APP ) {
				element.putAttribute( TAILCALL, "true" );
			} else if ( x == IF ) {
				bool has_odd_kids = ( x.size() % 2 ) == 1;
				for ( int i = 1; i < element.size(); i += 2 ) {
					element.child( i )->orFlags( TAIL_CALL_MASK );
				}
				if ( has_odd_kids ) {
					element.lastChild()->orFlags( TAIL_CALL_MASK );
				}
			} else if ( x == SEQ || x == BLOCK ) {
				if ( element.size() >= 1 ) {
					shared< Ginger::Mnx > ch = element.lastChild();
					ch->orFlags( TAIL_CALL_MASK );
				}
			}
			//	else don't push the marker down. 
			//	This includes <for>, <sysapp>, any constant,
			//	<id>, <set>
		}
	}
	
	void endVisit( Ginger::Mnx & element ) {
		element.clearFlags( TAIL_CALL_MASK );
	}
	
	
public:
	TailCall() {}

	virtual ~TailCall() {}
};





/*
	This pass does the following: 
	
	[1] It finds all references global variables and find their
		originating package. It caches this using the def.pkg attribute.

*/
class Absolute : public Ginger::MnxVisitor {
private:
	vector< string > & project_folders;
	std::string package;
	const bool undefined_allowed;
		
public:
	void startVisit( Ginger::Mnx & element ) {
		const string & x = element.name();
		if ( x == ID ) {
			const string & name = element.attribute( ID_NAME );
			if ( element.hasAttribute( SCOPE, "global" ) ) {
				if ( not element.hasAttribute( "def.pkg" ) ) {
					const string & enc_pkg = element.hasAttribute( "enc.pkg" ) ? element.attribute( "enc.pkg" ) : this->package;
					if ( element.hasAttribute( "alias" ) ) {
						const string & alias = element.attribute( "alias" );
						//cout << "RESOLVE (QUALIFIED): name=" << name << ", alias=" << alias << ", enc.pkg=" << enc_pkg << endl;
						const string def_pkg( resolveQualified( this->project_folders, enc_pkg, alias, name, this->undefined_allowed ) );
						//cout << "   def = " << def_pkg << endl;
						element.putAttribute( "def.pkg", def_pkg );
					} else {
						//cerr << "RESOLVE (UNQUALIFIED): name=" << name << ", enc.pkg=" << enc_pkg << endl;
						const string def_pkg( resolveUnqualified( this->project_folders, enc_pkg, name, this->undefined_allowed ) );
						//cerr << "   def = " << def_pkg << endl;
						element.putAttribute( "def.pkg", def_pkg );						
					}
				}
			}
		} else if ( x == "var" ) {
			if ( element.hasAttribute( SCOPE, "global" ) ) {
				if ( not element.hasAttribute( "def.pkg" ) ) {
					const string & enc_pkg = element.hasAttribute( "enc.pkg" ) ? element.attribute( "enc.pkg" ) : this->package;
					element.putAttribute( "def.pkg", enc_pkg );						
				}
			}
		}
	}
	
	void endVisit( Ginger::Mnx & element ) {
	}
	
public:
	Absolute( vector< string > & folders, const std::string & enc_pkg, const bool undefined_allowed ) : 
		project_folders( folders ),
		package( enc_pkg ),
		undefined_allowed( undefined_allowed )
	{
	}

	virtual ~Absolute() {}
};

struct VarInfo {
	Ginger::Mnx *			element;
	const string *			name;
	int						uid;
	size_t					dec_level;
	bool 					is_protected;
	bool					is_local;
	long					max_diff_use_level;
	
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
	
	VarInfo( Ginger::Mnx * element, const string * name, int uid, size_t dec_level, bool is_protected, bool is_local ) : 
		element( element ),
		name( name ),
		uid( uid ),
		dec_level( dec_level ),
		is_protected( is_protected ),
		is_local( is_local ),
		max_diff_use_level( 0 )
	{
	}
};

/*
	The Scope pass does the following tasks. 
	
	[1] It marks variables with a unique ID (UID).
	
	[2] It classifies variables into global and local and caches this
		using the SCOPE attribute. 
		
	[3] It propagates the protected="true" maplet into all <id> elements.
	
*/
class Scope : public Ginger::MnxVisitor {
private:
	int var_uid;
	bool outers_found;

	vector< int > scopes;
	vector< VarInfo > vars;
	
private:
	bool isGlobalScope() {
		return this->scopes.empty();
	}
	
	int newUid() {
		return this->var_uid++;
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
				long d = this->scopes.size() - it->dec_level;
				if ( it->is_local && d != 0 ) {
					//cout << "We have detected an outer declaration: " << name << endl;
					//cout << "Declared at level " << it->dec_level << " but used at level " << this->scopes.size() << endl;
					it->max_diff_use_level = max( d, it->max_diff_use_level );
					this->outers_found = true;
					it->element->putAttribute( IS_OUTER, "true" );
				}
				return &*it;
			}
		}
		return NULL;
	}
	
public:
	bool wereOutersFound() {
		return this->outers_found;
	}
	
	void startVisit( Ginger::Mnx & element ) {
		const string & x = element.name();
		element.clearAttribute( UID );		//	Throw away any previous marking.
		element.clearAttribute( SCOPE );	//	Throw away any previous marking.
		element.clearAttribute( OUTER_LEVEL );
		element.clearAttribute( IS_OUTER );
		if ( x == ID ) {
			const string & name = element.attribute( ID_NAME );
			VarInfo * v = this->findId( name );
			if ( v != NULL ) {
				element.putAttribute( UID, v->uid );
			}
			if ( v == NULL || v->isGlobal() ) {
				element.putAttribute( SCOPE, "global" );
			} else {
				element.putAttribute( SCOPE, "local" );
				if ( v->max_diff_use_level > 0 ) {
					element.putAttribute( OUTER_LEVEL, v->max_diff_use_level );
				}
			}
			if ( v != NULL && v->is_protected ) {
				element.putAttribute( PROTECTED, "true" );
			}
		} else if ( x == VAR ) {
			const string & name = element.attribute( VAR_NAME );
			const int uid = this->newUid();
			element.putAttribute( UID, uid );
			if ( not element.hasAttribute( PROTECTED ) ) {
				element.putAttribute( PROTECTED, "true" );
			}
			const bool is_protected = element.attribute( PROTECTED ) == "true";
			const bool is_global = this->isGlobalScope();
			element.putAttribute( SCOPE, is_global ? "global" : "local" );
			this->vars.push_back( VarInfo( &element, &name, uid, this->vars.size(), is_protected, not is_global ) );
		} else if ( x == "fn" ) { //|| x == "block" || x == "for" ) {
			this->scopes.push_back( this->vars.size() );
		}
	}
	
	void endVisit( Ginger::Mnx & element ) {
		const string & x = element.name();
		if ( x == FN ) { //|| x == "block" || x == "for" ) {
			int n = this->scopes.back();
			this->scopes.pop_back();
			this->vars.resize( n );
		} else if ( 
			x == SET && 
			element.size() == 2 &&
			element.firstChild()->name() == ID &&
			element.firstChild()->hasAttribute( PROTECTED, "true" ) &&
			element.firstChild()->hasAttribute( ID_NAME )
		) {
			throw Ginger::Mishap( "Assigning to a protected variable" ).culprit( "Variable", element.firstChild()->attribute( ID_NAME ) );
		}
	}
public:
	Scope() : 
		var_uid( 0 ),
		outers_found( false )
	{
	}

	virtual ~Scope() {}
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
		
*/
class SysFold : public Ginger::MnxVisitor {
private:
	bool changed;
	
public:
	bool hasChanged() { return this->changed; }
	
public:
	void startVisit( Ginger::Mnx & element ) {
	}

	void endVisit( Ginger::Mnx & element ) {
		const string & x = element.name();
		if ( x == ID ) {
			if ( element.hasAttribute( ID_DEF_PKG, "ginger.library" ) ) {
				//cout<<"IN 1" << endl;
				const string name( element.attribute( ID_NAME ) );
				element.clearAllAttributes();
				element.putAttribute( CONSTANT_VALUE, name );
				element.putAttribute( CONSTANT_TYPE, "sysfn" );
				element.name() = CONSTANT;
				this->changed = true;
				//cout<<"OUT 1" << endl;
			}
		} else if ( x == APP && element.size() == 2 ) {
			if ( element.child( 0 )->hasName( CONSTANT ) && element.child( 0 )->hasAttribute( CONSTANT_TYPE, "sysfn" ) && element.child( 0 )->hasAttribute( CONSTANT_VALUE ) ) {
				//cout<<"IN 2" << endl;
				const string value( element.child( 0 )->attribute( CONSTANT_VALUE ) );
				element.name() = SYSAPP;
				element.clearAllAttributes();
				element.putAttribute( SYSAPP_NAME, value );
				element.popFrontChild();
				this->changed = true;
				//cout<<"OUT 2" << endl;
				//element.render();
				//cout << endl;
			}
		}
	}

public:
	virtual ~SysFold() {}
};

//	Template for a Tree-walking Transformation
class Lift : public Ginger::MnxVisitor {
private:
	map< string, Ginger::Mnx * > 	dec_outer;
	vector< set< string > > 		capture_sets;

public:
	void startVisit( Ginger::Mnx & element ) {
		const string & x = element.name();
		if ( x == VAR && element.hasAttribute( IS_OUTER ) && element.hasAttribute( UID ) ) {
			this->dec_outer[ element.attribute( UID ) ] = &element;
		} else if ( x == ID && element.hasAttribute( UID ) ) {
			map< string, Ginger::Mnx * >::iterator m = this->dec_outer.find( element.attribute( UID ) );
			if ( m != this->dec_outer.end() ) {
				element.putAttribute( IS_OUTER, "true" );
				if ( element.hasAttribute( OUTER_LEVEL ) ) {
					size_t level;
					stringstream s( element.attribute( OUTER_LEVEL ) );
					s >> level;
					const size_t n = this->capture_sets.size();
					for ( size_t i = 1; i <= level; i++ ) {
						//cout << "INSERT " << i << "/" << n << " uid=" << element.attribute( UID ) << endl;
						this->capture_sets[ n - i ].insert( element.attribute( UID ) );
						//cout << "  #capture set[" << ( n - i ) << "] = " << this->capture_sets[ n - 1 ].size() << endl;
					}
					
					#ifdef DBG_SIMPLYGNX
						int k = 0;
						for (
							vector< set< string > >::iterator jt = this->capture_sets.begin();
							jt != this->capture_sets.end();
							++jt, k++
						) {
							cout << "  Capture set [" << k << "] size = " << jt->size() << endl; 
						}
					#endif
				}
			}
		} else if ( x == FN ) {
			this->capture_sets.push_back( set< string >() );
		}
	}
	
	//	method
	shared< Ginger::Mnx > makeLocals( const char * tag ) {
		shared< Ginger::Mnx > locals( new Ginger::Mnx( SEQ ) );
		for ( 
			set< string >::iterator it = this->capture_sets.back().begin();
			it != this->capture_sets.back().end();
			++it
		) {
			const string & uid = *it;
			Ginger::Mnx * v = this->dec_outer[ uid ];
			if ( v == NULL ) throw;
			//cout << "SO FAR: ";
			/*v->render();
			cout << endl;*/
			shared< Ginger::Mnx > var( new Ginger::Mnx( tag ) );
			var->putAttribute( "name", v->attribute( "name" ) );
			var->putAttribute( UID, uid );
			var->putAttribute( SCOPE, "local" );
			locals->addChild( var );
		}
		return locals;
	}
	
	void endVisit( Ginger::Mnx & element ) {
		const string & x = element.name();
		if ( x == FN ) {
			/*cout << "Capture set:" << element.attribute( "name", "[anon]" ) << ":";
			for ( 
				set< string >::iterator it = this->capture_sets.back().begin();
				it != this->capture_sets.back().end();
				++it
			) {
				cout << " " << *it << endl;
			}
			cout << endl;*/
			
			if ( not this->capture_sets.back().empty() ) {
				
				//	We should extend the set of local variables with the
				//	capture set.
				{
					shared< Ginger::Mnx > arg( element.child( 0 ) );
					shared< Ginger::Mnx > newarg( new Ginger::Mnx( SEQ ) );
					newarg->addChild( arg );
					newarg->addChild( this->makeLocals( VAR ) );
					
					/*newarg->render();
					cout << endl;*/
					
					element.child( 0 ) = newarg;
				}
				
				//	Then we should transform the function into a call
				//	to partapply onto the original function and 
				//	the capture set.
				{
					shared< Ginger::Mnx > fn( new Ginger::Mnx( FN ) );
					fn->copyFrom( element );
					shared< Ginger::Mnx > partapply( new Ginger::Mnx( SYSAPP ) );
					partapply->putAttribute( SYSAPP_NAME, "partApply" );
					partapply->addChild( this->makeLocals( "id" ) );
					partapply->addChild( fn );
					element.copyFrom( *partapply );
				}

			}			
			this->capture_sets.pop_back();
		}
	}
	
public:
	virtual ~Lift() {}
};

class AddDeref : public Ginger::MnxVisitor {
public:
	
	void startVisit( Ginger::Mnx & element ) {
	}
	
	//	This has to be an endVisit. If it is a startVisit, it might
	//	generate an infinite loop, generating and infinitely deep
	//	chain of derefs. It could be a start visit IF we added/removed
	//	a flag attribute.
	//
	void endVisit( Ginger::Mnx & element ) {
		const string & x = element.name();
		if ( element.hasAttribute( IS_OUTER, "true" ) && element.hasAttribute( PROTECTED, "false" ) && ( x == "var" || x == "id" ) ) {
			shared< Ginger::Mnx > t( new Ginger::Mnx( x ) );
			t->copyFrom( element );
			shared< Ginger::Mnx > d( new Ginger::Mnx( "deref" ) );
			d->addChild( t );
			element.copyFrom( *d );
		/*} else if ( x == "bind" && element.child(0).name() == "deref" ) {
			//	<bind><deref><var></deref>EXPR</bind> 
			//	<bind><var><makeref>EXPR</makeref></bind>
			GOT HERE*/
		}
	}
	
public:
	virtual ~AddDeref() {}
};


class Flatten : public Ginger::MnxVisitor {
public:
	void startVisit( Ginger::Mnx & element ) {
		const string & x = element.name();
		if ( x == SYSAPP || x == SEQ ) {
			for ( int i = 0; i < element.size(); i++ ) {
				if ( element.child( i )->name() == SEQ ) {
					element.flattenChild( i );
					i -= 1;
				}
			}
		}
	}

	void endVisit( Ginger::Mnx & element ) {
	}

public:
	Flatten() {}
	virtual ~Flatten() {}
};

bool Main::resimplify( shared< Ginger::Mnx > g ) {
	bool resimplify = false;
	
	if ( sysapp_processing ) {	
		SysFold s;
		g->visit( s );
		//	We are not interested in the changed flag at this point
		//	because folding is part of the monotonic change.
	}
	
	if ( flatten_processing ) {
		Flatten flatten;
		g->visit( flatten );
	}
	
	if ( arity_processing ) {
		ArityMarker a;
		g->visit( a );
		//	We *do* care about changing here as it breaks the
		//	otherwise monotonic process.
		resimplify = resimplify || a.hasChanged();
	}
	
	return resimplify;
}

void Main::simplify( shared< Ginger::Mnx > g ) {
	bool resimplify = false;

	//	If we are not doing absolute_processing we should assume we
	//	will find outers.
	bool outers_found = not absolute_processing;
	
	if ( scope_processing ) {
		Scope scope;
		g->visit( scope );
		outers_found = scope.wereOutersFound();
	}
	
	if ( absolute_processing ) {
		Absolute a( this->project_folders, this->package, this->undefined_allowed );
		g->visit( a );
		//outers_found = a.wereOutersFound();
	} 

	if ( sysapp_processing ) {	
		SysFold s;
		g->visit( s );
		//	N.B. We do not care about the changed flag as it cannot
		//	cause rework at this point.
	}
	
	if ( lift_processing ) {
		//	Only proceed if we have determined that there are indeed
		//	some variables which are declared as outers. Otherwise
		//	this is just an expensive no-op!
		if ( outers_found ) {
			Lift lift;
			g->visit( lift );
			AddDeref addd;
			g->visit( addd );
		}
	}
	
	if ( flatten_processing ) {
		Flatten flatten;
		g->visit( flatten );
	}
	
	if ( arity_processing ) {
		ArityMarker a;
		g->visit( a );
		resimplify = resimplify || a.hasChanged();
	}
	
	while ( resimplify ) {
		resimplify = this->resimplify( g );
	}

	if ( tailcall_processing ) {
		g->orFlags( TAIL_CALL_MASK );
		TailCall tc;
		g->visit( tc );
	}
	
}

void Main::run() {
	Ginger::MnxReader reader;
	
	for (;;) {
	
		shared< Ginger::Mnx > g( reader.readMnx() );
		if ( not g ) break;
	
		this->simplify( g );
		
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
		p.culprit( "Detected by", SIMPLIFYGNX );
		p.gnxReport();
		return EXIT_FAILURE;
	} catch ( Ginger::Problem & p ) {
		p.culprit( "Detected by", SIMPLIFYGNX );
		p.gnxReport();
		return EXIT_FAILURE;
	}
}
