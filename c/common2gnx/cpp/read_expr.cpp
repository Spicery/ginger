#include <stdexcept>
#include <string>

#include <stdio.h>
#include <assert.h>

#include "shared.hpp"
#include "mishap.hpp"

#include "item_factory.hpp"
#include "read_expr.hpp"
#include "item.hpp"
#include "role.hpp"
#include "sysconst.hpp"


static Node makeApp( Node lhs, Node rhs ) {
	if ( lhs->elementName() == "sysfn" ) {
		NodeFactory sysapp( "sysapp" );
		std::string name = lhs->get( "value" );
		sysapp.putAttr( "name", name );
		sysapp.addNode( rhs );
		return sysapp.node();
	} else {
		NodeFactory app( "app" );
		app.addNode( lhs );
		app.addNode( rhs );
		return app.node();
	}
}


Node ReadStateClass::read_id() {
	Item it = this->item_factory->read();
	if ( it->tok_type != tokty_id ) {
		throw Mishap( "Identifier expected" ).culprit( "Found", it->nameString() );
	}
	NodeFactory id( "id" );
	id.putAttr( "name", it->nameString() );
	return id.node();
}

Item ReadStateClass::read_id_item() {
	Item it = this->item_factory->read();
	if ( it->tok_type != tokty_id ) {
		throw Mishap( "Identifier expected" ).culprit( "Found", it->nameString() );
	}
	return it;
}

Node ReadStateClass::read_expr_prec( int prec ) {
	Node e = this->read_opt_expr_prec( prec );
	if ( not e ) {
		Item it = this->item_factory->peek();
		if ( it->item_is_anyfix() ) {
			throw Mishap( "Found reserved word" ).culprit( it->nameString() );
		} else {
			throw Mishap( "Unexpected end of expression" ).culprit( it->nameString() );
		}
	}
	return e;
}

void ReadStateClass::check_token( TokType fnc ) {
	ItemFactory ifact = this->item_factory;
	Item it = ifact->read();
	ifact->drop();
	if ( it->tok_type != fnc ) {
		throw Mishap( "Unexpected token" ).culprit( "Found", it->nameString() ).culprit( "Expected", tok_type_name( fnc ) );
	}
}

void ReadStateClass::checkSemi() {
	this->check_token( tokty_semi );
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
	Node e = this->read_opt_expr_prec( prec_max );
	if ( not e ) {
		NodeFactory skip( "seq" );
		return skip.node();
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
		lhs->updateAsPattern();
		switch ( fnc ) {
			case tokty_bind: {
				Node rhs = this->read_expr_prec( prec );
				NodeFactory bind( "dec" );
				bind.addNode( lhs );
				bind.addNode( rhs );
				return bind.node();
			}
			case tokty_from: {				
				Node from_expr = this->read_expr();
				this->check_token( tokty_to );
				Node to_expr = this->read_expr_prec( prec );
				NodeFactory node( "from" );
				node.addNode( lhs );
				node.addNode( from_expr );
				node.addNode( to_expr );
				return node.node();
			}
			default: 
				throw;
		}
	} else if ( role.IsBinary() ) {
		if ( role.IsSys() ) {
			NodeFactory a( "sysapp" );
			a.putAttr( "name", tok_type_as_sysapp( fnc ) );
			a.addNode( lhs );
			a.addNode( this->read_expr_prec( prec ) );
			return a.node();
		} else if ( role.IsForm() ) {
			NodeFactory a( tok_type_as_tag( fnc ) );
			a.addNode( lhs );
			a.addNode( this->read_expr_prec( prec ) );
			return a.node();			
		} else {
			throw;
		}
	} else {
		switch ( fnc ) {
			case tokty_semi: {
				Node rhs = this->read_opt_expr_prec( prec );
				if ( not( rhs ) ) {
					return lhs;
				} else {
					NodeFactory s( "seq" );
					s.addNode( lhs );
					s.addNode( rhs );
					return s.node();
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
				NodeFactory seq( "seq" );
				seq.addNode( lhs );
				if ( ! not rhs ) { seq.addNode( rhs ); }
				return makeApp( func, seq.node() );
			}
			case tokty_int: {
				NodeFactory add( "add" );
				add.addNode( lhs );
				add.start( "int" );
				add.putAttr( "value", item->nameString() );
				add.end();	
				Node t = add.node();
				//fprintf( stderr, "DEBUG arity %d\n", term_arity( t ) );
				return t;
			}
			default: {
				throw Mishap( "This keyword not handled" ).culprit( "Keyword", item->nameString() );
			}
		}
	}
    throw;	//	Unreachable.
}

static void predicate( TokType sense, NodeFactory & ifunless, Node pred ) {
	if ( sense == tokty_if ) {
		ifunless.addNode( pred );
	} else {
		ifunless.start( "sysval" );
		ifunless.putAttr( "name", "not" );
		ifunless.addNode( pred );
		ifunless.end();
	}
}

Node ReadStateClass::read_if( TokType sense, TokType closer ) {
	Node pred = this->read_expr();
	if ( ! this->try_token( tokty_then ) ) {
		this->check_token( tokty_do );
	}
	Node then_part = this->read_stmnts();
	if ( this->try_token( tokty_else ) ) {	
		NodeFactory ifunless( "if" );
		predicate( sense, ifunless, pred );
		ifunless.addNode( then_part );
		ifunless.addNode( this->read_stmnts_check( closer ) );
		return ifunless.node();
		//return term_new_basic3( sense, pred, then_part, this->read_stmnts_check( tokty_endif ) );
	} else if ( this->try_token( closer ) ) {
		NodeFactory ifunless( "if" );
		predicate( sense, ifunless, pred );
		ifunless.addNode( then_part );
		return ifunless.node();
		//return term_new_basic2( sense, pred, then_part );
	} else {
		TokType new_sense;
		if ( this->try_token( tokty_elseif ) ) {
			new_sense = tokty_if;
		} else {
			this->check_token( tokty_elseunless );
			new_sense = tokty_unless;
		}
		NodeFactory ifunless( "if" );
		predicate( sense, ifunless, pred );
		ifunless.addNode( then_part );
		ifunless.addNode( this->read_if( new_sense, closer ) );
		return ifunless.node();
		//return term_new_basic3( sense, pred, then_part, this->read_if( new_sense, closer ) );
	}
}

Node ReadStateClass::read_syscall() {
	ItemFactory ifact = this->item_factory;	
	Item it = ifact->read();
	if ( it->tok_type == tokty_id ) {
		//const std::string & name = it->nameString();
		NodeFactory sc( "syscall" );
		sc.putAttr( "name", it->nameString() );
		return sc.node();	//term_new_ref( tokty_syscall, (Ref)sc );
	} else {
		throw Mishap( "Invalid token after >-> (syscall) arrow" ).culprit( it->nameString() );
	}
}

Node ReadStateClass::read_for() {
	Node query = this->read_query();
	this->check_token( tokty_do );
	Node body = this->read_stmnts_check( tokty_endfor );
	NodeFactory for_node( "for" );
	for_node.addNode( query );
	for_node.addNode( body );
	return for_node.node();
}

//	ap is only passed in for error reporting.
static void squash( NodeFactory acc, Node rhs ) {
	const std::string 
	name = rhs->elementName();
	if ( name == "seq" ) {
		int n = rhs->size();
		for ( int i = 0; i < n; i++ ) {
			squash( acc, rhs->child( i ) );
		}
	} else if ( name == "var" ) {
		acc.addNode( rhs );
	/*} else if ( name == "id" ) {
		const std::string nm = rhs->get( "name" );
		acc.start( "var" );
		acc.putAttr( "name", nm );
		acc.end();
		return;*/
	} else {
		throw Mishap( "Invalid form for definition" );
	}
}

static void flatten( Node & ap, Node & fn, Node & args ) {
	if ( 
		ap->isElement() && 
		ap->elementName() == std::string( "app" ) 
	) {
		fn = ap->child( 0 );
		Node rhs = ap->child( 1 );
		NodeFactory acc( "seq" );
		squash( acc, ap->child( 1 ) ); 
		args = acc.node();
	} else {
		throw Mishap( "Invalid form for definition" );
	}
}

/*static Node just_args( Node args ) {
	NodeFactory acc( "seq" );
	squash( acc, args );
	return acc.node();
}*/

Node ReadStateClass::prefix_processing() {
	ItemFactory ifact = this->item_factory;
	Item item = ifact->read();
	TokType fnc = item->tok_type;
	Role role = item->role;

	if ( role.IsLiteral() ) {
		NodeFactory simple( tok_type_as_type( fnc ) );
		simple.putAttr( "value", item->nameString() );
	 	return simple.node();
	} else if ( role.IsUnary() ) {
		if ( role.IsForm() ) {
			NodeFactory unary( tok_type_as_tag( fnc ) );
			unary.addNode( this->read_expr_prec( item->precedence ) );
			return unary.node();
		} else if ( role.IsSys() ) {
			NodeFactory sysapp( "sysfn" );
			sysapp.putAttr( "value", tok_type_as_sysapp( fnc ) );
			return sysapp.node();
		} else {
			throw;	// 	Unreachable.
		}
	}

	switch ( fnc ) {
		case tokty_id: {
			std::string & name = item->nameString();
			SysConst * sysc = lookupSysConst( name );
			if ( sysc != NULL ) {
				NodeFactory constant( sysc->tag );
				constant.putAttr( "value", sysc->value );
				return constant.node();
			} else {
				NodeFactory id( this->pattern_mode ? "var" : "id" );
				id.putAttr( "name", name );
				return id.node();
			}
		}
		case tokty_string: {
			//printf( "Copying string %s\n", (char *)item->extra );
			NodeFactory str( "string" );
			str.putAttr( "value", item->nameString() );	//	Prolly wrong.
			return str.node();
		}
		case tokty_charseq: {
			int n = item->nameString().size();
			if ( n == 0 ) {
				NodeFactory skip( "seq" );
				return skip.node();
			} else {
				NodeFactory charseq( "seq" );
				const std::string & s = item->nameString();
				std::string::const_iterator iter = s.begin();
				for ( iter = s.begin(); iter != s.end(); ++iter ) {
					charseq.start( "char" );
					charseq.putAttr( "value", std::string() + *iter );	//	 WRONG
					charseq.end();
				}
				return charseq.node();
			}
		}
		case tokty_val:
        case tokty_var : {
	        Item item = this->read_id_item();
	        this->check_token( tokty_bind );
	        NodeFactory dec( "dec" );
	        dec.start( "var" );
	        dec.putAttr( "name", item->nameString() );
	        if ( fnc == tokty_val ) {
	        	dec.putAttr( "protected", "true" );
	        }
	        dec.end();
	        dec.addNode( this->read_expr() );
	        return dec.node();
        }
		case tokty_oparen: {
			return this->read_stmnts_check( tokty_cparen );
		}
		case tokty_obracket: {
			NodeFactory list( "sysapp" );
			list.putAttr( "name", "newList" );
			list.addNode( this->read_stmnts_check( tokty_cbracket ) );
			return list.node();
		}
		case tokty_obrace: {
			NodeFactory list( "sysapp" );
			list.putAttr( "name", "newVector" );
			list.addNode( this->read_stmnts_check( tokty_cbrace ) );
			return list.node();
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
		case tokty_define: {
			ReadStateClass pattern( *this );
			pattern.setPatternMode();
			Node ap = pattern.read_expr_prec( prec_arrow );
			Node fn;
			Node args;
			flatten( ap, fn, args );
			const std::string name = fn->get( "name" );
			this->check_token( tokty_arrow );
			Node body = this->read_stmnts_check( tokty_enddefine );
			NodeFactory def( "dec" );
			def.start( "var" );
			def.putAttr( "name", name );
			def.putAttr( "protected", "true" );
			def.end();
			def.start( "fn" );
			def.putAttr( "name", name );
			def.addNode( args );
			def.addNode( body );
			def.end();
			return def.node();
			//return term_new_basic3( tokty_define, fn, args, body );
		}
		case tokty_fn: {
			ReadStateClass pattern( *this );
			pattern.setPatternMode();
			Node args = pattern.read_expr_prec( prec_arrow );
			this->check_token( tokty_arrow );
			//args = just_args( args );
			Node body = this->read_stmnts_check( tokty_endfn );
			NodeFactory fn( "fn" );
			fn.addNode( args );
			fn.addNode( body );
			return fn.node();
		}
		default:
			;
	}
	ifact->unread();
    return shared< NodeClass >();
}

Node ReadStateClass::read_opt_expr_prec( int prec ) {
	ItemFactory ifact = this->item_factory;
	Node e = this->prefix_processing();
	if ( not e ) return shared< NodeClass >();
	for(;;){
	    int q;
		Item it = ifact->peek();
		if ( it->item_is_neg_num() ) {
			NodeFactory t( "sysapp" );
			t.putAttr( "name", "+" );
			t.addNode( e );			
			if ( it->tok_type == tokty_int ) {
				t.start( "int" );
				t.putAttr( "value", it->nameString() );
				t.end();
			} else {
				throw Mishap( "Only integers supported so far" ).culprit( "Item", it->nameString() );
			}
			//fprintf( stderr, "DEBUG arity %d\n", term_arity( t ) );
			e = t.node();
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
