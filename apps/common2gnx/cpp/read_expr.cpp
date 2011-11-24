#include <stdexcept>
#include <string>
#include <sstream>

#include <stdio.h>
#include <assert.h>

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

static void pushAbsent( NodeFactory & f ) {
	f.start( "constant" );
	f.put( "type", "absent" );
	f.put( "value", "absent" );
	f.end();
}

static void pushEmpty( NodeFactory & f ) {
	f.start( "seq" );
	f.end();
}

static void updateAsPattern( Node node ) {
	if ( node->hasName( "id" ) ) {
		node->name() = "var";
		node->putAttribute( "protected", "true" );
	}
}

Node ReadStateClass::read_id() {
	Item it = this->item_factory->read();
	if ( it->tok_type != tokty_id ) {
		throw Ginger::Mishap( "Identifier expected" ).culprit( "Found", it->nameString() );
	}
	NodeFactory id;
	id.start( "id" );
	id.put( "name", it->nameString() );
	id.end();
	return id.build();
}

string ReadStateClass::read_pkg_name() {
	Item it = this->item_factory->read();
	return it->nameString();
}

Item ReadStateClass::read_id_item() {
	Item it = this->item_factory->read();
	if ( it->tok_type != tokty_id ) {
		throw Ginger::Mishap( "Identifier expected" ).culprit( "Found", it->nameString() );
	}
	return it;
}

Node ReadStateClass::read_expr_prec( int prec ) {
	Node e = this->read_opt_expr_prec( prec );
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

void ReadStateClass::check_token( TokType fnc ) {
	Item it = this->item_factory->read();
	if ( it->tok_type != fnc ) {
		throw Ginger::Mishap( "Unexpected token" ).culprit( "Found", it->nameString() ).culprit( "Expected", tok_type_name( fnc ) );
	}
}

void ReadStateClass::check_peek_token( TokType fnc ) {
	Item it = this->item_factory->peek();
	if ( it->tok_type != fnc ) {
		throw Ginger::Mishap( "Unexpected token" ).culprit( "Found", it->nameString() ).culprit( "Expected", tok_type_name( fnc ) );
	}
}

void ReadStateClass::checkSemi() {
	this->check_token( tokty_semi );
}

bool ReadStateClass::try_name( const char * name ) {
	ItemFactory ifact = this->item_factory;
	Item it = ifact->peek();
	if ( it->nameString() == name ) {
		ifact->drop();
		return true;
	} else {
		return false;
	}
}

bool ReadStateClass::try_token( TokType fnc ) {
	ItemFactory ifact = this->item_factory;
	if ( ifact->peek()->tok_type == fnc ) {
		ifact->drop();
		return true;
	} else {
		return false;
	}
}

Node ReadStateClass::read_stmnts() {
	Node e = this->read_opt_expr_prec( this->cstyle_mode ? prec_semi : prec_max );
	if ( not e ) {
		NodeFactory skip;
		pushEmpty( skip );
		return skip.build();
	} else {
		return e;
	}
}

Node ReadStateClass::read_stmnts_check( TokType fnc ) {
	Node t = this->read_stmnts();
	this->check_token( fnc );
	return t;
}
	
Node ReadStateClass::read_expr() {
	return this->read_expr_prec( prec_semi );
}

//	1st approximation. It is too permissive as it stands.
Node ReadStateClass::read_query() {
	return this->read_expr();
}

Node ReadStateClass::read_opt_expr() {
	return this->read_opt_expr_prec( prec_semi );
}

Node ReadStateClass::read_expr_check( TokType fnc ) {
	Node t = this->read_expr();
	this->check_token( fnc );
	return t;
}

Node ReadStateClass::postfix_processing( Node lhs, Item item, int prec ) {
	Role role = item->role;
	TokType fnc = item->tok_type;
	if ( role.IsPattern() ) {
		updateAsPattern( lhs );
		switch ( fnc ) {
			case tokty_bind: {
				Node rhs = this->read_expr_prec( prec );
				NodeFactory bind;
				bind.start( "bind" );
				bind.add( lhs );
				bind.add( rhs );
				bind.end();
				return bind.build();
			}
			case tokty_from: {				
				Node from_expr = this->read_expr();
				this->check_token( tokty_to );
				Node to_expr = this->read_expr_prec( prec );
				NodeFactory node;
				node.start( "from" );
				node.add( lhs );
				node.add( from_expr );
				node.add( to_expr );
				node.end();
				return node.build();
			}
			case tokty_in: {				
				Node in_expr = this->read_expr();
				NodeFactory node;
				node.start( "in" );
				node.add( lhs );
				node.add( in_expr );
				node.end();
				return node.build();
			}
			default: 
				throw;
		}
	} else if ( role.IsBinary() ) {
		if ( role.IsSys() ) {
			NodeFactory a;
			a.start( "sysapp" );
			a.put( "name", tok_type_as_sysapp( fnc ) );
			a.add( lhs );
			Node x = this->read_expr_prec( prec );
			a.add( x );
			a.end();
			return a.build();
		} else if ( role.IsForm() ) {
			NodeFactory a;
			a.start( tok_type_as_tag( fnc ) );
			a.add( lhs );
			Node x = this->read_expr_prec( prec );
			a.add( x );
			a.end();
			return a.build();			
		} else {
			throw;
		}
	} else {
		switch ( fnc ) {
			case tokty_bindrev: {
				ReadStateClass pattern( *this );
				pattern.setPatternMode();
				Node rhs = pattern.read_expr_prec( prec );
				NodeFactory bind;
				bind.start( "bind" );
				//	N.B. LHS & RHS swapped!
				bind.add( rhs );
				bind.add( lhs );
				bind.end();
				return bind.build();
			}
			case tokty_semi: {
				Node rhs = this->read_opt_expr_prec( prec );
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
			case tokty_oparen: {
				Node rhs = this->read_stmnts_check( tokty_cparen );
				return makeApp( lhs, rhs );
			}
			case tokty_at:
			case tokty_dot: {
				Node func = this->read_expr_prec( prec_tight );
				Node rhs = this->read_opt_expr_prec( prec );			
				NodeFactory seq;
				seq.start( "seq" );
				seq.add( lhs );
				if ( ! not rhs ) { seq.add( rhs ); }
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

Node ReadStateClass::read_if( TokType sense, TokType closer ) {
	if ( this->cstyle_mode ) this->check_token( tokty_oparen );
	Node pred = this->read_expr();
	this->check_token( this->cstyle_mode ? tokty_cparen : tokty_then );
	Node then_part = this->read_stmnts();
	if ( this->try_token( tokty_else ) ) {	
		NodeFactory ifunless;
		ifunless.start( "if" );
		predicate( sense, ifunless, pred );
		ifunless.add( then_part );
		Node x = this->read_stmnts();
		if ( not this->cstyle_mode ) this->check_token( closer );
		ifunless.add( x );
		ifunless.end();
		return ifunless.build();
	} else if ( this->cstyle_mode || this->try_token( closer ) ) {
		NodeFactory ifunless;
		ifunless.start( "if" );
		predicate( sense, ifunless, pred );
		ifunless.add( then_part );
		ifunless.end();
		return ifunless.build();
	} else {
		TokType new_sense;
		if ( this->try_token( tokty_elseif ) ) {
			new_sense = tokty_if;
		} else {
			this->check_token( tokty_elseunless );
			new_sense = tokty_unless;
		}
		NodeFactory ifunless;
		ifunless.start( "if" );
		predicate( sense, ifunless, pred );
		ifunless.add( then_part );
		Node x = this->read_if( new_sense, closer );
		ifunless.add( x );
		ifunless.end();
		return ifunless.build();
	}
}

Node ReadStateClass::read_syscall() {
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

Node ReadStateClass::read_for() {
	if ( this->cstyle_mode ) this->check_token( tokty_oparen );
	Node query = this->read_query();
	this->check_token( this->cstyle_mode ? tokty_cparen : tokty_do );
	Node body = this->read_stmnts();
	if ( not this->cstyle_mode ) this->check_token( tokty_endfor );
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
		if ( r.try_name( "pervasive" ) ) {
			pervasive = true;
		} else if ( r.try_name( "nonpervasive" ) ) {
			pervasive = false;
		} else if ( r.try_name( "qualified" ) ) {
			qualified = true;
		} else if ( r.try_name( "unqualified" ) ) {
			qualified = false;
		} else {
			break;
		}
	}
}


static void readTags( ReadStateClass & r, NodeFactory & imp, const char * prefix, const bool add_default ) {
	if ( r.try_token( tokty_oparen ) ) {
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
		NodeFactory skip;
		pushEmpty( skip );
		return skip.build();
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

Node ReadStateClass::read_atomic_expr() {
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
		return this->read_expr_check( tokty_cparen );
	} else {
		throw Ginger::Mishap( "Unexpected token while reading attribute in element" ).culprit( "Token", item->nameString() );
	}
}

Node ReadStateClass::read_lambda() {
	ReadStateClass pattern( *this );
	pattern.setPatternMode();
	Node args = pattern.read_expr_prec( prec_arrow );
	if ( not this->cstyle_mode ) this->check_token( tokty_arrow );
	Node body = this->read_stmnts_check( tokty_endfn );
	NodeFactory fn;
	fn.start( "fn" );
	fn.add( args );
	fn.add( body );
	fn.end();
	return fn.build();			
}

Node ReadStateClass::read_definition() {
	ReadStateClass pattern( *this );
	pattern.setPatternMode();
	Node ap = pattern.read_expr_prec( prec_arrow );
	Node fn;
	Node args;
	flatten( ap, fn, args );
	const std::string name = fn->attribute( "name" );
	if ( this->cstyle_mode ) {
		this->check_peek_token( tokty_obrace );
	} else {
		this->check_token( tokty_arrow );
	}
	Node body = this->read_stmnts();
	if ( not this->cstyle_mode ) this->check_token( tokty_enddefine );
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

Node ReadStateClass::prefix_processing() {
	ItemFactory ifact = this->item_factory;
	Item item = ifact->read();
	
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
			Node x = this->read_expr_prec( item->precedence );
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
			this->check_token( tokty_obrace );
			NodeFactory envvar;
			envvar.start( "sysapp" );
			envvar.put( "name", "sysGetEnv" );
			envvar.start( "constant" );
			envvar.put( "type", "string" );
			envvar.put( "value", ifact->read()->nameString() );
			envvar.end();		
			this->check_token( tokty_cbrace );
			envvar.end();
			return envvar.build();
		}
		case tokty_throw: {
			//	throw [ TAG_EXPR ! ] [ with EXPR ];
			//	return [ \[ Missing \] ] [ { EXPR } ];	// C-style
			NodeFactory thr;
			thr.start( "returnthrow" );
			if ( this->cstyle_mode ) {
				if ( this->try_token( tokty_obracket ) ) {
					Node e1 = this->read_expr_check( tokty_cbracket );
					thr.add( e1 );
				} else {
					pushAbsent( thr );
				}
				if ( this->try_token( tokty_obrace ) ) {
					this->read_stmnts_check( tokty_cbrace );
				} else {
					pushEmpty( thr );
				}
			} else {
				Node e1 = this->read_opt_expr();
				if ( not e1 ) {
					pushAbsent( thr );
					pushAbsent( thr );
				} else {
					thr.add( e1 );
					pushAbsent( thr );
				} 
				if ( this->try_token( tokty_with ) ) {
					Node e2 = this->read_expr();
					thr.add( e2 );
				} else {
					pushEmpty( thr );
				}
			}
			thr.end();
			return thr.build();
		}
		case tokty_return: {
			//	return [ with EXPR ];
			//	return TAG_EXPR ! [ with EXPR ];
			//	return [ TAG_EXPR ! ] EXPR [ with EXPR ];
			//	return [ \[ Missing \] ] [ EXPR ] [ { EXPR } ];	// C-style
			NodeFactory ret;
			ret.start( "returnthrow" );
			if ( this->cstyle_mode ) {
				if ( this->try_token( tokty_obracket ) ) {
					Node e1 = this->read_expr_check( tokty_cbracket );
					ret.add( e1 );
				} else {
					pushAbsent( ret );
				}
				Node e = this->read_opt_expr();
				if ( not e ) {
					pushEmpty( ret );
				} else {
					ret.add( e );
				}
				if ( this->try_token( tokty_obrace ) ) {
					this->read_stmnts_check( tokty_cbrace );
				} else {
					pushEmpty( ret );
				}
			} else {
				Node e1 = this->read_opt_expr();
				if ( not e1 ) {
					pushAbsent( ret );
					pushEmpty( ret );
				} else {
					if ( this->try_token( tokty_tag ) ) {
						ret.add( e1 );
						Node e2 = this->read_opt_expr();
						if ( not e2 ) {
							pushEmpty( ret );
						} else {
							ret.add( e2 );
						}
					} else {
						pushAbsent( ret );
						ret.add( e1 );
					}
				}
				if ( this->try_token( tokty_with ) ) {
					Node e3 = this->read_expr();
					ret.add( e3 );
				} else {
					pushEmpty( ret );
				}
			}
			ret.end();
			return ret.build();
		}
		case tokty_charseq: {
			return makeCharSequence( item );
		}
		case tokty_val:
        case tokty_var : {
	        NodeFactory bind;
	        bind.start( "bind" );
	        NodeFactory var;
	        var.start( "var" );
	        readTags( *this, var, "tag", true );
	        Item item = this->read_id_item();
	        var.put( "name", item->nameString() );
	        if ( fnc == tokty_val ) {
	        	var.put( "protected", "true" );
	        }
	        var.end();
	        Node v = var.build();
	        bind.add( v );
	        this->check_token( tokty_bind );
	        Node x = this->read_expr();
	        bind.add( x );
	        bind.end();
	        return bind.build();
        }
		case tokty_oparen: {
			return this->read_stmnts_check( tokty_cparen );
		}
		case tokty_obracket: {
			NodeFactory list;
			list.start( "sysapp" );
			Node stmnts = this->read_stmnts();
			if ( not this->cstyle_mode && this->try_token( tokty_bar ) ) {
				list.put( "name", "newListOnto" );
				list.add( stmnts );
				Node x = this->read_stmnts_check( tokty_cbracket );
				list.add( x );
			} else {
				this->check_token( tokty_cbracket );
				list.put( "name", this->cstyle_mode ? "newVector" : "newList" );
				list.add( stmnts );
			}
			list.end();
			return list.build();
		}
		case tokty_obrace: {
			if ( cstyle_mode ) {
				NodeFactory seq;
				seq.start( "block" );
				while ( not this->try_token( tokty_cbrace ) ) {					
					Node x = this->read_stmnts();
					seq.add( x );
					this->try_token( tokty_semi );	//	optional semi.
				}
				seq.end();
				return seq.build();
			} else {
				NodeFactory list;
				list.start( "vector" );
				Node x = this->read_stmnts_check( tokty_cbrace );
				list.add( x );
				list.end();
				return list.build();
			}
		}
		case tokty_fat_obrace: {
			NodeFactory list;
			list.start( "sysapp" );
			list.put( "name", "newMap" );
			Node x = this->read_stmnts_check( tokty_fat_cbrace );
			list.add( x );
			list.end();
			return list.build();
		}
		case tokty_unless: {
			return this->read_if( tokty_unless, tokty_endunless );
		}
		case tokty_if: {
			return this->read_if( tokty_if, tokty_endif );
		}
		case tokty_syscall: {
			return this->read_syscall();
		}
		case tokty_for: {
		  	return this->read_for();
		}
		case tokty_function: {
			if ( this->item_factory->peek()->tok_type == tokty_oparen ) {
				return this->read_lambda();
			} else {
				return this->read_definition();
			}
		}
		case tokty_define: {
			return this->read_definition();
		}
		case tokty_fn: {
			return this->read_lambda();
		}
		case tokty_lt: {
			NodeFactory element;
			element.start( "seq" );
			for (;;) {
				element.start( "sysapp" );
				element.put( "name", "newElement" );
				element.start( "sysapp" );
				element.put( "name", "newAttrMap" );
				
				Item item = this->read_id_item();
				string element_name( item->nameString() );
				element.start( "constant" );
				element.put( "type", "string" );
				element.put( "value", element_name );
				element.end();
				
				bool closed = false;
				for (;;) {
					if ( this->try_token( tokty_gt ) ) break;
					if ( this->try_token( tokty_slashgt ) ) { closed = true; break; }
					Item item = this->read_id_item();
					element.start( "constant" );
					element.put( "type", "string" );
					element.put( "value", item->nameString() );
					element.end();
					this->check_token( tokty_equal );
					element.start( "assert" );
					element.put( "n", "1" );
					Node value = this->read_atomic_expr();
					element.add( value );
					element.end();
				}
				element.end(); 	// newAttrMap.
				if ( not closed ) {
					//	Read the children.
					while ( not this->try_token( tokty_ltslash ) ) {
						Node child = this->read_expr();
						element.add( child );
					}
					Item item = this->read_id_item();
					if ( item->nameString() != element_name ) {
						throw Ginger::Mishap( "Element close tag does not match open tag" ).culprit( "Open tag", element_name ).culprit( "Close tag", item->nameString() );
					}
					this->check_token( tokty_gt );
				}
				element.end();	//	newElement.

				//	Uniquely, at this point in the language, no semi-colon is
				//	needed if the next item is ANOTHER element.
				if ( not this->try_token( tokty_lt ) ) break;
			}			
			element.end();
			Node answer = element.build();
			return answer->size() == 1 ? answer->child( 0 ) : answer;
		}
		case tokty_package: {
			NodeFactory pkg;
			pkg.start( "package" );
			string url = this->read_pkg_name();
			pkg.put( "url", url );
			this->check_token( tokty_semi );
			Node body = this->read_stmnts_check( tokty_endpackage );
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
			
			this->check_token( tokty_from );
			string url = this->read_pkg_name();
			imp.put( "from", url );

			if ( this->try_name( "alias" ) ) {
	        	Item item = this->read_id_item();
				imp.put( "alias", item->nameString() );
			}
			
			if ( this->try_name( "into" ) ) {
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

Node ReadStateClass::read_opt_expr_prec( int prec ) {
	ItemFactory ifact = this->item_factory;
	Node e = this->prefix_processing();
	if ( not e ) return Node();
	for(;;){
	    int q;
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
	        e = this->postfix_processing( e, it, q );
		} else {
			break;
		}
    }
    return e;
}

