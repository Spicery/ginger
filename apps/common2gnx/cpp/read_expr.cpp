#include <stdexcept>
#include <string>
#include <sstream>

#include <stddef.h>
#include <stdio.h>
#include <assert.h>

#include "gnxconstants.hpp"
#include "shared.hpp"
#include "mishap.hpp"

#include "item_factory.hpp"
#include "read_expr.hpp"
#include "item.hpp"
#include "role.hpp"
#include "sysconst.hpp"
#include "gnxconstants.hpp"

//#define DBG_COMMON2GNX

using namespace std;


typedef Ginger::MnxBuilder NodeFactory;

static Node makeApp( Node lhs, Node rhs ) {
	if ( lhs->name() == SYSFN ) {
		NodeFactory sysapp;
		sysapp.start( SYSAPP );
		std::string name = lhs->attribute( SYSFN_VALUE );
		sysapp.put( SYSAPP_NAME, name );
		sysapp.add( rhs );
		sysapp.end();
		return sysapp.build();
	} else {
		NodeFactory app;
		app.start( APP );
		app.add( lhs );
		app.add( rhs );
		app.end();
		return app.build();
	}
}

static Node makeIndex( Node lhs, Node rhs ) {
	NodeFactory index;
	index.start( SYSAPP );
	index.put( SYSAPP_NAME, "index" );
	index.add( lhs );
	index.add( rhs );
	index.end();
	return index.build();
}

#ifdef NOT_CURRENTLY_USED_BUT_KEEP
static void pushStringConstant( NodeFactory & f, const string & s ) {
	f.start( CONSTANT );
	f.put( CONSTANT_TYPE, "string" );
	f.put( CONSTANT_VALUE, s );
	f.end();
}

static void pushAnyPattern( NodeFactory & f ) {
	f.start( VAR );
	f.put( VID_PROTECTED, "true" );
	f.end();
}
#endif


static Node makeEmpty() {
	return shared< Ginger::Mnx >( new Ginger::Mnx( SEQ ) );
}

static void updateAsPattern( Node node, const bool val_vs_var ) {
	if ( node->hasName( ID ) ) {
		node->name() = VAR;
		node->putAttribute( VID_PROTECTED, val_vs_var ? "true" : "false" );
	}
}


string ReadStateClass::readPkgName() {
	Item it = this->item_factory->read();
	return it->nameString();
}

const string ReadStateClass::readIdName() {
	Item it = this->item_factory->read();
	if ( it->tok_type != tokty_id ) {
		throw Ginger::Mishap( "Identifier expected" ).culprit( "Found", it->nameString() );
	}
	return it->nameString();
}

Node ReadStateClass::readExprPrec( int prec ) {
	Node e = this->readOptExprPrec( prec );
	if ( not e ) {
		Item it = this->item_factory->peek();
		if ( it->item_is_anyfix() ) {
			throw Ginger::Mishap( "Found reserved word" ).culprit( it->nameString() );
		} else {
			throw Ginger::Mishap( "Unexpected end of expression" ).culprit( it->nameString() );
		}
	}
	return e;
}

void ReadStateClass::checkToken( TokType fnc ) {
	Item it = this->item_factory->read();
	if ( it->tok_type != fnc ) {
		throw Ginger::Mishap( "Unexpected token" ).culprit( "Found", it->nameString() ).culprit( "Expected", tok_type_name( fnc ) );
	}
}

bool ReadStateClass::tryPeekToken( TokType fnc ) {
	Item it = this->item_factory->peek();
	return it->tok_type == fnc;
}

void ReadStateClass::checkPeekToken( TokType fnc ) {
	Item it = this->item_factory->peek();
	if ( it->tok_type != fnc ) {
		throw Ginger::Mishap( "Unexpected token" ).culprit( "Found", it->nameString() ).culprit( "Expected", tok_type_name( fnc ) );
	}
}

bool ReadStateClass::tryName( const char * name ) {
	ItemFactory ifact = this->item_factory;
	Item it = ifact->peek();
	if ( it->nameString() == name ) {
		ifact->drop();
		return true;
	} else {
		return false;
	}
}

bool ReadStateClass::tryToken( TokType fnc ) {
	ItemFactory ifact = this->item_factory;
	if ( ifact->peek()->tok_type == fnc ) {
		ifact->drop();
		return true;
	} else {
		return false;
	}
}

Node ReadStateClass::readCompoundStmnts( bool obrace_read ) {
	if ( this->cstyle_mode ) {
		NodeFactory stmnts;
		stmnts.start( SEQ );
		if ( not obrace_read ) this->checkToken( tokty_obrace );
		while ( not this->tryToken( tokty_cbrace ) ) {
			Node n = this->readSingleStmnt();
			stmnts.add( n );
		}
		stmnts.end();
		return stmnts.build();
	} else {
		return this->readStmnts();
	}
}

Node ReadStateClass::readSingleStmnt() {
	if ( this->cstyle_mode ) {
		Item it = this->item_factory->read();
		//cerr << "SINGLE " << tok_type_name( it->tok_type ) << endl;
		switch ( it->tok_type ) {
			case tokty_obrace: 
				return this->readCompoundStmnts( true );
			case tokty_function: 
				if ( this->cstyle_mode && this->item_factory->peek()->tok_type == tokty_oparen ) {
					return this->readLambda();
				} else {
					return this->readDefinition();
				}
			case tokty_if: 
				return this->readIf( tokty_if, tokty_endif );
			case tokty_for: 
				return this->readFor();
			case tokty_switch: 
				return this->readSwitch();
			case tokty_recordclass:
				return this->readRecordClass();
			case tokty_dsemi:
			case tokty_semi: 
				return makeEmpty();
			default: 
				this->item_factory->unread();
				// --- Fall-through --- //
		}
	}
	
	//	Fall thru!
	Node n = this->readOptEmptyExpr();
	if ( this-> tryToken( tokty_dsemi ) ) {
		NodeFactory f;
		f.start( ERASE );
		f.add( n );
		f.end();
		return f.build();
	} else {
		this->checkToken( tokty_semi );
		return n;
	}
}

Node ReadStateClass::readStmnts() {
	if ( this->cstyle_mode ) {
		return this->readSingleStmnt();
	} else {
		return this->readOptEmptyExprPrec( prec_max );
	}
}


Node ReadStateClass::readStmntsCheck( TokType fnc ) {
	Node t = this->readStmnts();
	this->checkToken( fnc );
	return t;
}
	
Node ReadStateClass::readExpr() {
	return this->readExprPrec( prec_semi );
}

//	1st approximation. It is too permissive as it stands.
Node ReadStateClass::readQuery() {
	return this->readExpr();
}

Node ReadStateClass::readOptExpr() {
	return this->readOptExprPrec( prec_semi );
}

Node ReadStateClass::readOptEmptyExprPrec( int prec ) {
	Node n = this->readOptExprPrec( prec);
	if ( not n ) {
		return makeEmpty();
	} else {
		return n;
	}
}

Node ReadStateClass::readOptEmptyExpr() {
	return this->readOptEmptyExprPrec( prec_semi );
}

Node ReadStateClass::readOptEmptyExprCheck( TokType fnc ) {
	Node n = this->readOptEmptyExpr();
	this->checkToken( fnc );
	return n;
}

Node ReadStateClass::readExprCheck( TokType fnc ) {
	Node t = this->readExpr();
	this->checkToken( fnc );
	return t;
}

Node ReadStateClass::postfixProcessing( Node lhs, Item item, int prec ) {
	Role role = item->role;
	TokType fnc = item->tok_type;
	if ( role.IsBinary() ) {
		const bool direction = tok_type_as_direction( fnc );
		if ( role.IsSys() ) {
			NodeFactory a;
			a.start( SYSAPP );
			a.put( SYSAPP_NAME, tok_type_as_sysapp( fnc ) );
			Node rhs = this->readExprPrec( prec );
			a.add( direction ? lhs : rhs );
			a.add( !direction ? lhs : rhs );
			a.end();
			return a.build();
		} else if ( role.IsForm() ) {
			NodeFactory a;
			a.start( tok_type_as_tag( fnc ) );
			Node rhs = this->readExprPrec( prec );
			a.add( direction ? lhs : rhs );
			a.add( !direction ? lhs : rhs );
			a.end();
			return a.build();			
		} else {
			throw Ginger::Mishap( "Internal error - postfixProcessing" );
		}
	} else if ( role.IsUnary() ) {
		if ( role.IsSys() ) {
			NodeFactory a;
			a.start( SYSAPP );
			a.put( SYSAPP_NAME, tok_type_as_sysapp( fnc ) );
			a.add( lhs );
			a.end();
			return a.build();
		} else if ( role.IsForm() ) {
			NodeFactory a;
			a.start( tok_type_as_tag( fnc ) );
			Node rhs = this->readExprPrec( prec );
			a.add( lhs );
			a.end();
			return a.build();			
		} else {
			throw Ginger::Mishap( "Internal error (postfixProcessing): None of these cases defined yet" );			
		}
	} else {
		switch ( fnc ) {
			case tokty_bindrev:
			case tokty_bind: {
				Node rhs = this->readExprPrec( prec );
				updateAsPattern( fnc == tokty_bind ? lhs : rhs, true );
				NodeFactory bind;
				bind.start( BIND );
				bind.add( fnc == tokty_bind ? lhs : rhs );
				bind.add( fnc != tokty_bind ? lhs : rhs );
				bind.end();
				return bind.build();
			}
			case tokty_from: {				
				updateAsPattern( lhs, true );
				NodeFactory node;
				node.start( FROM );
				Node from_expr = this->readExpr();
				node.add( lhs );
				node.add( from_expr );
				if ( this->tryToken( tokty_to ) ) {
					Node to_expr = this->readExprPrec( prec );
					node.add( to_expr );
				}
				node.end();
				return node.build();
			}
			case tokty_in: {				
				updateAsPattern( lhs, true );
				Node in_expr = this->readExpr();
				NodeFactory node;
				node.start( IN );
				node.add( lhs );
				node.add( in_expr );
				node.end();
				return node.build();
			}
			case tokty_dsemi:
			case tokty_semi: {
				Node rhs = this->readOptExprPrec( prec );
				bool hasnt_rhs = not( rhs );
				if ( fnc == tokty_semi && hasnt_rhs ) {
					return lhs;
				} else {
					NodeFactory s;
					s.start( fnc == tokty_semi ? SEQ : ERASE );
					s.add( lhs );
					if ( not hasnt_rhs ) s.add( rhs );
					s.end();
					return s.build();
				}
			}
			case tokty_obracket: {
				//	Indexing operator.
				Node rhs = this->readStmntsCheck( tokty_cbracket );
				return makeIndex( lhs, rhs );
			}
			case tokty_oparen: {	
				Node rhs = this->readOptEmptyExprCheck( tokty_cparen );
				return makeApp( lhs, rhs );
			}
			case tokty_at:
			case tokty_dot: {
				Node func = this->readExprPrec( prec_tight );
				Node rhs = this->readOptExprPrec( prec );			
				NodeFactory seq;
				seq.start( SEQ );
				seq.add( lhs );
				if ( not not rhs ) { seq.add( rhs ); }
				seq.end();
				return makeApp( func, seq.build() );
			}
			case tokty_double: {
				NodeFactory add;
				//	TODO: Bug??
				add.start( "add" );
				add.add( lhs );
				add.start( CONSTANT );
				add.put( CONSTANT_TYPE, "double" );
				add.put( CONSTANT_VALUE, item->nameString() );
				add.end();
				add.end();
				Node t = add.build();
				//fprintf( stderr, "DEBUG arity %d\n", term_arity( t ) );
				return t;
			}
			case tokty_int: {
				NodeFactory add;
				//	TODO: Bug??
				add.start( "add" );
				add.add( lhs );
				add.start( CONSTANT );
				add.put( CONSTANT_TYPE, "int" );
				add.put( CONSTANT_VALUE, item->nameString() );
				add.end();
				add.end();
				Node t = add.build();
				//fprintf( stderr, "DEBUG arity %d\n", term_arity( t ) );
				return t;
			}
			/*
				case tokty_explode: {
				NodeFactory expl;
				expl.start( SYSAPP );
				expl.put( SYSAPP_NAME, "explode" );
				expl.add( lhs );
				expl.end();
				Node t = expl.build();
				return t;
			}
			*/
			default: {
				throw Ginger::Mishap( "This keyword not handled" ).culprit( "Keyword", item->nameString() );
			}
		}
	}
    throw;	//	Unreachable.
}

static void predicate( TokType sense, NodeFactory & ifunless, Node pred ) {
	if ( sense == tokty_if ) {
		ifunless.add( pred );
	} else {
		//	TODO: This is a bug?
		ifunless.start( "sysval" );
		ifunless.put( "name", "not" );
		ifunless.add( pred );
		ifunless.end();
	}
}

Node ReadStateClass::readIf( TokType sense, TokType closer ) {
	if ( this->cstyle_mode ) this->checkToken( tokty_oparen );
	Node pred = this->readExpr();
	this->checkToken( this->cstyle_mode ? tokty_cparen : tokty_then );
	Node then_part = this->readStmnts();
	if ( this->tryToken( tokty_else ) ) {	
		NodeFactory ifunless;
		ifunless.start( IF );
		predicate( sense, ifunless, pred );
		ifunless.add( then_part );
		Node x = this->readStmnts();
		if ( not this->cstyle_mode ) this->checkToken( closer );
		ifunless.add( x );
		ifunless.end();
		return ifunless.build();
	} else if ( this->cstyle_mode || this->tryToken( closer ) ) {
		NodeFactory ifunless;
		ifunless.start( IF );
		predicate( sense, ifunless, pred );
		ifunless.add( then_part );
		ifunless.end();
		return ifunless.build();
	} else {
		TokType new_sense;
		if ( this->tryToken( tokty_elseif ) ) {
			new_sense = tokty_if;
		} else {
			this->checkToken( tokty_elseunless );
			new_sense = tokty_unless;
		}
		NodeFactory ifunless;
		ifunless.start( IF );
		predicate( sense, ifunless, pred );
		ifunless.add( then_part );
		Node x = this->readIf( new_sense, closer );
		ifunless.add( x );
		ifunless.end();
		return ifunless.build();
	}
}

Node ReadStateClass::readSyscall() {
	ItemFactory ifact = this->item_factory;	
	Item it = ifact->read();
	if ( it->tok_type == tokty_id ) {
		NodeFactory sc;
		sc.start( CONSTANT );
		sc.put( CONSTANT_TYPE, SYSFN );
		sc.put( CONSTANT_VALUE, it->nameString() );
		sc.end();
		return sc.build();
	} else {
		throw Ginger::Mishap( "Invalid token after >-> (syscall) arrow" ).culprit( it->nameString() );
	}
}

Node ReadStateClass::readFor() {
	if ( this->cstyle_mode ) this->checkToken( tokty_oparen );
	Node query = this->readQuery();
	this->checkToken( this->cstyle_mode ? tokty_cparen : tokty_do );
	Node body = this->readStmnts();
	if ( not this->cstyle_mode ) this->checkToken( tokty_endfor );
	NodeFactory for_node;
	for_node.start( FOR );
	for_node.add( query );
	for_node.add( body );
	for_node.end();
	return for_node.build();
}

static void squash( NodeFactory acc, Node rhs ) {
	const std::string 
	name = rhs->name();
	if ( name == SEQ ) {
		int n = rhs->size();
		for ( int i = 0; i < n; i++ ) {
			squash( acc, rhs->child( i ) );
		}
	} else if ( name == VAR ) {
		acc.add( rhs );
	} else {
		throw Ginger::Mishap( "Invalid form for definition" );
	}
}

static void flatten( bool name_needed, Node & ap, Node & fn, Node & args ) {
	if ( ap->name() == APP ) {
		fn = ap->child( 0 );
		Node rhs = ap->child( 1 );
		NodeFactory acc;
		acc.start( SEQ );
		squash( acc, ap->child( 1 ) ); 
		acc.end();
		args = acc.build();
	} else if ( not name_needed ) {
		//fn = NULL;
		NodeFactory acc;
		acc.start( SEQ );
		squash( acc, ap );
		acc.end();
		args = acc.build();
	} else {
		Ginger::Mishap mishap( "Invalid function header" );
		mishap.culprit( "Name", ap->name() );
		if ( ap->name() == SYSAPP ) {
			mishap.culprit( "Reason", "Trying to redefine a system function" );
		}
		throw mishap;
	}
}


static void readImportQualifiers( ReadStateClass & r, bool & pervasive, bool & qualified ) {
	pervasive = true;
	qualified = false;
	for (;;) {
		if ( r.tryName( "pervasive" ) ) {
			pervasive = true;
		} else if ( r.tryName( "nonpervasive" ) ) {
			pervasive = false;
		} else if ( r.tryName( "qualified" ) ) {
			qualified = true;
		} else if ( r.tryName( "unqualified" ) ) {
			qualified = false;
		} else {
			break;
		}
	}
}


static void readTags( ReadStateClass & r, NodeFactory & imp, const char * prefix, const bool add_default ) {
	if ( r.tryToken( tokty_oparen ) ) {
		ItemFactory ifact = r.item_factory;
		if ( ifact->peek()->tok_type != tokty_cparen ) {
			for ( int i = 0; true; i++ ) {
				Item item = ifact->read();
				ostringstream s;
				s << prefix << i;
				imp.put( s.str(), item->nameString() );
				
				item = ifact->read();
				if ( item->tok_type != tokty_comma ) {
					if ( item->tok_type != tokty_cparen ) throw Ginger::Mishap( "Expecting close parenthesis" );
					break;
				}
			}	
		}
	} else if ( add_default ) {
		imp.put( prefix, "public" );
	}
}

static void readImportMatch( ReadStateClass & r, NodeFactory & imp ) {
	readTags( r, imp, "match", true );
}

static void readImportInto( ReadStateClass & r, NodeFactory & imp ) {
	readTags( r, imp, "into", false );
}

static Node makeCharSequence( Item item ) {
	int n = item->nameString().size();
	if ( n == 0 ) {
		return makeEmpty();
	} else {
		NodeFactory charseq;
		charseq.start( SEQ );
		const std::string & s = item->nameString();
		std::string::const_iterator iter = s.begin();
		for ( iter = s.begin(); iter != s.end(); ++iter ) {
			charseq.start( CONSTANT );
			charseq.put( CONSTANT_TYPE, "char" );
			charseq.put( CONSTANT_VALUE, std::string() + *iter );	//	 WRONG
			charseq.end();
		}
		charseq.end();
		return charseq.build();
	}
}

Node ReadStateClass::readAtomicExpr() {
	ItemFactory ifact = this->item_factory;
	Item item = ifact->read();
	TokType fnc = item->tok_type;
	Role role = item->role;
	if ( role.IsLiteral() ) {
		NodeFactory simple;
		simple.start( CONSTANT );
		simple.put( CONSTANT_TYPE, tok_type_as_type( fnc ) );
		simple.put( CONSTANT_VALUE, item->nameString() );
		simple.end();
	 	return simple.build();
	} else if ( fnc == tokty_charseq ) {
		return makeCharSequence( item );
	} else if ( fnc == tokty_oparen ) {
		return this->readExprCheck( tokty_cparen );
	} else {
		throw Ginger::Mishap( "Unexpected token while reading attribute in element" ).culprit( "Token", item->nameString() );
	}
}

Node ReadStateClass::readLambda() {
	#ifdef DBG_COMMON2GNX
		cerr << "LAMBDA" << endl;
	#endif
		
	ReadStateClass pattern( *this );
	pattern.setPatternMode();
	#ifdef DBG_COMMON2GNX
		cerr << "About to read pattern" << endl;
	#endif
	Node ap = pattern.readExprPrec( prec_arrow );
	Node fun;
	Node args;
	flatten( false, ap, fun, args );
	
	#ifdef DBG_COMMON2GNX
		cerr << "Pattern read" << endl;
	#endif

	if ( not this->cstyle_mode ) this->checkToken( tokty_fnarrow );
	Node body = this->readCompoundStmnts( false );
	if ( not this->cstyle_mode ) this->checkToken( tokty_endfn );	
	#ifdef DBG_COMMON2GNX
		cerr << "About to check stuff????? <-----<" << endl;
	#endif
	NodeFactory fn;
	fn.start( FN );
	if ( fun && fun->hasAttribute( "name" ) ) {
		fn.put( FN_NAME, fun->attribute( "name" ) );
	}
	fn.add( args );
	fn.add( body );
	fn.end();
	return fn.build();			
}

Node ReadStateClass::readDefinition() {
	//cerr << "DEFINITION" << endl;
	ReadStateClass pattern( *this );
	pattern.setPatternMode();
	Node ap = pattern.readExprPrec( prec_arrow );
	Node fn;
	Node args;
	flatten( true, ap, fn, args );
	const std::string name( fn->attribute( VID_NAME ) );
	if ( not this->cstyle_mode ) this->checkToken( tokty_fnarrow );
	Node body = this->readCompoundStmnts( false );
	if ( not this->cstyle_mode ) this->checkToken( tokty_enddefine );
	if ( this->cstyle_mode ) {
		//cerr << "postfix NOT allowed (2)" << endl;
		this->is_postfix_allowed = false;
	} else {
		//cerr << "Huh?" << endl;
	}
	NodeFactory def;
	def.start( BIND );
	def.start( VAR );
	def.put( VID_NAME, name );
	def.put( VID_PROTECTED, "true" );
	def.end();
	def.start( FN );
	def.put( FN_NAME, name );
	def.add( args );
	def.add( body );
	def.end();
	def.end();
	return def.build();
}

Node ReadStateClass::readTry( const bool try_vs_transaction ) {
	NodeFactory ftry;
	ftry.start( try_vs_transaction ? "try" : "transaction" );
	Node app = this->readExpr();
	ftry.add( app );

	while ( this->tryToken( tokty_catch ) ) {

		if ( this->tryPeekToken( tokty_oparen ) ) {
			ftry.start( "catch.return" );
		} else {
			ftry.start( "catch.then" );
			ftry.put( "event", this->readIdName() );			
		}		
		
		ReadStateClass pattern( *this );
		pattern.setPatternMode();
		Node catch_patt = pattern.readExpr();
		ftry.add( catch_patt );
		
		this->checkToken( tokty_then );
		Node e = this->readExpr();
		ftry.add( e );

		ftry.end();
	}

	if ( this->tryToken( tokty_else ) ) {
		ftry.start( "catch.else" );
		Node else_stmnts = this->readStmnts();
		ftry.add( else_stmnts );
		ftry.end();
	}

	this->checkToken( try_vs_transaction? tokty_endtry : tokty_endtransaction );
	ftry.end();
	return ftry.build();
}

Node ReadStateClass::readVarVal( TokType fnc ) {
	NodeFactory bind;
	bind.start( BIND );
	
	Node lhs = this->readExprPrec( prec_assign );
	updateAsPattern( lhs, fnc == tokty_val );

	bind.add( lhs );
	this->checkToken( this->cstyle_mode ? tokty_equal : tokty_bind );
	Node x = this->readExpr();
	bind.add( x );
	bind.end();
	return bind.build();
}

Node ReadStateClass::readElement() {
	NodeFactory element;
	element.start( SEQ );
	for (;;) {
		element.start( SYSAPP );
		element.put( SYSAPP_NAME, "newElement" );
		element.start( SYSAPP );
		element.put( SYSAPP_NAME, "newAttrMap" );

		const string element_name( this->readIdName() );
		element.start( CONSTANT );
		element.put( CONSTANT_TYPE, "string" );
		element.put( CONSTANT_VALUE, element_name );
		element.end();
		
		bool closed = false;
		for (;;) {
			if ( this->tryToken( tokty_gt ) ) break;
			if ( this->tryToken( tokty_slashgt ) ) { closed = true; break; }
			element.start( CONSTANT );
			element.put( CONSTANT_TYPE, "string" );
			element.put( CONSTANT_VALUE, this->readIdName() );
			element.end();
			this->checkToken( tokty_equal );
			element.start( "assert" );
			element.put( "n", "1" );
			Node value = this->readAtomicExpr();
			element.add( value );
			element.end();
		}
		element.end(); 	// newAttrMap.
		if ( not closed ) {
			//	Read the children.
			bool ended = false;
			for (;;) {
				if ( this->tryToken( tokty_ltslash ) ) break;
				if ( this->tryToken( tokty_endelement ) ) { ended = true; break; }
				Node child = this->readExpr();
				element.add( child );
			}
			if ( not ended ) {
				const string ename( this->readIdName() );
				if ( ename != element_name ) {
					throw Ginger::Mishap( "Element close tag does not match open tag" ).culprit( "Open tag", element_name ).culprit( "Close tag", ename );
				}
				this->checkToken( tokty_gt );
			}
		}
		element.end();	//	newElement.

		//	Uniquely, at this point in the language, no semi-colon is
		//	needed if the next item is ANOTHER element.
		if ( not this->tryToken( tokty_lt ) ) break;
	}			
	element.end();
	Node answer = element.build();
	return answer->size() == 1 ? answer->child( 0 ) : answer;
}

Node ReadStateClass::prefixProcessing() {
	ItemFactory ifact = this->item_factory;
	const int start = ifact->lineNumber();
	Node node = this->prefixProcessingCore();
	if ( node && this->span_mode ) {
		const int end = ifact->lineNumber();
		stringstream span;
		span << start;
		if ( start != end ) {
			span << ";" << end;
		}
		string spanstr( span.str() );
		string spanspan( SPAN );
		node->putAttribute( spanspan, spanstr );
	}
	return node;
}

Node ReadStateClass::readListOrVector( bool vector_vs_list, TokType closer ) {
	NodeFactory list;
	list.start( vector_vs_list ? VECTOR : LIST );
	Node stmnts = this->readStmnts();
	if ( not vector_vs_list && this->tryToken( tokty_bar ) ) {
		list.add( stmnts );
		list.end();
		Node L = list.build();
		NodeFactory append;
		append.start( LIST_APPEND );
		append.add( L );
		Node x = this->readStmntsCheck( closer );
		append.add( x );
		append.end();
		return append.build();
	} else {
		this->checkToken( closer );
		list.add( stmnts );
		list.end();
		return list.build();
	}
}

Node ReadStateClass::readMap( TokType closer ) {
	NodeFactory list;
	list.start( SYSAPP );
	list.put( SYSAPP_NAME, "newMap" );
	Node x = this->readStmntsCheck( closer );
	list.add( x );
	list.end();
	return list.build();
}

Node ReadStateClass::readSwitchStmnts() {
	NodeFactory st;
	st.start( SEQ );
	while (
		not ( 
			this->tryPeekToken( tokty_cbrace ) ||
			this->tryPeekToken( tokty_case ) ||
			this->tryPeekToken( tokty_default )
		) 
	) {
		Node x = this->readSingleStmnt();
		st.add( x );
	}
	st.end();
	return st.build();
}

Node ReadStateClass::readSwitch() {
	NodeFactory sw;
	sw.start( SWITCH );

	bool else_seen = false;
	Node swelse;

	if ( this->cstyle_mode ) {
		
		this->checkToken( tokty_oparen );
		Node swvalue = this->readExprCheck( tokty_cparen );
		sw.add( swvalue );
	
		this->checkToken( tokty_obrace );
	
		for (;;) {
			if ( this->tryToken( tokty_case ) ) {
				Node e = this->readExpr();
				sw.add( e );
				this->checkToken( tokty_colon );
				Node b = this->readSwitchStmnts();
				sw.add( b );
			} else if ( this->tryToken( tokty_default ) ) {
				if ( else_seen ) {
					throw Ginger::Mishap( "Switch with two default parts" );
				}
				else_seen = true;
				this->checkToken( tokty_colon );
				swelse = this->readSwitchStmnts();
			} else {
				break;
			}
		}
		this->checkToken( tokty_cbrace );

	} else {
		Node swvalue = this->readExpr();
		sw.add( swvalue );

		for (;;) {
			if ( this->tryToken( tokty_case ) ) {
				Node e = this->readExprCheck( tokty_then );
				sw.add( e );
				Node b = this->readStmnts();
				sw.add( b );
			} else if ( this->tryToken( tokty_else ) ) {
				if ( else_seen ) {
					throw Ginger::Mishap( "Switch with two else parts" );
				}
				else_seen = true;
				swelse = this->readOptExpr();
			} else {
				break;
			}
		}
		this->checkToken( tokty_endswitch );
	}

	if ( else_seen ) {
		sw.add( swelse );
	}

	sw.end();
	return sw.build();
}

Node ReadStateClass::readThrow() {
	NodeFactory panic;
	panic.start( "throw" );

	panic.put( "event", this->readIdName() );
	panic.put(
		"level",
		this->tryToken( tokty_bang ) ? "rollback" :
		this->tryToken( tokty_dbang ) ? "failover" :
		this->tryToken( tokty_panic ) ? "panic" :
		"escape"
	);
	if ( this->tryPeekToken( tokty_oparen ) ) {
		Node e = this->readExpr();
		panic.add( e );
	} else {
		panic.start( SEQ );
		panic.end();
	}

	panic.end();
	return panic.build();
}

Node ReadStateClass::readId( const std::string name ) {
	SysConst * sysc = lookupSysConst( name );
	if ( sysc != NULL ) {
		NodeFactory constant;
		constant.start( "constant" );
		constant.put( "type", sysc->tag );
		constant.put( "value", sysc->value );
		constant.end();
		return constant.build();
	} else {
		NodeFactory id;
		id.start( this->pattern_mode ? "var" : "id" );
		id.put( "name", name );
		id.end();
		return id.build();
	}
}

Node ReadStateClass::readEnvVar() {
	this->checkToken( tokty_obrace );
	NodeFactory envvar;
	envvar.start( SYSAPP );
	envvar.put( "name", "sysGetEnv" );
	envvar.start( "constant" );
	envvar.put( "type", "string" );
	envvar.put( "value", this->item_factory->read()->nameString() );
	envvar.end();		
	this->checkToken( tokty_cbrace );
	envvar.end();
	return envvar.build();
}

Node ReadStateClass::readDHat() {
	NodeFactory maplet;
	maplet.start( SYSAPP );
	maplet.put( "name", "newMaplet" );
	const string name( this->readIdName() );
	maplet.start( "constant" );
	maplet.put( "type", "symbol" );
	maplet.put( "value", name );
	maplet.end();
	maplet.start( "id" );
	maplet.put( "name", name );
	maplet.end(); 
	maplet.end();
	return maplet.build();
}

Node ReadStateClass::readPackage() {
	NodeFactory pkg;
	pkg.start( "package" );
	string url = this->readPkgName();
	pkg.put( "url", url );
	this->checkToken( tokty_semi );
	Node body = this->readStmntsCheck( tokty_endpackage );
	pkg.add( body );
	pkg.end();
	return pkg.build();
}

Node ReadStateClass::readImport() {
	NodeFactory imp;
	imp.start( "import" );
	bool pervasive, qualified;
	
	readImportQualifiers( *this, pervasive, qualified );
	imp.put( "pervasive", pervasive ? "true" : "false" );
	imp.put( "qualified", qualified ? "true" : "false" );
	
	readImportMatch( *this, imp );
	
	this->checkToken( tokty_from );
	string url = this->readPkgName();
	imp.put( "from", url );

	if ( this->tryName( "alias" ) ) {
		imp.put( "alias", this->readIdName() );
	}
	
	if ( this->tryName( "into" ) ) {
		readImportInto( *this, imp );
	}
	
	imp.end();
	return imp.build();
}

Node ReadStateClass::readReturn() {
	NodeFactory ret;
	ret.start( "assert" );
	ret.put( "tailcall", "true" );
	Node n = this->readExpr();
	ret.add( n );
	ret.end();
	return ret.build();
}

/*
Transforms 
	recordclass <RNAME>
		slot <NAME1>;
		slot <NAME2>;
		...
		slot <NAME_N>;
	endrecordclass;
into
	<seq>
		<bind>
			<var name="${RNAME}"/>
			<sysapp value="newRecordClass">
				<constant type="string" value="${RNAME}"/>
				<constant type="int" value="${N}"/>
			</sysapp>
		</bind>
		<bind>
			<var name="new${RNAME}"/>
			<sysapp value="classConstructor"/>
				<id name="${RNAME}"/>
			</sysapp>
		</bind>
		<!-- one for each slot -->
		<bind>
			<var name="${NAME_1}"/>
			<sysapp value="classAccessor"/>
				<id name="${RNAME}"/>
				<constant type="int" value="1"/>
			</sysapp>
		</bind>
		....
		<bind>
			<var name="${NAME_N}"/>
			<sysapp value="classAccessor"/>
				<id name="${RNAME}"/>
				<constant type="int" value="${N}"/>
			</sysapp>
		</bind>
	</seq>
*/
Node ReadStateClass::readRecordClass() {
	
	vector< string > slot_names;
	const string class_name( this->readIdName() );
	if ( this->cstyle_mode ) this->checkToken( tokty_obrace );
	while ( this->tryToken( tokty_slot ) ) {
		slot_names.push_back( this->readIdName() );
		this->checkToken( tokty_semi );
	}
	this->checkToken( this->cstyle_mode ? tokty_cbrace : tokty_endrecordclass );

	NodeFactory f;
	f.start( SEQ );

	//	Datakey.
	f.start( BIND );
	f.start( VAR ); 
	f.put( VID_NAME, class_name );
	f.end(); // VAR
	f.start( SYSAPP );
	f.put( SYSAPP_NAME, "newRecordClass" );
	f.start( CONSTANT );
	f.put( CONSTANT_TYPE, "string" );
	f.put( CONSTANT_VALUE, class_name );
	f.end();	//	CONSTANT
	f.start( CONSTANT );
	f.put( CONSTANT_TYPE, "int" );
	f.put( CONSTANT_VALUE, slot_names.size() );	
	f.end(); 	// CONSTANT
	f.end();	//	SYSAPP
	f.end(); // BIND

	//	Constructor
	f.start( BIND );
	f.start( VAR ); 
	{
		stringstream s;
		s << "new";
		s << class_name;
		f.put( VID_NAME, s.str() );
	}
	f.end(); // VAR
	f.start( SYSAPP );
	f.put( SYSAPP_NAME, "newClassConstructor" );
	f.start( ID );
	f.put( VID_NAME, class_name );
	f.end();	//	ID
	f.end();	//	SYSAPP
	f.end(); //	BIND

	//	Recogniser
	f.start( BIND );
	f.start( VAR ); 
	{
		stringstream s;
		s << "is";
		s << class_name;
		f.put( VID_NAME, s.str() );
	}
	f.end(); // VAR
	f.start( SYSAPP );
	f.put( SYSAPP_NAME, "newClassRecogniser" );
	f.start( ID );
	f.put( VID_NAME, class_name );
	f.end();	//	ID
	f.end();	//	SYSAPP
	f.end(); //	BIND

	for ( int i = 0; i < slot_names.size(); i++ ) {
		const string & ith_name( slot_names[ i ] );

		f.start( BIND );
		f.start( VAR ); 
		f.put( VID_NAME, ith_name );
		f.end(); // VAR
		f.start( SYSAPP );
		f.put( SYSAPP_NAME, "newClassAccessor" );
		f.start( ID );
		f.put( VID_NAME, class_name );
		f.end();	//	ID
		f.start( CONSTANT );
		f.put( CONSTANT_TYPE, "int" );
		f.put( CONSTANT_VALUE, i + 1 );	
		f.end(); //	CONSTANT
		f.end();	//	SYSAPP
		f.end(); //	BIND

	}

	f.end(); // SEQ
	return f.build();
}

Node ReadStateClass::prefixProcessingCore() {
	ItemFactory ifact = this->item_factory;
	Item item = ifact->read();
	
	#ifdef DBG_COMMON2GNX
		cerr << "First item was " << tok_type_name( item->tok_type ) << endl;
	#endif
	
	TokType fnc = item->tok_type;
	Role role = item->role;

	//cout << "PREFIX PROCESSING: " << item->nameString() << endl;
	//cout << "  tokty = " << tok_type_name( fnc ) << endl;

	if ( role.IsLiteral() ) {
		NodeFactory simple;
		simple.start( "constant" );
		simple.put( "type", tok_type_as_type( fnc ) );
		simple.put( "value", item->nameString() );
		simple.end();
	 	return simple.build();
	} else if ( role.IsUnary() ) {
		if ( role.IsForm() ) {
			NodeFactory unary;
			unary.start( tok_type_as_tag( fnc ) );
			Node x = this->readExprPrec( item->precedence );
			unary.add( x );
			unary.end();
			return unary.build();
		} else if ( role.IsSys() ) {
			NodeFactory sf;
			sf.start( SYSAPP );
			sf.put( SYSAPP_NAME, tok_type_as_sysapp( fnc ) );
			Node x = this->readExprPrec( item->precedence );
			sf.add( x );
			sf.end();
			return sf.build();
		} else {
			throw Ginger::Mishap( "Internal error - unreachable code reached" );
		}
	}

	switch ( fnc ) {
		case tokty_sub: {
			Node e = this->readExprPrec( prec_negate );
			NodeFactory neg;
			neg.start( SYSAPP );
			neg.put( SYSAPP_NAME, "negate" );
			neg.add( e );
			neg.end();
			return neg.build();
		}
		case tokty_id: 
			return this->readId( item->nameString() );
		// changed for ${VAR} case study
		case tokty_envvar: 
			return this->readEnvVar();
		case tokty_return: 
			return this->readReturn();
		case tokty_throw: 
			return this->readThrow();
		case tokty_try: 
		case tokty_transaction: 
			return this->readTry( fnc == tokty_try );
		case tokty_charseq: 
			return makeCharSequence( item );
		case tokty_val:
        case tokty_var : 
	        return this->readVarVal( fnc );
		case tokty_oparen: {
			if ( this->cstyle_mode ) {
				return this->readOptEmptyExprCheck( tokty_cparen );
			} else {
				return this->readStmntsCheck( tokty_cparen );
			}
		}
		case tokty_obracket:
			return this->readListOrVector( true, tokty_cbracket );
		case tokty_obrace: 
			return this->readMap( tokty_cbrace );
		case tokty_fat_obracket:
			return this->readListOrVector( false, tokty_fat_cbracket );
		case tokty_fat_ocbracket: {
			Node list( new Ginger::Mnx( LIST ) );
			return list;
		}
		case tokty_unless:
			return this->readIf( tokty_unless, tokty_endunless );
		case tokty_if: {
			if ( this->cstyle_mode ) break;
			return this->readIf( tokty_if, tokty_endif );
		}
		case tokty_syscall:
			return this->readSyscall();
		case tokty_for: {
			if ( this->cstyle_mode ) break;
		  	return this->readFor();
		}
		case tokty_function: {
			#ifdef DBG_COMMON2GNX
				cerr << "FUNCTION" << endl;
			#endif
			if ( this->item_factory->peek()->tok_type == tokty_oparen ) {
				return this->readLambda();
			} else {
				break;
			}
		}
		case tokty_define:
			//cerr << "DEFINE" << endl;
			return this->readDefinition();
		case tokty_fn: 
			return this->readLambda();
		case tokty_lt:
			return this->readElement();
		case tokty_package: 
			return this->readPackage();
		case tokty_import: 
			return this->readImport();
		case tokty_dhat:
			return this->readDHat();
		case tokty_switch: {
			if ( this->cstyle_mode ) break;
			return this->readSwitch();
		}
		case tokty_recordclass:
			return this->readRecordClass();
		default: 
			{}

	}
	ifact->unread();
    return Node();
}

bool ReadStateClass::isAtEndOfInput() {
	ItemFactory ifact = this->item_factory;
	Item it = ifact->peek();
	return it->tok_type == tokty_eof;
}

Node ReadStateClass::readOptExprPrec( int prec ) {
	ItemFactory ifact = this->item_factory;
	Node e = this->prefixProcessing();
	if ( not e ) return Node();
	//cerr << "starting postfix checking ... " << this->isPostfixAllowed() << endl;
	while ( this->isPostfixAllowed() ) {
	    int q;
	    //cerr << "peeking" << endl;
		Item it = ifact->peek();
		if ( it->item_is_neg_num() ) {
			NodeFactory t;
			t.start( SYSAPP );
			t.put( "name", "+" );
			t.add( e );			
			if ( it->tok_type == tokty_int ) {
				t.start( "constant" );
				t.put( "type", "int" );
				t.put( "value", it->nameString() );
				t.end();
			} else {
				throw Ginger::Mishap( "Only integers supported so far" ).culprit( "Item", it->nameString() );
			}
			t.end();
			e = t.build();
			ifact->drop();
		} else if ( it->item_is_postfix() ) {
	        q = it->precedence;
            if ( q >= prec ) break;
	        ifact->drop();
	        e = this->postfixProcessing( e, it, q );
		} else {
			break;
		}
    }
    return e;
}

