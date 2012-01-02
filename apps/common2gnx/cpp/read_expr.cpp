#include <stdexcept>
#include <string>
#include <sstream>

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


using namespace std;

typedef Ginger::MnxBuilder NodeFactory;

static Node makeApp( Node lhs, Node rhs ) {
	if ( lhs->name() == "sysfn" ) {
		NodeFactory sysapp;
		sysapp.start( "sysapp" );
		std::string name = lhs->attribute( "value" );
		sysapp.put( "name", name );
		sysapp.add( rhs );
		sysapp.end();
		return sysapp.build();
	} else {
		NodeFactory app;
		app.start( "app" );
		app.add( lhs );
		app.add( rhs );
		app.end();
		return app.build();
	}
}

static Node makeIndex( Node lhs, Node rhs ) {
	NodeFactory index;
	index.start( "sysapp" );
	index.put( "name", "index" );
	index.add( lhs );
	index.add( rhs );
	index.end();
	return index.build();
}

/*static void pushAbsent( NodeFactory & f ) {
	f.start( "constant" );
	f.put( "type", "absent" );
	f.put( "value", "absent" );
	f.end();
}

static void pushEmpty( NodeFactory & f ) {
	f.start( "seq" );
	f.end();
}*/

static void pushStringConstant( NodeFactory & f, const string & s ) {
	f.start( "constant" );
	f.put( "type", "string" );
	f.put( "value", s );
	f.end();
}

static Node makeEmpty() {
	return shared< Ginger::Mnx >( new Ginger::Mnx( "seq" ) );
}

static void pushAnyPattern( NodeFactory & f ) {
	f.start( "val" );
	f.end();
}

static void updateAsPattern( Node node ) {
	if ( node->hasName( "id" ) ) {
		node->name() = "var";
		node->putAttribute( "protected", "true" );
	}
}


string ReadStateClass::readPkgName() {
	Item it = this->item_factory->read();
	return it->nameString();
}

Item ReadStateClass::readIdItem() {
	Item it = this->item_factory->read();
	if ( it->tok_type != tokty_id ) {
		throw Ginger::Mishap( "Identifier expected" ).culprit( "Found", it->nameString() );
	}
	return it;
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

void ReadStateClass::checkSemi() {
	this->checkToken( tokty_semi );
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
		stmnts.start( "seq" );
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
			case tokty_obrace: return this->readCompoundStmnts( true );
			case tokty_function: return this->readDefinition();
			case tokty_if: return this->readIf( tokty_if, tokty_endif );
			case tokty_for: return this->readFor();
			case tokty_semi: return makeEmpty();
			default: ;
		}
	}
	
	//	Fall thru!
	Node n = this->readOptEmptyExpr();
	this->checkToken( tokty_semi );
	return n;
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
			a.start( "sysapp" );
			a.put( "name", tok_type_as_sysapp( fnc ) );
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
			throw;
		}
	} else {
		switch ( fnc ) {
			case tokty_bindrev:
			case tokty_bind: {
				Node rhs = this->readExprPrec( prec );
				updateAsPattern( fnc == tokty_bind ? lhs : rhs );
				NodeFactory bind;
				bind.start( "bind" );
				bind.add( fnc == tokty_bind ? lhs : rhs );
				bind.add( fnc != tokty_bind ? lhs : rhs );
				bind.end();
				return bind.build();
			}
			case tokty_from: {				
				updateAsPattern( lhs );
				NodeFactory node;
				node.start( "from" );
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
				updateAsPattern( lhs );
				Node in_expr = this->readExpr();
				NodeFactory node;
				node.start( "in" );
				node.add( lhs );
				node.add( in_expr );
				node.end();
				return node.build();
			}
			case tokty_semi: {
				Node rhs = this->readOptExprPrec( prec );
				if ( not( rhs ) ) {
					return lhs;
				} else {
					NodeFactory s;
					s.start( "seq" );
					s.add( lhs );
					s.add( rhs );
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
				seq.start( "seq" );
				seq.add( lhs );
				if ( not not rhs ) { seq.add( rhs ); }
				seq.end();
				return makeApp( func, seq.build() );
			}
			case tokty_int: {
				NodeFactory add;
				add.start( "add" );
				add.add( lhs );
				add.start( "constant" );
				add.put( "type", "int" );
				add.put( "value", item->nameString() );
				add.end();
				add.end();
				Node t = add.build();
				//fprintf( stderr, "DEBUG arity %d\n", term_arity( t ) );
				return t;
			}
			case tokty_explode: {
				NodeFactory expl;
				expl.start( "sysapp" );
				expl.put( "name", "explode" );
				expl.add( lhs );
				expl.end();
				Node t = expl.build();
				return t;
			}
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
		ifunless.start( "if" );
		predicate( sense, ifunless, pred );
		ifunless.add( then_part );
		Node x = this->readStmnts();
		if ( not this->cstyle_mode ) this->checkToken( closer );
		ifunless.add( x );
		ifunless.end();
		return ifunless.build();
	} else if ( this->cstyle_mode || this->tryToken( closer ) ) {
		NodeFactory ifunless;
		ifunless.start( "if" );
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
		ifunless.start( "if" );
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
		sc.start( "constant" );
		sc.put( "type", "sysfn" );
		sc.put( "value", it->nameString() );
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
	for_node.start( "for" );
	for_node.add( query );
	for_node.add( body );
	for_node.end();
	return for_node.build();
}

static void squash( NodeFactory acc, Node rhs ) {
	const std::string 
	name = rhs->name();
	if ( name == "seq" ) {
		int n = rhs->size();
		for ( int i = 0; i < n; i++ ) {
			squash( acc, rhs->child( i ) );
		}
	} else if ( name == "var" ) {
		acc.add( rhs );
	} else {
		throw Ginger::Mishap( "Invalid form for definition" );
	}
}

static void flatten( Node & ap, Node & fn, Node & args ) {
	if ( ap->name() == "app" ) {
		fn = ap->child( 0 );
		Node rhs = ap->child( 1 );
		NodeFactory acc;
		acc.start( "seq" );
		squash( acc, ap->child( 1 ) ); 
		acc.end();
		args = acc.build();
	} else {
		Ginger::Mishap mishap( "Invalid use of 'define'" );
		mishap.culprit( "Name", ap->name() );
		if ( ap->name() == "sysapp" ) {
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
		charseq.start( "seq" );
		const std::string & s = item->nameString();
		std::string::const_iterator iter = s.begin();
		for ( iter = s.begin(); iter != s.end(); ++iter ) {
			charseq.start( "constant" );
			charseq.put( "type", "char" );
			charseq.put( "value", std::string() + *iter );	//	 WRONG
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
		simple.start( "constant" );
		simple.put( "type", tok_type_as_type( fnc ) );
		simple.put( "value", item->nameString() );
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
	//cerr << "LAMBDA" << endl;
	ReadStateClass pattern( *this );
	pattern.setPatternMode();
	//cerr << "About to read pattern" << endl;
	Node args = pattern.readExprPrec( prec_arrow );
	//cerr << "Pattern read" << endl;
	if ( not this->cstyle_mode ) this->checkToken( tokty_fnarrow );
	Node body = this->readCompoundStmnts( false );
	if ( not this->cstyle_mode ) this->checkToken( tokty_endfn );
	//cerr << "About to check stuff????? <-----<" << endl;
	NodeFactory fn;
	fn.start( "fn" );
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
	flatten( ap, fn, args );
	const std::string name = fn->attribute( "name" );
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
	def.start( "bind" );
	def.start( "var" );
	def.put( "name", name );
	def.put( "protected", "true" );
	def.end();
	def.start( "fn" );
	def.put( "name", name );
	def.add( args );
	def.add( body );
	def.end();
	def.end();
	return def.build();
}

void ReadStateClass::readTryCatch( NodeFactory & ftry ) {
	while( this->tryToken( tokty_case ) ) {
		ftry.start( "case" );
		ReadStateClass pattern( *this );
		pattern.setPatternMode();
		Node case_patt = pattern.readExpr();
		ftry.add( case_patt );
		this->checkToken( tokty_then );
		Node then_stnmnts = this->readStmnts();
		ftry.add( then_stnmnts );
		ftry.end();
	}
	if ( this->tryToken( tokty_else ) ) {
		ftry.start( "case" );
		pushAnyPattern( ftry );
		Node else_stmnts = this->readStmnts();
		ftry.add( else_stmnts );
		ftry.end();
	}
}

Node ReadStateClass::readTry( const bool try_vs_transaction ) {
	// 	try APP ( catch ( case PATTERN then STMNTS )* [ else STMNTS ] )* endtry
	//	<try> APP ( <catch> ... </catch> )* </try>
	//	<catch> PATTERN ( <case> PATTERN STMNTS </case> )* </catch>
	NodeFactory ftry;
	ftry.start( try_vs_transaction ?"try" : "transaction" );
	Node app = this->readExpr();
	ftry.add( app );
	if ( try_vs_transaction && this->tryPeekToken( tokty_case ) ) {
		ftry.start( "catch" );
		pushAnyPattern( ftry ); 
		this->readTryCatch( ftry );
		ftry.end();
	} else if ( try_vs_transaction && this->tryToken( tokty_else ) ) {
		ftry.start( "catch" );
		pushAnyPattern( ftry );
		ftry.start( "case" );
		pushAnyPattern( ftry );
		Node stmnts = this->readStmnts();
		ftry.add( stmnts );
		ftry.end();
		ftry.end();
	} else if ( this->tryPeekToken( tokty_catch ) ) {
		while ( this->tryToken( tokty_catch ) ) {
			ftry.start( "catch" );
			ReadStateClass pattern( *this );
			pattern.setPatternMode();
			Node catch_patt = pattern.readExpr();
			ftry.add( catch_patt );
			if ( try_vs_transaction ) {
				this->readTryCatch( ftry );
			} else {
				this->checkToken( tokty_then );
				Node stmnts = this->readStmnts();
				ftry.add( stmnts );
			}
			ftry.end();
		}
	} 
	this->checkToken( try_vs_transaction? tokty_endtry : tokty_endtransaction );
	ftry.end();
	return ftry.build();
}

Node ReadStateClass::readVarVal( TokType fnc ) {
	NodeFactory bind;
	bind.start( "bind" );
	NodeFactory var;
	var.start( "var" );
	readTags( *this, var, "tag", true );
	Item item = this->readIdItem();
	var.put( "name", item->nameString() );
	var.put( "protected", fnc == tokty_val ? "true" : "false" );
	var.end();
	Node v = var.build();
	bind.add( v );
	this->checkToken( tokty_bind );
	Node x = this->readExpr();
	bind.add( x );
	bind.end();
	return bind.build();
}

Node ReadStateClass::prefixProcessing() {
	ItemFactory ifact = this->item_factory;
	Item item = ifact->read();
	
	//cerr << "First item was " << tok_type_name( item->tok_type ) << endl;
	
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
			sf.start( "constant" );
			sf.put( "type", "sysfn" );
			sf.put( "value", tok_type_as_sysapp( fnc ) );
			sf.end();
			return sf.build();
		} else {
			throw;	// 	Unreachable.
		}
	}

	switch ( fnc ) {
		case tokty_id: {
			std::string & name = item->nameString();
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
		// changed for ${VAR} case study
		case tokty_envvar: {
			this->checkToken( tokty_obrace );
			NodeFactory envvar;
			envvar.start( "sysapp" );
			envvar.put( "name", "sysGetEnv" );
			envvar.start( "constant" );
			envvar.put( "type", "string" );
			envvar.put( "value", ifact->read()->nameString() );
			envvar.end();		
			this->checkToken( tokty_cbrace );
			envvar.end();
			return envvar.build();
		}
		case tokty_return: {
			NodeFactory ret;
			ret.start( "assert" );
			ret.put( "tailcall", "true" );
			Node n = this->readExpr();
			ret.add( n );
			ret.end();
			return ret.build();
		}
		case tokty_throw:
		case tokty_escape:
		case tokty_rollback:
		case tokty_failover:
		case tokty_panic: {
			NodeFactory panic;
			panic.start( "throw" );
			Item item = readIdItem();
			if ( this->tryToken( tokty_oparen ) ) {
				panic.start( "sysapp" );
				panic.put( "name", "withExceptionDetails" );
				
				panic.start( "sysapp" );
				panic.put( "name", "newException" );
				pushStringConstant( panic, item->nameString() );
				panic.end();
				
				Node d = this->readExprCheck( tokty_cparen );
				panic.add( d );
				
				panic.end();
			} else {
				panic.start( "sysapp" );
				panic.put( "name", "newException" );
				pushStringConstant( panic, item->nameString() );
				panic.end();
			}
			panic.end();
			return panic.build();
		}
		/*case tokty_transaction: {
			return this->readTry( false );
		}
		case tokty_try: {
			return this->readTry( true );
		}*/
		case tokty_charseq: {
			return makeCharSequence( item );
		}
		case tokty_val:
        case tokty_var : {
	        return this->readVarVal( fnc );
        }
		case tokty_oparen: {
			if ( this->cstyle_mode ) {
				return this->readOptEmptyExprCheck( tokty_cparen );
			} else {
				return this->readStmntsCheck( tokty_cparen );
			}
		}
		case tokty_obracket: {
			NodeFactory list;
			list.start( this->cstyle_mode ? VECTOR : LIST );
			Node stmnts = this->readStmnts();
			if ( not this->cstyle_mode && this->tryToken( tokty_bar ) ) {
				list.add( stmnts );
				list.end();
				Node L = list.build();
				NodeFactory append;
				append.start( LIST_APPEND );
				append.add( L );
				Node x = this->readStmntsCheck( tokty_cbracket );
				append.add( x );
				append.end();
				return append.build();
			} else {
				this->checkToken( tokty_cbracket );
				list.add( stmnts );
				list.end();
				return list.build();
			}
		}
		case tokty_obrace: {
			NodeFactory list;
			list.start( VECTOR );
			Node x = this->readStmntsCheck( tokty_cbrace );
			list.add( x );
			list.end();
			return list.build();
		}
		case tokty_fat_obrace: {
			NodeFactory list;
			list.start( SYSAPP );
			list.put( SYSAPP_NAME, "newMap" );
			Node x = this->readStmntsCheck( tokty_fat_cbrace );
			list.add( x );
			list.end();
			return list.build();
		}
		case tokty_unless: {
			return this->readIf( tokty_unless, tokty_endunless );
		}
		case tokty_if: {
			if ( this->cstyle_mode ) break;
			return this->readIf( tokty_if, tokty_endif );
		}
		case tokty_syscall: {
			return this->readSyscall();
		}
		case tokty_for: {
			if ( this->cstyle_mode ) break;
		  	return this->readFor();
		}
		case tokty_function: {
			//cerr << "FUNCTION" << endl;
			if ( this->item_factory->peek()->tok_type == tokty_oparen ) {
				return this->readLambda();
			} else {
				break;
			}
		}
		case tokty_define: {
			//cerr << "DEFINE" << endl;
			return this->readDefinition();
		}
		case tokty_fn: {
			return this->readLambda();
		}
		case tokty_lt: {
			NodeFactory element;
			element.start( SEQ );
			for (;;) {
				element.start( "sysapp" );
				element.put( "name", "newElement" );
				element.start( "sysapp" );
				element.put( "name", "newAttrMap" );
				
				Item item = this->readIdItem();
				string element_name( item->nameString() );
				element.start( "constant" );
				element.put( "type", "string" );
				element.put( "value", element_name );
				element.end();
				
				bool closed = false;
				for (;;) {
					if ( this->tryToken( tokty_gt ) ) break;
					if ( this->tryToken( tokty_slashgt ) ) { closed = true; break; }
					Item item = this->readIdItem();
					element.start( "constant" );
					element.put( "type", "string" );
					element.put( "value", item->nameString() );
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
					while ( not this->tryToken( tokty_ltslash ) ) {
						Node child = this->readExpr();
						element.add( child );
					}
					Item item = this->readIdItem();
					if ( item->nameString() != element_name ) {
						throw Ginger::Mishap( "Element close tag does not match open tag" ).culprit( "Open tag", element_name ).culprit( "Close tag", item->nameString() );
					}
					this->checkToken( tokty_gt );
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
		case tokty_package: {
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
		case tokty_import: {
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
	        	Item item = this->readIdItem();
				imp.put( "alias", item->nameString() );
			}
			
			if ( this->tryName( "into" ) ) {
				readImportInto( *this, imp );
			}
			
			imp.end();
			return imp.build();
		}
		default: {
		}
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
			t.start( "sysapp" );
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

