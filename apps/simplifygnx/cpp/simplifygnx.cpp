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

//#define DBG_SIMPLIFYGNX
//#define DBG_PACKAGE_AUTOLOAD

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

--sysapp: Replace references to system calls with <constant type="sysfn">
	and <sysapp>. It relies on absolute references and will typically
	be combined with --absolute.
	
--tailcall: Marks tail-calls.

--match: Implements the destructuring of pattern matching.
 */ 
 
#define SIMPLIFYGNX 	"simplifygnx"

//	These are attributes that are used to annotate the tree. Should
//	be reserved to SimplifyGNX.

#define TAIL_CALL_MASK 		0x1
#define SELF_BIND_MASK		0x2

#include <iostream>
#include <fstream>
#include <sstream>

#include <list>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <algorithm>

#include <stddef.h>
#include <getopt.h>
#include <syslog.h>
#include <stdlib.h>
#include <unistd.h>

#include <uuid/uuid.h>

#include "gnxconstants.hpp"
#include "mishap.hpp"
#include "sax.hpp"
#include "mnx.hpp"
#include "command.hpp"
#include "metainfo.hpp"
#include "printgpl.hpp"

using namespace std;

#define FETCHGNX ( INSTALL_TOOL "/fetchgnx" )
typedef shared< Ginger::Mnx > Gnx;

class ProcessingOptions {
private:
	bool absolute_processing;
	bool arity_processing;
	bool flatten_processing;
	bool lifetime_processing;
	bool lift_processing;
	bool scope_processing;
	bool self_processing;
	bool sysapp_processing;
	bool tailcall_processing;
	bool val_processing;
	bool slot_processing;
	bool top_level_processing;
    bool match_processing;
	
public:
	static void helpOptions() {
		cout << "PROCESSING STAGE OPTIONS (n.b. -4 and -9 free for future use)" << endl;
        cout << "-s, --standard        a standard set of transformations, equal to options 0-9" << endl;
        cout << "-u, --undefined       undefined allowed" << endl;
		cout << "-0, --self            replace named lambda self-references with special forms" << endl;
		cout << "-1, --absolute        add def.pkg attribute to all globals" << endl;
		cout << "-2, --arity           add arity marking to all subexpressions" << endl;
		cout << "-3, --flatten         eliminate nested sequences" << endl;
        cout << "-4, --lifetime        lifetime analysis" << endl;
		cout << "-5, --lift            transform nested lambdas so that they only reference immediate locals" << endl;
		cout << "-6, --scope           mark all variable definitions and references as local or global" << endl;
		cout << "-7, --sysapp          replace calls to standard variable with sys-calls" << endl;
		cout << "-8, --tailcall        marks function applications as tail-call or not" << endl;
        cout << "-9, --val             val analysis" << endl;
		cout << "-A, --slotalloc       assigns a frame slot to every variable" << endl;
		cout << "-B, --toplevel        moves every lambda to top-level" << endl;
        cout << "-C, --match           pattern match simplification" << endl;

	}	
		
public:
	bool getAbsoluteProcessing() const { return this->absolute_processing; }
	bool getArityProcessing() const { return this->arity_processing; }
	bool getFlattenProcessing() const { return this->flatten_processing; }
	bool getLifetimeProcessing() const { return this->lifetime_processing; }
	bool getLiftProcessing() const { return this->lift_processing; }
	bool getScopeProcessing() const { return this->scope_processing; }
	bool getSelfProcessing() const { return this->self_processing; }
	bool getSysappProcessing() const { return this->sysapp_processing; }
	bool getTailcallProcessing() const { return this->tailcall_processing; }
	bool getValProcessing() const { return this->val_processing; }
	bool getSlotProcessing() const { return this->slot_processing; }
	bool getTopLevelProcessing() const { return this->top_level_processing; }
    bool getMatchProcessing() const { return this->match_processing; }
	
public:
	void setAbsoluteProcessing() { 
		this->absolute_processing = true;         		
		this->scope_processing = true;		//	requires scope analysis.
	}
	void setArityProcessing() { 
		this->arity_processing = true; 
	}
	void setFlattenProcessing() { 
		this->flatten_processing = true; 
	}
	void setLifetimeProcessing() { 
		this->lifetime_processing = true; 
	}
	void setLiftProcessing() { 
		this->lift_processing = true; 
		this->scope_processing = true;		//	lift needs scope analysis.
		this->flatten_processing = true;	//	lift benefits from flattening.
	}
	void setScopeProcessing() { 
		this->scope_processing = true; 
	}
	void setSelfProcessing() { 
		this->self_processing = true; 
	}
	void setSysappProcessing() { 
		this->sysapp_processing = true; 
		this->flatten_processing = true;	//	introduces new flattening opportunities.
	}
	void setTailcallProcessing() { 
		this->tailcall_processing = true; 
	}
	void setValProcessing() { 
		this->val_processing = true; 
	}
	void setSlotProcessing() { 
		this->slot_processing = true; 
		this->setLiftProcessing();			//	Won't work unless lifted.
	}
	void setTopLevelProcessing() { 
		this->top_level_processing = true;
		this->setLiftProcessing();			//	Won't work unless lifted.
	}
    void setMatchProcessing() {
        this->match_processing = true;       
    }
	
public:
	void setStandard() {
		this->absolute_processing = true;
		this->arity_processing = true;
		this->flatten_processing = true;
		this->lifetime_processing = true;
		this->lift_processing = true;
		this->scope_processing = true;
		this->self_processing = true;
		this->sysapp_processing = true;
		this->tailcall_processing = true;        		
		this->val_processing = true;  
        this->match_processing = true;      		
	}

public:
	ProcessingOptions() :
		absolute_processing( false ),
		arity_processing( false ),
		flatten_processing( false ),
		lifetime_processing( false ),
		lift_processing( false ),
		scope_processing( false ),
		self_processing( false ),
		sysapp_processing( false ),
		tailcall_processing( false ),
		val_processing( false ),
		slot_processing( false ),
		top_level_processing( false ),
        match_processing( false )
	{}
	
};

class Main : public ProcessingOptions {
private:
	vector< string > 	project_folders;
	
public:
	std::string package;
	bool undefined_allowed;
	bool retain_debug_info;

private:
	void simplify( Gnx & g );
	bool resimplify( Gnx & g );


public:
	void parseArgs( int argc, char **argv, char **envp );
	void run();
	std::string version() { return "0.1"; }
	
public:
	Main() :
		undefined_allowed( false ),
		retain_debug_info( false )
	{}
};


extern char * optarg;
static struct option long_options[] =
    {
    	{ "self",			no_argument,			0, '0' },
		{ "absolute",		no_argument,			0, '1' },
		{ "arity",			no_argument,			0, '2' },
		{ "flatten",		no_argument,			0, '3' },
        { "lifetime", 		no_argument,			0, '4' },
        { "lift",			no_argument,			0, '5' },
        { "scope",			no_argument,			0, '6' },
        { "sysapp",			no_argument,			0, '7' },
        { "tailcall",		no_argument,			0, '8' },
        { "val",         	no_argument,			0, '9' },
        { "slotalloc",		no_argument,			0, 'A' },
        { "toplevel",		no_argument,			0, 'B' },
        { "debug",			no_argument,			0, 'd' },
        { "help",			optional_argument,		0, 'H' },
		{ "project",		required_argument,		0, 'j' },
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
        int c = getopt_long( argc, argv, "0123456789ABdj:H::L::p:suV", long_options, &option_index );
        if ( c == -1 ) break;
        switch ( c ) {
        	case '0': {
        		this->setSelfProcessing();
        		break;
        	}
        	case '1': {
        		this->setAbsoluteProcessing();
        		//this->scope_processing = true;		//	requires scope analysis.
        		break;
        	}
        	case '2': {
        		//this->arity_processing = true;
        		this->setArityProcessing();
        		break;
        	}
        	case '3': {
        		//this->flatten_processing = true;
        		this->setFlattenProcessing();
        		break;
        	}
        	case '4': {
        		//this->lifetime_processing = true;
        		this->setLifetimeProcessing();
        		break;
        	}
        	case '5': {
        		//this->lift_processing = true;
        		//this->scope_processing = true;		//	lift needs scope analysis.
        		//this->flatten_processing = true;	//	lift benefits from flattening.
        		this->setLiftProcessing();
        		break;
        	}
        	case '6': {
        		//this->sysapp_processing = true;
        		//this->flatten_processing = true;	//	introduces new flattening opportunities.
        		this->setSysappProcessing();
        		break;
        	}
        	case '7': {
        		//this->scope_processing = true;
        		this->setScopeProcessing();
        		break;
        	}
        	case '8': {
        		this->setTailcallProcessing();
        		break;
        	}
        	case '9': {
        		//this->val_processing = true;
        		this->setValProcessing();
        		break;
        	}
        	case 'A' : {
        		//this->slot_processing = true;
        		this->setSlotProcessing();
        		break;
        	}
            case 'B' : {
                //this->top_level_processing = true;
                this->setTopLevelProcessing();
                break;
            }
            case 'C' : {
                this->setMatchProcessing();
                break;
            }
        	case 's' : {
        		//	A standard choice of options. Everything, unless it is
        		//	insanely expensive or of marginal benefit to all back-ends.
        		//	Only slot_processing excluded at the moment.       		
        		this->setStandard();
        		break;
        	}
        	case 'd' : {
        		this->retain_debug_info = true;
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
                    cout << "Usage:  simplifygnx -p PACKAGE OPTIONS < GNX_IN > GNX_OUT" << endl;
                    cout << "GENERAL OPTIONS" << endl;
                    cout << "-d, --debug           retain debugging information" << endl;
                    cout << "-H, --help[=TOPIC]    help info on optional topic (see --help=help)" << endl;
                    cout << "-j, --project=PATH    adds a project folder onto the search path" << endl;
                    cout << "-L, --license[=PART]  print out license information and exit (see --help=license)" << endl;
                    cout << "-p, --package=PACKAGE defines the enclosing package" << endl;
                    cout << "-V, --version         print out version information and exit" << endl;
                    ProcessingOptions::helpOptions();
                    cout << "" << endl;
                } else if ( std::string( optarg ) == "help" ) {
                    cout << "--help=help           this short help" << endl;
                    cout << "--help=licence        help on displaying license information" << endl;
                } else if ( std::string( optarg ) == "license" ) {
                    cout << "Displays key sections of the GNU Public License." << endl;
                    cout << "--license=warranty    Shows warranty." << endl;
                    cout << "--license=conditions  Shows terms and conditions." << endl;
                } else {
                    cout << "Unknown help topic " << optarg << endl;
                }
                exit( EXIT_SUCCESS );   //  Is that right?
            }
            case 'L': {
                exit( Ginger::optionPrintGPL( optarg ) );   //  Is that right?              
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
                cout << "?? getopt returned character code 0x" << hex << static_cast< int >( c ) << dec << endl;
            }
        }
    }
}

static string makeGUID() {  
    uuid_t x;
    uuid_generate( x );
    char s[ 37 ];               //  Because Linux does not define uuid_string_t ... argh!
    uuid_unparse( x, s );
    return string( s ); 
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
			cmd.addArg( "-j" );
			cmd.addArg( *it );
		}
	}
	cmd.addArg( "-p" );
	cmd.addArg( enc_pkg );
	cmd.addArg( "-v" );
	cmd.addArg( name );

    #ifdef DBG_SIMPLIFYGNX
	   cerr << "simplifygnx - About to run the command." << endl;
       cerr << "  Package   : " << enc_pkg << endl;
       cerr << "  Command   : " << cmd.asPrintString() << endl;
    #endif

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
        cerr << "simplifygnx resolving unqualified, reply from fetchgnx -R" << endl;
		cerr << "  [[" << prog.str() << "]]" << endl;
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
        cerr << "simplifygnx resolving qualified, reply from fetchgnx -R" << endl;
		cerr << "  [[" << prog.str() << "]]" << endl;
	#endif
GOT TO HERE
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


/**
	This is an arity marking pass that follows the main pass, patching
	up any use of self.app calls. This is a worthwhile optimisation for
	recursive benchmarks!
*/
class SelfAppArityMarker : public Ginger::MnxVisitor {
public:
	vector< Ginger::Arity > body_arities;
	
private:
    // Forward declaration.
    Ginger::Arity calcArity( Gnx expr );
    Ginger::Arity calcArityIfThenOptElse( Gnx expr, int offset );

 public:
	void startVisit( Ginger::Mnx & element ) {
		const string & nm = element.name();
		if ( nm == FN ) {
			//cerr << "#1" << endl;
			this->body_arities.push_back( calcArity( element.getChild( 1 ) ) );
		} else if ( nm == SELF_APP && this->body_arities.size() > 0 ) {
			//cerr << "#2" << endl;
			Ginger::Arity arity( this->body_arities[ this->body_arities.size() - 1 ] );
			element.putAttribute( ARITY, arity.toString() );
		}
	}
	
	void endVisit( Ginger::Mnx & element ) {
		const string & nm = element.name();
		if ( nm == FN ) {
			//cerr << "#3" << endl;
			this->body_arities.pop_back();
		}
	}

public:
	SelfAppArityMarker() {
		//cerr << "SELF APP MARKING" << endl;
	}
	
	virtual ~SelfAppArityMarker() {}
};

Ginger::Arity SelfAppArityMarker::calcArityIfThenOptElse( Gnx expr, int offset ) {
    vector< Ginger::Arity > subexpr_arities;
    for ( int i = 1 + offset; i < expr->size(); i += 2 ) {
        subexpr_arities.push_back( calcArity( expr->getChild( i ) ) );
    }
    const bool has_else = ( ( expr->size() - offset ) & 0x1 ) == 1;
    if ( has_else ) {
        subexpr_arities.push_back( calcArity( expr->getLastChild() ) );
    }
    if ( subexpr_arities.size() == 0 ) {
        return Ginger::Arity( 0, true );
    } else {
        Ginger::Arity sofar( subexpr_arities[ 0 ] );
        for ( size_t i = 1; i < subexpr_arities.size(); i++ ) {
            sofar = sofar.join( subexpr_arities[ i ] );
        }
        return sofar;
    }       
}

/**
    This purpose of this method is to cope with direct calls to
    self.app, which helps us get much closer to the least fixed
    point. 
*/
Ginger::Arity SelfAppArityMarker::calcArity( Gnx expr ) {
    if ( expr->hasAttribute( ARITY ) ) {
        return Ginger::Arity( expr->attribute( ARITY ) );
    } else if ( expr->name() == IF ) {
        return calcArityIfThenOptElse( expr, 0 );
    } else if ( expr->name() == SWITCH ) {
        return calcArityIfThenOptElse( expr, 1 );
    } else {
        return Ginger::Arity( 0, true );
    }
}



/**
    This is the main arity marking pass. It may incidentally replace
    erases with seqs if it can prove the enclosed expressions have 0
    arity. In that case it becomes advantageous to perform a round of
    flattening.

    It distinguishes between arity, which is the number of values that
    an expression will return (normally), and the pattern.arity which 
    is the number of values that a pattern binds to. The rules for 
    arity and pattern.arity are unsurprisingly almost identical.
 */
class ArityMarker : public Ginger::MnxVisitor {
private:
	bool changed;
	bool self_app_pass_needed;
	bool clear_arities;
    const char * arity_attribute;
	
public:
	bool hasChanged() { return this->changed; }
	bool hasSelfAppPassNeeded() { return this->self_app_pass_needed; }
	
private:
	Ginger::Arity sumOverChildren( Ginger::Mnx & element ) {
		bool all_have_arity = true;
		for ( int i = 0; all_have_arity && i < element.size(); i++ ) {
			all_have_arity = element.getChild( i )->hasAttribute( this->arity_attribute );
		}
		if ( all_have_arity ) {
			Ginger::Arity sofar( 0 );
			for ( int i = 0; i < element.size(); i++ ) {
				Ginger::Arity kid( element.getChild( i )->attribute( this->arity_attribute ) );
				sofar = sofar.add( kid );
			}
			return sofar;
		} else {
			return Ginger::Arity( 0, true );
		}
	}

    bool isEvalMode() const {
        return this->arity_attribute == ARITY;
    }

    bool isPatternMode() const {
        return this->arity_attribute == PATTERN_ARITY;
    }


public:
	void startVisit( Ginger::Mnx & element ) {
		if ( this->clear_arities ) {
            element.clearAttribute( ARITY );            //  Throw away any previous marking.
            element.clearAttribute( PATTERN_ARITY );    //  Throw away any previous marking.
        }
        if ( element.hasAttribute( ANALYSIS_TYPE ) ) {
            //  Fetch the analysis-type we want to switch to.
            const std::string a_type = element.attribute( ANALYSIS_TYPE );
            //  Cache the current arity analysis-type. It is a bit
            //  naughty to reuse the ANALYSIS_TYPE attribute.
            element.putAttribute( ANALYSIS_TYPE, this->arity_attribute );
            //  Now make the switch.
            this->arity_attribute = a_type == ARITY ? ARITY : PATTERN_ARITY;
        } else {
            const string & nm = element.name();
            const int N = element.size();
            if ( 
                ( nm == BIND && N == 2 ) ||
                ( nm == FN && N == 2 ) ||
                ( nm == CATCH_THEN && N == 2 ) ||
                ( nm == CATCH_RETURN && N == 2 )
            ) {
                element.getChild( 0 )->putAttribute( ANALYSIS_TYPE, PATTERN_ARITY );
            }
        }
	}
	
	void endVisit( Ginger::Mnx & element ) {
		const string & x = element.name();
        const int N = element.size();
		if ( x == CONSTANT || x == SELF_CONSTANT ) {
			element.putAttribute( this->arity_attribute, "1" );
		} else if ( x == ID || x == FN ) {
			element.putAttribute( this->arity_attribute, "1" );
        } else if ( x == AND || x == OR || x == ABSAND || x == ABSOR ) {
            element.putAttribute( this->arity_attribute, "1" );
		} else if ( x == LIST || x == LIST_APPEND || x == VECTOR ) {
			element.putAttribute( this->arity_attribute, "1" );
		} else if ( x == FOR && this->isEvalMode() ) {
            //  TODO: Check this is correct - looks wrong!
			if ( element.hasAttribute( this->arity_attribute, "0" ) ) {
				element.putAttribute( this->arity_attribute, "0" );
			}
		} else if ( x == SET && this->isEvalMode() ) {
			element.putAttribute( this->arity_attribute, "0" );
        } else if ( x == BIND ) {
            element.putAttribute( this->arity_attribute, "0" );
		} else if ( x == SEQ ) {
			element.putAttribute( this->arity_attribute, this->sumOverChildren( element ).toString() );
		} else if ( x == IF && this->isEvalMode() ) {
			bool has_else = ( element.size() % 2 ) == 1;
			bool all_have_arity = true;
			for ( int i = 1; all_have_arity && i < element.size(); i += 2 ) {
				all_have_arity = element.getChild( i )->hasAttribute( this->arity_attribute );
			}
			if ( all_have_arity && has_else ) {
				all_have_arity = element.getLastChild()->hasAttribute( this->arity_attribute );
			}
			if ( all_have_arity ) {
				const int N = element.size();
				if ( N == 0 ) {
                    //  Not even an else clause, equal to a <seq/>.
					element.putAttribute( this->arity_attribute, "0" );
				} else if ( N == 1 ) {
                    //  Consists only of an else clause.
					element.putAttribute( this->arity_attribute, element.getChild( 1 )->attribute( this->arity_attribute ) );
				} else {
                    //  Has at least one if-then pair.
					Ginger::Arity sofar( element.getChild( 1 )->attribute( this->arity_attribute ) );
					for ( int i = 3; i < element.size(); i += 2 ) {
						Ginger::Arity kid( element.getChild( i )->attribute( this->arity_attribute ) );
						sofar = sofar.unify( kid );
					}
					if ( has_else ) {
						sofar = sofar.unify( Ginger::Arity( element.getLastChild()->attribute( this->arity_attribute ) ) );
					} else {
                        //  Otherwise the else clause is implicitly an <seq/>.
                        sofar = sofar.unify( Ginger::Arity( 0 ) );
                    }
					element.putAttribute( this->arity_attribute, sofar.toString() );
				}
			}
        } else if ( x == SWITCH && element.size() >= 1 && this->isEvalMode() ) {
            bool has_else = ( element.size() % 2 ) == 0;
            bool all_have_arity = true;
            for ( int i = 2; all_have_arity && i < element.size(); i += 2 ) {
                all_have_arity = element.getChild( i )->hasAttribute( this->arity_attribute );
            }
            if ( all_have_arity && has_else ) {
                all_have_arity = element.getLastChild()->hasAttribute( this->arity_attribute );
            }
            if ( all_have_arity ) {
                const int N = element.size();
                if ( N == 1 ) {
                    element.putAttribute( this->arity_attribute, "0" );
                } else if ( N == 2 ) {
                    element.putAttribute( this->arity_attribute, element.getChild( 1 )->attribute( this->arity_attribute ) );
                } else {
                    Ginger::Arity sofar( element.getChild( 2 )->attribute( this->arity_attribute ) );
                    for ( int i = 4; i < element.size(); i += 2 ) {
                        Ginger::Arity kid( element.getChild( i )->attribute( this->arity_attribute ) );
                        sofar = sofar.unify( kid );
                    }
                    if ( has_else ) {
                        sofar = sofar.unify( Ginger::Arity( element.getLastChild()->attribute( this->arity_attribute ) ) );
                    } else {
                        //  Otherwise the else clause is implicitly an <seq/>.
                        sofar = sofar.unify( Ginger::Arity( 0 ) );                        
                    }
                    element.putAttribute( this->arity_attribute, sofar.toString() );
                }
            }
		} else if ( x == SYSAPP && this->isEvalMode() ) {
			element.putAttribute( this->arity_attribute, Ginger::outArity( element.attribute( SYSAPP_NAME ) ).toString() );
			element.putAttribute( ARGS_ARITY, this->sumOverChildren( element ).toString() );
        } else if ( x == ERASE && this->isEvalMode() ) {
            if ( this->sumOverChildren( element ) == 0 ) {
                //  Replace in favour of sequence and allow subsequent stages to remove that.
                element.name() = SEQ;
                this->changed = true;
            }
            element.putAttribute( this->arity_attribute, "0" );
		} else if ( x == ASSERT && element.size() == 1 && this->isEvalMode() ) {
			if ( element.hasAttribute( ASSERT_N, "1" ) ) {
				if ( element.getChild( 0 )->hasAttribute( this->arity_attribute, "1" ) ) {
					//	As we can prove the child satisfies the condition we should 
					//	eliminate the parent. So we simply replace the contents of the 
					//	node with the contents of the child!
					Gnx c = element.getChild( 0 );
					element.copyFrom( *c );
					//	Because we have changed something we should let the system know
					//	in case it would synergise with other optimsations.
					this->changed = true;
				} else {
					element.putAttribute( this->arity_attribute, "1" );
				}
			} else if ( element.hasAttribute( ASSERT_TYPE ) ) {
				Gnx c = element.getChild( 0 );
				if ( c->hasName( CONSTANT ) && element.attribute( ASSERT_TYPE ) == c->attribute( CONSTANT_TYPE ) ) {
					element.copyFrom( *c );
					this->changed = true;
				} else {
					element.putAttribute( this->arity_attribute, "1" );
				}
			}
		} else if ( x == SELF_APP && this->isEvalMode() ) {
			this->self_app_pass_needed = true;
			element.putAttribute( ARGS_ARITY, this->sumOverChildren( element ).toString() );
		} else if ( x == VAR && this->isPatternMode() ) {
            element.putAttribute( this->arity_attribute, "1" );
        } else if ( x == TRY && N >= 1 ) {
            //  The non-escape arity is either the arity of the expression
            //  being tried or, if present, the catch.return arity.
            //  The overall arity is the sum of the arities of the 
            //  non-escape arity and the catch arities and the catch.else
            //  arity (if present).
            Ginger::Arity non_esc_arity( element.getChild( 0 )->attribute( this->arity_attribute, "0+" ) );
            for ( Ginger::Mnx::Generator g( element ); !!g; ++g ) {
                Gnx e = *g;
                if ( e->hasName( CATCH_RETURN ) ) {
                    non_esc_arity = e->attribute( this->arity_attribute, "0+" );
                }
            }
            if ( non_esc_arity.isntExact( 0 ) ) {
                //  This can't change, so stash now.
                element.putAttribute( this->arity_attribute, non_esc_arity.toString() );
            } else {
                Ginger::Arity sofar = non_esc_arity;
                for ( Ginger::Mnx::Generator g( element ); !!g; ++g ) {
                    Gnx e = *g;
                    if ( e->hasName( CATCH_THEN ) || e->hasName( CATCH_RETURN ) ) {
                        sofar = sofar.unify( e->attribute( this->arity_attribute, "0+" ) );
                    }
                }               
                element.putAttribute( this->arity_attribute, sofar.toString() );
            }
        } else if ( ( x == CATCH_THEN || x == CATCH_RETURN ) && N == 2 ) {
            element.putAttribute( this->arity_attribute, element.getChild( 1 )->attribute( this->arity_attribute, "0+" ) );
        }

        if ( element.hasAttribute( ANALYSIS_TYPE ) ) {
            //  Restore the cached analysis type.
            this->arity_attribute = element.attribute( ANALYSIS_TYPE ) == ARITY ? ARITY : PATTERN_ARITY;
            element.clearAttribute( ANALYSIS_TYPE );
        }
	}
	
public:
    ArityMarker( const char * arity_attribute, const bool clear ) : changed( false ), self_app_pass_needed( false ), clear_arities( clear ), arity_attribute( arity_attribute ) {}
    
    //  TODO: this constructor should be deprecated once we have the scaffolding in place for pattern.arities.
    ArityMarker( const bool clear ) : changed( false ), self_app_pass_needed( false ), clear_arities( clear ), arity_attribute( ARITY ) {}

	virtual ~ArityMarker() {}
};


class TailCall : public Ginger::MnxVisitor {

public:
	void startVisit( Ginger::Mnx & element ) {
		const string & x = element.name();
		element.clearAttribute( TAILCALL );	//	Throw away any previous marking.
		if ( x == FN ) {
			Gnx ch( element.getLastChild() );
			ch->orFlags( TAIL_CALL_MASK );
		} else if ( element.hasAllFlags( TAIL_CALL_MASK ) ) {
			if ( x == APP ) {
				element.putAttribute( TAILCALL, "true" );
			} else if ( x == IF ) {
				bool has_odd_kids = ( x.size() % 2 ) == 1;
				for ( int i = 1; i < element.size(); i += 2 ) {
					element.getChild( i )->orFlags( TAIL_CALL_MASK );
				}
				if ( has_odd_kids ) {
					element.getLastChild()->orFlags( TAIL_CALL_MASK );
				}
            } else if ( x == SWITCH && element.size() >= 1 ) {
                bool has_else = ( x.size() % 2 ) == 0;
                for ( int i = 2; i < element.size(); i += 2 ) {
                    element.getChild( i )->orFlags( TAIL_CALL_MASK );
                }
                if ( has_else ) {
                    element.getLastChild()->orFlags( TAIL_CALL_MASK );
                }
			} else if ( x == SEQ || x == BLOCK ) {
				if ( element.size() >= 1 ) {
					Gnx ch = element.getLastChild();
					ch->orFlags( TAIL_CALL_MASK );
				}
			} else if ( x == ASSERT && element.size() == 1 && element.hasAttribute( ASSERT_TAILCALL, "true" ) ) {
                Gnx c = element.getChild( 0 );
                element.copyFrom( *c );
                this->startVisit( element );
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
			if ( element.hasAttribute( SCOPE, "global" ) ) {
				if ( not element.hasAttribute( GNX_VID_DEF_PKG ) ) {
					const string & name = element.attribute( GNX_VID_NAME );
					const string & enc_pkg = element.hasAttribute( GNX_VID_ENC_PKG ) ? element.attribute( GNX_VID_ENC_PKG ) : this->package;
					if ( element.hasAttribute( "alias" ) ) {
						const string & alias = element.attribute( "alias" );
						//cout << "RESOLVE (QUALIFIED): name=" << name << ", alias=" << alias << ", enc.pkg=" << enc_pkg << endl;
						const string def_pkg( resolveQualified( this->project_folders, enc_pkg, alias, name, this->undefined_allowed ) );
						//cout << "   def = " << def_pkg << endl;
						element.putAttribute( GNX_VID_DEF_PKG, def_pkg );
					} else {
						//cerr << "RESOLVE (UNQUALIFIED): name=" << name << ", enc.pkg=" << enc_pkg << endl;
						const string def_pkg( resolveUnqualified( this->project_folders, enc_pkg, name, this->undefined_allowed ) );
						//cerr << "   def = " << def_pkg << endl;
						element.putAttribute( GNX_VID_DEF_PKG, def_pkg );						
					}
				}
			}
		} else if ( x == "var" ) {
			if ( element.hasAttribute( SCOPE, "global" ) ) {
				if ( not element.hasAttribute( GNX_VID_DEF_PKG ) ) {
					const string & enc_pkg = element.hasAttribute( GNX_VID_ENC_PKG ) ? element.attribute( GNX_VID_ENC_PKG ) : this->package;
					element.putAttribute( GNX_VID_DEF_PKG, enc_pkg );						
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

/*
	This pass does the following: 
	
	[1] It finds all references global variables and find their
		originating package. It caches this using the def.pkg attribute.

*/
class AltAbsolute : public Ginger::MnxVisitor {
private:
	vector< string > & project_folders;
	std::string package;
	const bool undefined_allowed;
		
public:
	void finaliseVisit( Ginger::Mnx & element ) {
		//	Now we route through fetchgnx -X with <fetch.resolve>... </>.

		TO BE DONE§

	    Ginger::Command cmd( FETCHGNX );
	    cmd.addArg( "-X" );
	   	{
			for ( 
				vector< string >::iterator it = this->project_folders.begin();
				it != this->project_folders.end();
				++it
			) {
				cmd.addArg( "-j" );
				cmd.addArg( *it );
			}
		}

	    Ginger::MnxBuilder qb;
	    qb.start( "fetch.resolve" );
	    qb.add( element );
	    qb.end();
	    shared< Ginger::Mnx > query( qb.build() );

	    cmd.runWithInputAndOutput();
	    const int fd = cmd.getInputFD();   
	    FILE * foutd = fdopen( cmd.getOutputFD(), "w" );
	    query->frender( foutd );
	    fflush( foutd );

	    stringstream prog;
	    for (;;) {
	        static char buffer[ 1024 ];
	        //  TODO: protect close with finally.
	        int n = read( fd, buffer, sizeof( buffer ) );
	        if ( n == 0 ) break;
	        if ( n == -1 ) {
	            if ( errno != EINTR ) {
	                perror( "" );
	                throw Ginger::Mishap( "Failed to read" );
	            }
	        } else if ( n > 0 ) {
	            prog.write( buffer, n );
	        }
	    }

	    //  TODO: protect close with finally.
	    fclose( foutd );
	
		Ginger::SaxParser parser( prog, resolve );
		parser.readElement();
	}

	void startVisit( Ginger::Mnx & element ) {
		const string & x = element.name();
		if ( x == ID ) {
			if ( element.hasAttribute( SCOPE, "global" ) ) {
				if ( not element.hasAttribute( GNX_VID_DEF_PKG ) ) {
					const string & name = element.attribute( GNX_VID_NAME );
					const string & enc_pkg = element.hasAttribute( GNX_VID_ENC_PKG ) ? element.attribute( GNX_VID_ENC_PKG ) : this->package;
					element.putAttribute( GNX_VID_ENC_PKG, enc_pkg );
					element.putAttribute( "fetchgnx", "resolve" );
					/*
					if ( element.hasAttribute( "alias" ) ) {
						const string & alias = element.attribute( "alias" );
						//cout << "RESOLVE (QUALIFIED): name=" << name << ", alias=" << alias << ", enc.pkg=" << enc_pkg << endl;
						const string def_pkg( resolveQualified( this->project_folders, enc_pkg, alias, name, this->undefined_allowed ) );
						//cout << "   def = " << def_pkg << endl;
						element.putAttribute( GNX_VID_DEF_PKG, def_pkg );
					} else {
						//cerr << "RESOLVE (UNQUALIFIED): name=" << name << ", enc.pkg=" << enc_pkg << endl;
						const string def_pkg( resolveUnqualified( this->project_folders, enc_pkg, name, this->undefined_allowed ) );
						//cerr << "   def = " << def_pkg << endl;
						element.putAttribute( GNX_VID_DEF_PKG, def_pkg );						
					}
					*/
				}
			}
		} else if ( x == "var" ) {
			if ( element.hasAttribute( SCOPE, "global" ) ) {
				if ( not element.hasAttribute( GNX_VID_DEF_PKG ) ) {
					const string & enc_pkg = element.hasAttribute( GNX_VID_ENC_PKG ) ? element.attribute( GNX_VID_ENC_PKG ) : this->package;
					element.putAttribute( GNX_VID_DEF_PKG, enc_pkg );	
				}
			}
		}
	}
	
	void endVisit( Ginger::Mnx & element ) {
	}
	
public:
	AltAbsolute( vector< string > & folders, const std::string & enc_pkg, const bool undefined_allowed ) : 
		project_folders( folders ),
		package( enc_pkg ),
		undefined_allowed( undefined_allowed )
	{
	}

	virtual ~AltAbsolute() {}
};



/*
	Self-reference analysis is responsible for simplifying a single feature,
	"named lambdas". It does this by reducing them to two language elements
		<self.constant/>
		<self.app> EXPRS* </self.app>
		
	In addition it demotes the name attribute of the fn element into a title
	attribute. i.e.
		<fn name=NAME ..> becomes <fn title=NAME>
	This demotion occurs if there is not already a title in place. If there
	is a title then the name attribute is simply discarded.
	
	Note that although BLOCKs and FORs introduce a new level of scope, they
	are not recognised in this analysis because they do not introduce a new
	dynamic scope.
*/

struct SelfInfo {
	Ginger::Mnx * 		element;	//	Not allowed to be NULL.
	const string *		name;		//	Might be NULL.

	SelfInfo( Ginger::Mnx * element, const string * name ) :
		element( element ),
		name( name )
	{}
	
	SelfInfo() :
		element( NULL ),
		name( NULL )
	{}
};

class SelfInfoFinder {
private:
	vector< SelfInfo > & self_info;
	SelfInfo * found;
	bool nested;

public:
	//	Returns true if the name is bound to a named-lambda.
	bool wasFn() {
		return this->found != NULL && this->found->element->name() == FN;
	}
	
	bool wasNested() {
		return this->nested;
	}
	
	Ginger::Mnx * element() {
		return this->found->element;
	}

	bool find( const std::string & name ) {
		for ( 
			vector< SelfInfo >::reverse_iterator it = this->self_info.rbegin();
			it != this->self_info.rend();
			++it
		) {
			if ( it->name != NULL && name == *it->name ) {
				this->found = &*it;
				//cerr << "Found " << name << ": nested = " << this->nested << ", element = " << it->element->name() << endl;
				return true;
			} else if ( it->element->name() == FN ) {
				//cerr << "Chaining out:" << it->element->name() << endl;
				this->nested = true;
			} else {
				//cerr << "Chaining out:" << it->element->name() << endl;
			}
		}
		//cerr << "Did not find " << name << endl;
		return false;
	}

public:
	SelfInfoFinder( vector< SelfInfo > & self_info ) : 
		self_info( self_info ),
		found( NULL ),
		nested( false )
	{}
};

class SelfAnalysis : public Ginger::MnxVisitor {
private:
	vector< int > scopes;
	vector< SelfInfo > self_info;
	
public:
	void startVisit( Ginger::Mnx & element ) {
		element.clearFlags( SELF_BIND_MASK );
		const string & nm = element.name();
		if ( nm == ID ) {
			SelfInfoFinder finder( self_info );
			if ( finder.find( element.attribute( GNX_VID_NAME ) ) && finder.wasFn() ) {
				if ( finder.wasNested() ) {
					finder.element()->orFlags( SELF_BIND_MASK );
				} else {
					Ginger::MnxBuilder b;
					b.start( SELF_CONSTANT );
					b.end();
					Gnx bgnx( b.build() );
					element.copyFrom( *bgnx );
				}
			}
		} else if ( nm == VAR ) {
            if ( element.hasAttribute( GNX_VID_NAME ) ) {
    			//	Efficiency hack - only push the variable if it masks a 
    			//	function name! This keeps the length of the stack low which
    			//	is what pushes this to be an O(N^2) algorithm. 
    			const string & name = element.attribute( GNX_VID_NAME );
    			SelfInfoFinder finder( self_info );
    			if ( finder.find( name ) ) {
    				this->self_info.push_back( SelfInfo( &element, &name ) );
    			}
            } else {
                //  Anonymous variable - cannot mask a function name.
                //  No action needed.
            }
		} else if ( nm == FN ) {
			this->scopes.push_back( this->self_info.size() );
			this->self_info.push_back( 
				SelfInfo( 
					&element, 
					element.hasAttribute( FN_NAME ) ? &element.attribute( FN_NAME ) : NULL
				)
			);
		}
	}
	
	void endVisit( Ginger::Mnx & element ) {
		const string & nm = element.name();
		if ( 
			nm == APP &&
			element.size() >= 1 &&
			element.getChild( 0 )->name() == "self.constant"
		) {
			element.name() = "self.app";
			element.popFrontChild();
		} else if ( nm == FN ) {
			int n = this->scopes.back();
			this->scopes.pop_back();
			this->self_info.resize( n );
			if ( element.hasAllFlags( SELF_BIND_MASK ) ) {
				Ginger::MnxBuilder mnx;
				mnx.start( SEQ );
				mnx.start( BIND );
				mnx.start( VAR );
				mnx.put( GNX_VID_PROTECTED, "true" );
				mnx.put( GNX_VID_NAME, element.attribute( FN_NAME ) );
				mnx.end();
				mnx.start( SELF_CONSTANT );
				mnx.end();
				mnx.end();
				mnx.add( element.getChild( 1 ) );
				mnx.end();
				element.setChild( 1, mnx.build() );
			}
		}
	}
public:
	SelfAnalysis() {
		//cerr << "SELF ANALYSIS" << endl;
	}
	
	~SelfAnalysis() {}
};



struct VarInfo {
	Ginger::Mnx *			element;
	const string *			name;
	int						uid;
	size_t					dec_level;
    bool                    is_temporary;
    bool                    is_protected;
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
	
	VarInfo( Ginger::Mnx * element, const string * name, int uid, size_t dec_level, bool is_temporary, bool is_protected, bool is_local ) : 
		element( element ),
		name( name ),
		uid( uid ),
		dec_level( dec_level ),
        is_temporary( is_temporary ),
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
	
	[4] It marks VARs and IDs as to whether or not they are assigned to.
	
	It is very important to note that although BLOCKs and FORs introduces
	scopes they are NOT recognised in this analysis. This is because
	the only use of the scopes is to perform outer-analysis.
	
*/
class Scope : public Ginger::MnxVisitor {
private:
	int var_uid;
	bool outers_found;

	//	These maps are used to propagate assignment info.
	set< int > is_assigned_to;
	map< int, Ginger:: Mnx * > var_of_uid;
	map< int, list< Ginger::Mnx * > > xrefs_to_uid;
	
	vector< int > scopes;		//	How many variables were declared in each of the nested scopes.
	vector< VarInfo > vars;		//	A nested stack of variables, scoped using the counts in "scopes".
	
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
	
	VarInfo * findId( const std::string & name, const bool is_tmp ) {
		for ( 
			vector< VarInfo >::reverse_iterator it = this->vars.rbegin();
			it != this->vars.rend();
			++it
		) {
            VarInfo & vi = *it;
			if ( *vi.name == name && is_tmp == vi.is_temporary ) {
				long d = this->scopes.size() - vi.dec_level;
				if ( vi.is_local && d != 0 ) {
					//cerr << "We have detected an outer declaration: " << name << endl;
					//cerr << "Declared at level " << it->dec_level << " but used at level " << this->scopes.size() << endl;
					vi.max_diff_use_level = max( d, vi.max_diff_use_level );
					this->outers_found = true;
					vi.element->putAttribute( IS_OUTER, "true" );
				}
				return &*it;
			}
		}
		return NULL;
	}

	void markAsAssigned( Gnx id ) {
		if ( id->hasAttribute( PROTECTED, "true" ) ) {
			throw Ginger::Mishap( "Assigning to a protected variable" ).culprit( "Variable", id->attribute( GNX_VID_NAME, "<unknown>" ) );
		} else if ( id->hasAttribute( SCOPE, "local" ) ) {
			stringstream uidstream( id->attribute( UID, "" ) );
			int uid;
			if ( uidstream >> uid ) {
				this->is_assigned_to.insert( uid );
			} else {
				throw SystemError( "Missing UID" ).culprit( "GNX", id->toString() );
			}
		}
	}
	
public:
	bool wereOutersFound() {
		return this->outers_found;
	}
	
	void startVisit( Ginger::Mnx & element ) {
		const string & x = element.name();
		element.clearAttribute( UID );			//	Throw away any previous marking.
		element.clearAttribute( SCOPE );		//	Throw away any previous marking.
		element.clearAttribute( OUTER_LEVEL );	//	Throw away any previous marking.
		element.clearAttribute( IS_OUTER );		//	Throw away any previous marking.
		element.clearAttribute( IS_ASSIGNED );	//	Throw away any previous marking.
		if ( x == ID ) {
			const string & name = element.attribute( GNX_VID_NAME );
            const bool is_tmp = element.hasAttribute( IS_TEMPORARY, "true" );
			VarInfo * v = this->findId( name, is_tmp );
			if ( v != NULL ) {
				element.putAttribute( UID, v->uid );
				this->xrefs_to_uid[ v->uid ].push_back( &element );
			}
			if ( v == NULL || v->isGlobal() ) {
				element.putAttribute( SCOPE, "global" );
			} else {
				element.putAttribute( SCOPE, "local" );
				if ( v->max_diff_use_level > 0 ) {
					element.putAttribute( OUTER_LEVEL, v->max_diff_use_level );
				}
			}
			if ( v != NULL ) {
				element.putAttribute( PROTECTED, v->is_protected ? "true" : "false" );
			}
		} else if ( x == VAR ) {
            if ( element.hasAttribute( GNX_VID_NAME ) ) {
    			const string & name = element.attribute( GNX_VID_NAME );
    			const int uid = this->newUid();
    			this->var_of_uid[ uid ] = &element;
    			element.putAttribute( UID, uid );
                const bool is_temporary = element.hasAttribute( IS_TEMPORARY, "true" );
    			if ( not element.hasAttribute( PROTECTED ) ) {
    				element.putAttribute( PROTECTED, "true" );
    			}
    			const bool is_protected = element.attribute( PROTECTED ) == "true";
    			const bool is_global = this->isGlobalScope();
    			element.putAttribute( SCOPE, is_global ? "global" : "local" );
    			//cerr << "Declaring " << name << " at level " << this->vars.size() << endl;
    			//cerr << "  -- but should it be " << this->scopes.size() << endl;
    			this->vars.push_back( VarInfo( &element, &name, uid, this->scopes.size(), is_temporary, is_protected, not is_global ) );
            } else {
                //  Anonymous variable - should be marked as having local scope.
                //  It also needs a unique ID to keep the slot counting code clean.
                element.putAttribute( SCOPE, "local" );
                element.putAttribute( UID, this->newUid() );
                element.putAttribute( PROTECTED, "true" );      //  Not strictly required but correct & future-proofing.
            }
		} else if ( x == FN ) {
			this->scopes.push_back( this->vars.size() );
		}
	}
	
	void endVisit( Ginger::Mnx & element ) {
		const string & x = element.name();
		if ( x == FN ) {
			int n = this->scopes.back();
			this->scopes.pop_back();
			this->vars.resize( n );
		} else if ( 
			x == SET && 
			element.size() == 2 &&
			element.getChild( 1 )->name() == ID 
		) {
			this->markAsAssigned( element.getChild( 1 ) );
		}
	}
	
public:
	Scope() : 
		var_uid( 0 ),
		outers_found( false )
	{
	}

	virtual ~Scope() {
		//	Now mark all assigned var+id elements as IS_ASSIGNED. This
		//	is useful for lifting and also for constant folding (to be done).
		for ( 
			set< int >::iterator it = this->is_assigned_to.begin();
			it != this->is_assigned_to.end();
			++it
		) {
			//cerr << endl;
			//cerr << "VAR[" << this->var_of_uid[ *it ]->attribute( GNX_VID_NAME ) << "] has " << xrefs.size() << " references" << endl;
			//	Mark the var declaration as IS_ASSIGNED.
			this->var_of_uid[ *it ]->putAttribute( IS_ASSIGNED, "true" );
			//	And now mark all the id cross-refs to that declaration the same.
			list< Ginger::Mnx * > & xrefs = this->xrefs_to_uid[ *it ];		
			for (
				list< Ginger::Mnx * >::iterator xt = xrefs.begin();
				xt != xrefs.end();
				++xt
			) {
				(*xt)->putAttribute( IS_ASSIGNED, "true" );
			}
		}
	}
};

/*
	This pass does the following tasks.
	
	[1] It finds all references to the ginger.library, which are guaranteed
		to be built-in functions. It replaces these references with 
		<constant type="sysfn"> constants.
		
		If this transformation is successful, it does not need to force a
		further pass. The action (#2) it needs to combine with happens on the
		way up.
		
	[2] It replaces all <app><constant type="sysfn" .../>...</app> forms with the especially
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
			if ( element.hasAttribute( GNX_VID_DEF_PKG, "ginger.library" ) ) {
				//cout<<"IN 1" << endl;
				const string name( element.attribute( GNX_VID_NAME ) );
				element.clearAllAttributes();
				element.putAttribute( CONSTANT_VALUE, Ginger::baseName( name ) );
				element.putAttribute( CONSTANT_TYPE, "sysfn" );
				element.name() = CONSTANT;
				this->changed = true;
				//cout<<"OUT 1" << endl;
			}
		} else if ( x == APP && element.size() == 2 ) {
			if ( element.getChild( 0 )->hasName( CONSTANT ) && element.getChild( 0 )->hasAttribute( CONSTANT_TYPE, "sysfn" ) && element.getChild( 0 )->hasAttribute( CONSTANT_VALUE ) ) {
				//cout<<"IN 2" << endl;
				const string value( element.getChild( 0 )->attribute( CONSTANT_VALUE ) );
				element.name() = SYSAPP;
				element.clearAllAttributes();
				element.putAttribute( SYSAPP_NAME, value );
				element.popFrontChild();
				this->changed = true;
				//cout<<"OUT 2" << endl;
				//element.render();
				//cout << endl;
			}
		} else if ( x == SET && element.size() == 2 ) {
            if ( element.getChild( 1 )->hasName( SYSAPP ) && element.getChild( 1 )->hasAttribute( SYSAPP_NAME ) ) {
                const string & sysfn_name = element.getChild( 1 )->attribute( SYSAPP_NAME );
                const char * uname = (
                    sysfn_name == "index" ? "updaterOfIndex" :
                    sysfn_name == "explode" ? "updaterOfExplode" :
                    NULL
                );
                if ( uname != NULL ) {
                    /*
                        <set> EXPR <sysapp name=A> ARGS* </sysapp> </set>
                        <sysapp name=U> EXPR ARGS </sysapp>
                    */
                    Gnx uform( new Ginger::Mnx( SYSAPP ) );
                    uform->putAttribute( SYSAPP_NAME, uname );
                    uform->addChild( element.getChild( 0 ) );
                    Ginger::MnxChildIterator kids( element.getChild(1) );
                    while ( kids.hasNext() ) {
                        uform->addChild( kids.next() );
                    }
                    element.copyFrom( *uform );
                    this->changed = true;
                }
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
					
					#ifdef DBG_SIMPLIFYGNX
                        cerr << "simplifygnx capture sets" << endl;
						int k = 0;
						for (
							vector< set< string > >::iterator jt = this->capture_sets.begin();
							jt != this->capture_sets.end();
							++jt, k++
						) {
							cerr << "  Capture set [" << k << "] size = " << jt->size() << endl; 
						}
					#endif
				}
			}
		} else if ( x == FN ) {
			this->capture_sets.push_back( set< string >() );
		}
	}
	
	//	method
	Gnx makeLocals( const char * tag ) {
		Gnx locals( new Ginger::Mnx( SEQ ) );
		for ( 
			set< string >::iterator it = this->capture_sets.back().begin();
			it != this->capture_sets.back().end();
			++it
		) {
			const string & uid = *it;
			Ginger::Mnx * v = this->dec_outer[ uid ];
			if ( v == NULL ) throw SystemError( "Internal error" );
			//cout << "SO FAR: ";
			/*v->render();
			cout << endl;*/
			Gnx var( new Ginger::Mnx( tag ) );
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

			
			if ( not this->capture_sets.back().empty() ) {
				
				//	We should extend the set of local variables with the
				//	capture set.
				{
					Gnx arg( element.getChild( 0 ) );
					Gnx newarg( new Ginger::Mnx( SEQ ) );
					newarg->addChild( arg );
					newarg->addChild( this->makeLocals( VAR ) );
					
					/*newarg->render();
					cout << endl;*/
					
					element.setChild( 0, newarg );
				}
				
				//	Then we should transform the function into a call
				//	to partapply onto the original function and 
				//	the capture set.
				{
					Gnx fn( new Ginger::Mnx( FN ) );
					fn->copyFrom( element );
					Gnx partapply( new Ginger::Mnx( SYSAPP ) );
					partapply->putAttribute( SYSAPP_NAME, "partApply" );
					partapply->addChild( this->makeLocals( ID ) );
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

class TopLevelFunction : public Ginger::MnxVisitor {
private:
	vector< string > guids;
	vector< Gnx > fns;
	int nesting;
	
public:
	Gnx finishByInsertingBindings( Gnx element ) {
		Ginger::MnxBuilder b;
		b.start( SEQ );
		
		for ( size_t i = 0; i < this->guids.size(); i++ ) {
			const string & g = this->guids[ i ];
			Gnx m( this->fns[ i ] );
			
			b.start( BIND );
			b.start( VAR );
			b.put( GNX_VID_PROTECTED, "true" );
			b.put( GNX_VID_NAME, g );						
			b.put( GNX_VID_DEF_PKG, "ginger.temporaries" );
			b.put( GNX_VID_SCOPE, "global" );			
            b.end();
			b.add( m );
			b.end();
			
		}
		
		b.add( element );
		b.end();
		return b.build();
	}

	void startVisit( Ginger::Mnx & element ) {
		if ( element.name() == FN ) {
			this->nesting += 1;			
		}
	}

	void endVisit( Ginger::Mnx & element ) {
		if ( element.name() == FN ) {
			this->nesting -= 1;
			if ( this->nesting > 0 ) {
				//	We have a nested function definition. Lift it to
				//	the top level.
				
				//	Copy the current element into a temporary.
				Gnx t( new Ginger::Mnx( "" ) );
				t->copyFrom( element );
				
				//	Create globally unique ID.
				const string name( makeGUID() );
				this->guids.push_back( name );
				this->fns.push_back( t );

				
				//	Now replace the current element.
				{
					Ginger::MnxBuilder b;
					b.start( ID );
					b.put( GNX_VID_PROTECTED, "true" );
					b.put( GNX_VID_NAME, name );						
					b.put( GNX_VID_DEF_PKG, "ginger.temporaries" );
					b.put( GNX_VID_SCOPE, "global" );
					b.end();
					element.copyFrom( *b.build() );					
				}
			}
		}
	}
	
public:
	TopLevelFunction() : nesting( 0 ) {}
};

class AddDeref : public Ginger::MnxVisitor {
public:
	
	//	This has to EITHER be an endVisit or add a processed flag.
	//	If it is a startVisit, it might generate an infinite loop, generating an
	//	infinitely deep chain of derefs. So we alter the IS_OUTER flag attribute
	//	and test it to inhibit the loop. [Or we could just move it to be an 
	//	endVisit]
	//
	void startVisit( Ginger::Mnx & element ) {
		const string & x = element.name();
		if ( element.hasAttribute( IS_OUTER, "true" ) && element.hasAttribute( PROTECTED, "false" ) && ( x == VAR || x == ID ) ) {
			element.putAttribute( IS_OUTER, "deref" );			//	Mark as processed.
			Gnx t( new Ginger::Mnx( x ) );
			t->copyFrom( element );
			Gnx d( new Ginger::Mnx( "deref" ) );
			d->addChild( t );
			element.copyFrom( *d );
		}	
	}
	
	void endVisit( Ginger::Mnx & element ) {
		const string & x = element.name();
		if ( 
			x == BIND && 
			element.size() == 2 &&
			element.getChild(0)->name() == DEREF &&
			element.getChild(0)->size() == 1
		) {
			//	<bind><deref><var...></deref>EXPR</bind> 
			//	<bind><var...><makeref>EXPR</makeref></bind>			
			Gnx makeref( new Ginger::Mnx( MAKEREF ) );
			makeref->addChild( element.getChild(1) );
			element.setChild( 0, element.getChild(0)->getChild(0) );
			element.setChild( 1, makeref );
		} else if (
			x == SET &&
			element.size() == 2 &&
			element.getChild(1)->name() == DEREF &&
			element.getChild(1)->size() == 1
		) {
			//	<set> EXPR <deref><var...></deref> </bind> 
			//	<setcont> EXPR <var...> </bind>		
			element.name() = SETCONT;
			element.setChild( 1, element.getChild(1)->getChild(0) );
		} 
	}
	
public:
	virtual ~AddDeref() {}
};

class Match : public Ginger::MnxVisitor {
private:
    bool    has_changed;

public:
    bool hasChanged() const {
        return this->hasChanged();
    }

    static int flatten( Gnx x, vector< Gnx > & lhs ) {
        int count = 0;
        if ( x->hasName( SEQ ) ) {
            count += 1;
            for ( Ginger::MnxChildIterator chit( x ); chit.hasNext(); ) {
                Gnx g = chit.next();
                count += flatten( g, lhs );
            }
        } else {
            lhs.push_back( x );
        }
        return count;
    }

public:
    void startVisit( Ginger::Mnx & element ) {
        if ( 
            (
                element.hasName( IN ) ||
                element.hasName( FROM ) ||
                element.hasName( BIND )
            ) && 
            element.size() >= 1 
        ) {
            Gnx target = element.getChild( 0 );
            if ( target->hasName( SEQ ) ) {
                vector< Gnx > lhs;
                bool was_flattened = flatten( target, lhs ) > 1;
                if ( lhs.size() == 1 ) {
                    target->copyFrom( *lhs[ 0 ] );
                } else if ( was_flattened ) {
                    Ginger::MnxBuilder e;
                    e.start( SEQ );
                    for ( 
                        vector< Gnx >::iterator it = lhs.begin();
                        it != lhs.end();
                        ++it
                    ) {
                        Gnx & g = *it;
                        e.add( g );
                    }
                    e.end();
                    target->copyFrom( *e.build() );
                }
            }
            if ( target->hasName( CONSTANT ) ) {
                const string uuid = makeGUID();
                Ginger::MnxBuilder where;
                where.start( WHERE );
                where.start( element.name() );
                where.start( VAR );
                where.put( GNX_VID_NAME, uuid );
                where.put( IS_TEMPORARY, "true" );
                where.end();
                bool not_first = false;
                for ( Ginger::MnxChildIterator chit( element ); chit.hasNext(); not_first = true ) {
                    Gnx g = chit.next();
                    if ( not_first ) {
                        where.add( g );
                    }
                }
                //where.add( element.getChild( 1 ) );
                where.end();
                where.start( SYSAPP );
                where.put( SYSAPP_NAME, "=" );
                where.add( element.getChild( 0 ) );
                where.start( ID );
                where.put( GNX_VID_NAME, uuid );
                where.put( IS_TEMPORARY, "true" );
                where.end();
                where.end();
                where.end();
                Gnx w = where.build();
                element.copyFrom( *w );
                this->has_changed = true;
            }
        }
    }
    void endVisit( Ginger::Mnx & element ) {
    }
public:
    Match() : has_changed( false ) {}
    virtual ~Match() {}
};


class Flatten : public Ginger::MnxVisitor {
private:
	void flattenSubSeqs( Ginger::Mnx & element ) {
		for ( int i = 0; i < element.size(); i++ ) {
			if ( element.getChild( i )->name() == SEQ ) {
				element.flattenChild( i );
				i -= 1;
			}
		}
	}

public:

	void startVisit( Ginger::Mnx & element ) {
		const string & nm = element.name();
		if ( nm == SYSAPP || nm == SEQ || nm == LIST || nm == VECTOR || nm == ERASE ) {
			this->flattenSubSeqs( element );
		} 
	}

	void endVisit( Ginger::Mnx & element ) {
		const string & nm = element.name();
		if ( 
			nm == LIST_APPEND and 
			element.size() == 2 
		) {
			if ( element.getChild( 0 )->name() == LIST and element.getChild( 0 )->isEmpty() ) {
				element.copyFrom( *element.getChild( 1 ) );
			} else if ( element.getChild( 1  )->name() == LIST and element.getChild( 1 )->isEmpty() ) {
				element.copyFrom( *element.getChild( 0 ) );
			} else if ( element.getChild( 0 )->name() == LIST and element.getChild( 1 )->name() == LIST ) {
				// 	Does LIST/VECTOR introduce a lexical block? In this optimisation
				//	we assume it does not.
				element.name() = LIST;
				element.getChild(0)->name() = SEQ;
				element.getChild(1)->name() = SEQ;
				this->flattenSubSeqs( element );
			}
		}
	}

public:
	Flatten() {}
	virtual ~Flatten() {}
};

class StripOuterAttributes : public Ginger::MnxVisitor {
public:
	void startVisit( Ginger::Mnx & element ) {
		element.clearAttribute( IS_OUTER );
		element.clearAttribute( OUTER_LEVEL );
	}
	
	void endVisit( Ginger::Mnx & element ) {
	}	

public:
	StripOuterAttributes() {}
	virtual ~StripOuterAttributes() {}
};

/**	The responsibility of this class is to determine the layout of
	the stack frame by
	(1) determining the number of arguments to take off the stack.
	(2) the total size of the stack frame, ignoring temporaries.
	(3) assign indexes into the stack frame (slots) for inner variables.
*/
class SlotAllocation : public Ginger::MnxWalker {
private:
	int count;					//	A count of the local variables allocated so far.
	int maxcount;				//	High tide marker for count.
	vector< int > scopes;		//	A stack of count, roughly corresponding to counts.
	map< int, int > uid_slot;	//	Maps from UID to slots.
	
private:
    static bool isBranchIf( Ginger::MnxWalkPath * path ) {
        return (
            path != NULL && 
            path->getMnx().name() == IF && 
            (
                ( ( path->getIndex() & 0x1 ) == 0x1 ) ||
                path->isLastIndex()
            )
        );
    }
    
    static bool isBranchSwitch( Ginger::MnxWalkPath * path ) {
        return (
            path != NULL && 
            path->getMnx().name() == SWITCH && 
            (
                ( ( path->getIndex() & 0x1 ) == 0x0 ) ||
                path->isLastIndex()
            )
        );
    }
    
	static bool isFormalArgs( Ginger::MnxWalkPath * path ) {
		return path != NULL && path->getMnx().name() == FN && path->getIndex() == 0;
	}
	
	static Ginger::Mnx & getParent( Ginger::MnxWalkPath * path ) {
		return path->getMnx();
	}
	
	void popCount() {
		this->count = this->scopes[ this->scopes.size() - 1 ];
		this->scopes.pop_back();
	}
	
	void pushCount() {
		this->scopes.push_back( this->count );
	}
	
	int bumpCount() {
		const int n = this->count++;
		if ( this->maxcount < this->count ) {
			this->maxcount = this->count;
		}
		return n;
	}
	
public:
	void startWalk( Ginger::Mnx & element, Ginger::MnxWalkPath * path ) {
		const string & x = element.name();
		if ( x == ID && element.hasAttribute( "scope", "local" ) ) {
			int slot = this->uid_slot[ element.attributeToInt( "uid" ) ];
			element.putAttribute( "slot", slot );
		} else if ( x == VAR && element.hasAttribute( "scope", "local" ) ) {
			int slot = this->bumpCount();
			this->uid_slot[ element.attributeToInt( "uid" ) ] = slot;
			element.putAttribute( "slot", slot );
		} else if ( x == FN ) {
			this->pushCount();
			this->count = 0;
		} else if ( x == BLOCK || x == FOR || isBranchIf( path ) || isBranchSwitch( path ) ) {
			this->pushCount();
		}
	}
	
	void endWalk( Ginger::Mnx & element, Ginger::MnxWalkPath * path ) {
		const string & x = element.name();
		if ( x == FN ) {
			element.putAttribute( "locals.count", this->maxcount );
			this->popCount();			
		} else if ( x == BLOCK || x == FOR || isBranchIf( path ) || isBranchSwitch( path ) ) {
			this->popCount();
            if ( x == BLOCK ) {
                element.name() = SEQ;
            }
		} else if ( isFormalArgs( path ) ) {
			//	We are ending an argument list. Stash the count of the formals.
			getParent( path ).putAttribute( "args.count", this->count );
		}
	}	

public:
	SlotAllocation() : count( 0 ), maxcount( 0 ) {}
	virtual ~SlotAllocation() {}
};

bool Main::resimplify( Gnx & g ) {
	bool resimplify = false;
	
	if ( this->getSysappProcessing() ) {	
		SysFold s;
		g->visit( s );
		//	We are not interested in the changed flag at this point
		//	because folding is part of the monotonic change.
	}
	
	if ( this->getFlattenProcessing() ) {
		Flatten flatten;
		g->visit( flatten );
	}
	
	if ( this->getArityProcessing() ) {
		ArityMarker a( true );
		g->visit( a );
		//	We *do* care about changing here as it breaks the
		//	otherwise monotonic process.
		resimplify = resimplify || a.hasChanged();
		if ( not resimplify && a.hasSelfAppPassNeeded() ) {
			SelfAppArityMarker m;
			g->visit( m );
			ArityMarker b( false );
			g->visit( b );
		}
	}
	
	return resimplify;
}

void Main::simplify( Gnx & g ) {
	bool resimplify = false;

	//	If we are not doing absolute_processing we should assume we
	//	will find outers.
	bool outers_found = not this->getAbsoluteProcessing();
	
	if ( this->getSelfProcessing() ) {
		SelfAnalysis self;
		g->visit( self );
	}

    if ( this->getMatchProcessing() ) {
        Match match;
        g->visit( match );
    }
	
	if ( this->getScopeProcessing() ) {
		Scope scope;
		g->visit( scope );
		outers_found = scope.wereOutersFound();
	}
	
	if ( this->getAbsoluteProcessing() ) {
		Absolute a( this->project_folders, this->package, this->undefined_allowed );
		g->visit( a );
	} 

	if ( this->getSysappProcessing() ) {	
		SysFold s;
		g->visit( s );
		//	N.B. We do not care about the changed flag as it cannot
		//	cause rework at this point.
	}
	
	if ( this->getLiftProcessing() ) {
		//	Only proceed if we have determined that there are indeed
		//	some variables which are declared as outers. Otherwise
		//	this is just an expensive no-op!
		if ( outers_found ) {
			Lift lift;
			g->visit( lift );
			
			AddDeref addd;
			g->visit( addd );
		}
		
		if ( not retain_debug_info ) {
			StripOuterAttributes strip;
			g->visit( strip );
		}
	}
	
	if ( this->getFlattenProcessing() ) {
		Flatten flatten;
		g->visit( flatten );
	}
	
	if ( this->getArityProcessing() ) {
		ArityMarker a( true );
		g->visit( a );
		resimplify = resimplify || a.hasChanged();
		if ( not resimplify && a.hasSelfAppPassNeeded() ) {
			SelfAppArityMarker m;
			g->visit( m );
			ArityMarker b( false );
			g->visit( b );
		}
	}
	
	while ( resimplify ) {
		resimplify = this->resimplify( g );
	}
	
	if ( this->getTopLevelProcessing() ) {
		TopLevelFunction top;
		g->visit( top );
		g = top.finishByInsertingBindings( g );
	}

	if ( this->getTailcallProcessing() ) {
		g->orFlags( TAIL_CALL_MASK );
		TailCall tc;
		g->visit( tc );
	}
	
	if ( this->getSlotProcessing() ) {
		SlotAllocation s;
		g->walk( s );
	}
	
}

void Main::run() {
	Ginger::MnxReader reader;	
	for (;;) {	
		Gnx g( reader.readMnx() );
		if ( not g ) break;	
		this->simplify( g );		
		g->render();
		cout << endl;
        #ifdef DBG_SIMPLIFYGNX
            cerr << "simplifygnx replying" << endl;
            cerr << "  [[" ;
            g->render( cerr );
            cerr << "]]" << endl;
        #endif		
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
	} catch ( Ginger::Mishap & p ) {
		p.culprit( "Detected by", SIMPLIFYGNX );
		p.gnxReport();
		return EXIT_FAILURE;
	}
}
