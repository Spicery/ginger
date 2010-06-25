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
	if ( it->functor != fnc_id ) {
		throw Mishap( "Identifier expected" ).culprit( "Found", it->nameString() );
	}
	NodeFactory id( "id" );
	id.putAttr( "name", it->nameString() );
	return id.node();
}

Item ReadStateClass::read_id_item() {
	Item it = this->item_factory->read();
	if ( it->functor != fnc_id ) {
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

void ReadStateClass::check_token( Functor fnc ) {
	ItemFactory ifact = this->item_factory;
	Item it = ifact->read();
	ifact->drop();
	if ( it->functor != fnc ) {
		throw Mishap( "Unexpected token" ).culprit( "Found", it->nameString() ).culprit( "Expected", functor_name( fnc ) );
	}
}

void ReadStateClass::checkSemi() {
	this->check_token( fnc_semi );
}

bool ReadStateClass::try_token( Functor fnc ) {
	ItemFactory ifact = this->item_factory;
	if ( ifact->peek()->functor == fnc ) {
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

Node ReadStateClass::read_stmnts_check( Functor fnc ) {
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

Node ReadStateClass::read_expr_check( Functor fnc ) {
	Node t = this->read_expr();
	this->check_token( fnc );
	return t;
}

Node ReadStateClass::postfix_processing( Node lhs, Item item, int prec ) {
	Role role = item->role;
	Functor fnc = item->functor;
	this->clearPatternMode();			//	RHS of operators are never in pattern mode.
	if ( role.IsPattern() ) {
		lhs->updateAsPattern();
		switch ( fnc ) {
			case fnc_bind: {
				Node rhs = this->read_expr_prec( prec );
				NodeFactory bind( "dec" );
				bind.addNode( lhs );
				bind.addNode( rhs );
				return bind.node();
			}
			case fnc_from: {				
				Node from_expr = this->read_expr();
				this->check_token( fnc_to );
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
			a.putAttr( "name", functor_as_sysapp( fnc ) );
			a.addNode( lhs );
			a.addNode( this->read_expr_prec( prec ) );
			return a.node();
		} else if ( role.IsForm() ) {
			NodeFactory a( functor_as_tag( fnc ) );
			a.addNode( lhs );
			a.addNode( this->read_expr_prec( prec ) );
			return a.node();			
		} else {
			throw;
		}
	} else {
		switch ( fnc ) {
			case fnc_semi: {
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
			case fnc_oparen: {
				Node rhs = this->read_stmnts_check( fnc_cparen );
				return makeApp( lhs, rhs );
			}
			case fnc_at:
			case fnc_dot: {
				Node func = this->read_expr_prec( prec_tight );
				Node rhs = this->read_opt_expr_prec( prec );			
				NodeFactory seq( "seq" );
				seq.addNode( lhs );
				if ( ! not rhs ) { seq.addNode( rhs ); }
				return makeApp( func, seq.node() );
			}
			case fnc_int: {
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

Node ReadStateClass::read_if( Functor sense, Functor closer ) {
	Node pred = this->read_expr();
	if ( ! this->try_token( fnc_then ) ) {
		this->check_token( fnc_do );
	}
	Node then_part = this->read_stmnts();
	if ( this->try_token( fnc_else ) ) {
		NodeFactory ifunless( sense == fnc_if ? "if" : "unless" );
		ifunless.addNode( pred );
		ifunless.addNode( then_part );
		ifunless.addNode( this->read_stmnts_check( fnc_endif ) );
		return ifunless.node();
		//return term_new_basic3( sense, pred, then_part, this->read_stmnts_check( fnc_endif ) );
	} else if ( this->try_token( closer ) ) {
		NodeFactory ifunless( sense == fnc_if ? "if" : "unless" );
		ifunless.addNode( pred );
		ifunless.addNode( then_part );
		return ifunless.node();
		//return term_new_basic2( sense, pred, then_part );
	} else {
		Functor new_sense;
		if ( this->try_token( fnc_elseif ) ) {
			new_sense = fnc_if;
		} else {
			this->check_token( fnc_elseunless );
			new_sense = fnc_unless;
		}
		NodeFactory ifunless( sense == fnc_if ? "if" : "unless" );
		ifunless.addNode( pred );
		ifunless.addNode( then_part );
		ifunless.addNode( this->read_if( new_sense, closer ) );
		return ifunless.node();
		//return term_new_basic3( sense, pred, then_part, this->read_if( new_sense, closer ) );
	}
}

Node ReadStateClass::read_syscall() {
	ItemFactory ifact = this->item_factory;	
	Item it = ifact->read();
	if ( it->functor == fnc_id ) {
		//const std::string & name = it->nameString();
		NodeFactory sc( "syscall" );
		sc.putAttr( "name", it->nameString() );
		return sc.node();	//term_new_ref( fnc_syscall, (Ref)sc );
	} else {
		throw Mishap( "Invalid token after >-> (syscall) arrow" ).culprit( it->nameString() );
	}
}

/*Node ReadStateClass::read_bindings() {
	NodeFactory bindings( "bindings" );
	return bindings.node();		//term_new_basic0( fnc_bindings );
}*/

/*Node ReadStateClass::read_conditions() {
	NodeFactory conditions( "conditions" );
	for (;;) {
	    if ( this->try_token( fnc_while ) ) {
	    	conditions.addNode( this->read_expr() );
	    } else if ( this->try_token( fnc_until ) ) {
	    	conditions.start( "not" );
	    	conditions.addNode( this->read_expr() );
	    	conditions.end();
	    } else {
			break;
	    }
	}
	return conditions.node();
}*/

Node ReadStateClass::read_for() {
	Node query = this->read_query();
	this->check_token( fnc_do );
	Node body = this->read_stmnts_check( fnc_endfor );
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
	Functor fnc = item->functor;
	Role role = item->role;

	if ( role.IsLiteral() ) {
		NodeFactory simple( functor_as_type( fnc ) );
		simple.putAttr( "value", item->nameString() );
	 	return simple.node();
	} else if ( role.IsUnary() ) {
		if ( role.IsForm() ) {
			NodeFactory unary( functor_as_tag( fnc ) );
			unary.addNode( this->read_expr_prec( item->precedence ) );
			return unary.node();
		} else if ( role.IsSys() ) {
			NodeFactory sysapp( "sysfn" );
			sysapp.putAttr( "value", functor_as_sysapp( fnc ) );
			return sysapp.node();
		} else {
			throw;	// 	Unreachable.
		}
	}

	switch ( fnc ) {
		case fnc_id: {
			NodeFactory id( this->pattern_mode ? "var" : "id" );
			id.putAttr( "name", item->nameString() );
			return id.node();
		}
		case fnc_string: {
			//printf( "Copying string %s\n", (char *)item->extra );
			NodeFactory str( "string" );
			str.putAttr( "value", item->nameString() );	//	Prolly wrong.
			return str.node();
		}
		case fnc_charseq: {
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
        case fnc_var : {
	        Item item = this->read_id_item();
	        this->check_token( fnc_bind );
	        NodeFactory dec( "dec" );
	        dec.start( "var" );
	        dec.putAttr( "name", item->nameString() );
	        dec.end();
	        dec.addNode( this->read_expr() );
	        return dec.node();
        }
		case fnc_oparen: {
			return this->read_stmnts_check( fnc_cparen );
		}
		case fnc_unless: {
			return this->read_if( fnc_unless, fnc_endunless );
		}
		case fnc_if: {
			return this->read_if( fnc_if, fnc_endif );
		}
		case fnc_syscall: {
			return this->read_syscall();
		}
		case fnc_for: {
		  	return this->read_for();
		}
		case fnc_define: {
			ReadStateClass pattern( *this );
			pattern.setPatternMode();
			Node ap = pattern.read_expr_prec( prec_arrow );
			Node fn;
			Node args;
			flatten( ap, fn, args );
			const std::string name = fn->get( "name" );
			this->check_token( fnc_arrow );
			Node body = this->read_stmnts_check( fnc_enddefine );
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
			//return term_new_basic3( fnc_define, fn, args, body );
		}
		case fnc_fn: {
			ReadStateClass pattern( *this );
			pattern.setPatternMode();
			Node args = pattern.read_expr_prec( prec_arrow );
			this->check_token( fnc_arrow );
			//args = just_args( args );
			Node body = this->read_stmnts_check( fnc_endfn );
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
			if ( it->functor == fnc_int ) {
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
