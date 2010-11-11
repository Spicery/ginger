#include "term.hpp"
#include "item_factory.hpp"
#include "read_expr.hpp"
#include "mishap.hpp"
#include "item.hpp"
#include "sys.hpp"

#include <stdio.h>

Term ReadStateClass::read_id() {
	Item it = item_factory_read( this->item_factory );
	if ( it->functor != fnc_id ) {
		reset( "Identifier expected but found '%s'", it->nameString().c_str() );
	}
	return term_new_id( it->nameString() );
}

Term ReadStateClass::read_expr_prec( int prec ) {
	Term e = this->read_opt_expr_prec( prec );
	if ( not e ) {
		Item it = item_factory_peek( this->item_factory );
		if ( it->item_is_anyfix() ) {
			reset( "Found reserved word '%s'", it->nameString().c_str() );
		} else {
			reset( "Unexpected end of expression (at ifact '%s')", it->nameString().c_str() );
		}
	}
	return e;
}

void ReadStateClass::check_token( Functor fnc ) {
	ItemFactory ifact = this->item_factory;
	Item it = item_factory_read( ifact );
	if ( it->functor != fnc ) {
		reset(
			"Found '%s' but expecting '%s'",
			it->nameString().c_str(),
			functor_name( fnc )
		);
	}
	item_factory_drop( ifact );
}

bool ReadStateClass::try_token( Functor fnc ) {
	ItemFactory ifact = this->item_factory;
	if ( item_factory_peek( ifact )->functor == fnc ) {
		item_factory_drop( ifact );
		return true;
	} else {
		return false;
	}
}

Term ReadStateClass::read_stmnts() {
	Term e = this->read_opt_expr_prec( prec_max );
	return not e ? term_skip : e;
}

Term ReadStateClass::read_stmnts_check( Functor fnc ) {
	Term t = this->read_stmnts();
	this->check_token( fnc );
	return t;
}
	
Term ReadStateClass::read_expr() {
	return this->read_expr_prec( prec_semi );
}

Term ReadStateClass::read_expr_check( Functor fnc ) {
	Term t = this->read_expr();
	this->check_token( fnc );
	return t;
}

Term ReadStateClass::postfix_processing( Term lhs, Item item, int prec ) {
	Role role = item->role;
	Functor fnc = item->functor;
	//Pool pool = r->pool;
	ItemFactory ifact = this->item_factory;
	if ( IsBinary( role ) ) {
		return term_new_basic2( fnc, role, lhs, this->read_expr_prec( prec ) );
	}
	switch ( fnc ) {
		case fnc_semi: {
			Term rhs = this->read_opt_expr_prec( prec );
			return not( rhs ) ? lhs : term_new_basic2( fnc, lhs, rhs );
		}
		case fnc_oparen: {
			Term rhs = this->read_stmnts_check( fnc_cparen );
			return term_new_basic2( fnc_app, lhs, rhs );
		}
		case fnc_at:
		case fnc_dot: {
			Term id = this->read_id();
			Item it = item_factory_peek( ifact );
			if ( it->item_is_postfix() ) {
				return term_new_basic2( fnc_app, id, lhs );
			} else {
				Term rhs = this->read_expr_prec( prec );
				return(
					term_new_basic2(
						fnc_app,
						id,
						term_new_basic2( fnc_comma, lhs, rhs )
					)
				);
			}
		}
		case fnc_int: {
			Term t = term_new_basic2( fnc_add, AddOpRole, lhs, term_new_simple_item( item )  );
			fprintf( stderr, "DEBUG arity %d\n", term_arity( t ) );
			return t;
			/*return(
				term_new_role(
					fnc_add,
					AddOpRole,
					lhs,
					term_new_simple_item( item )
				)
			);*/
		}
		default: {
			reset( "This keyword not handled (%s)", functor_name( fnc ) );
		}
	}
    throw;	//	Unreachable.
}

Term ReadStateClass::read_if( Functor sense, Functor closer ) {
	Term pred = this->read_expr();
	if ( ! this->try_token( fnc_then ) ) {
		this->check_token( fnc_do );
	}
	Term then_part = this->read_stmnts();
	if ( this->try_token( fnc_else ) ) {
		return term_new_basic3( sense, pred, then_part, this->read_stmnts_check( fnc_endif ) );
	} else if ( this->try_token( closer ) ) {
		return term_new_basic2( sense, pred, then_part );
	} else {
		Functor new_sense;
		if ( this->try_token( fnc_elseif ) ) {
			new_sense = fnc_if;
		} else {
			this->check_token( fnc_elseunless );
			new_sense = fnc_unless;
		}
		return term_new_basic3( sense, pred, then_part, this->read_if( new_sense, closer ) );
	}
}

Term ReadStateClass::read_syscall() {
	ItemFactory ifact = this->item_factory;	
	Item it = item_factory_read( ifact );
	if ( it->functor == fnc_id ) {
		const std::string & name = it->nameString();
		SysMap::iterator smit = sysMap.find( name );
		if ( smit == sysMap.end() ) {
			mishap( "No such system call" );
		}
		SysCall * sc = smit->second;
		return term_new_ref( fnc_syscall, (Ref)sc );
	} else {
		mishap( "Invalid token after >-> (syscall) arrow, %s", functor_name( it->functor ) );
		throw;
	}
}

Term ReadStateClass::read_bindings() {
	return term_new_basic0( fnc_bindings );
}

Term ReadStateClass::read_conditions() {
	Term result = term_new_basic0( fnc_conditions );
	for (;;) {
	    if ( this->try_token( fnc_while ) ) {
			term_add( result, this->read_expr() );
	    } else if ( this->try_token( fnc_until ) ) {
	    	Term t = term_new_basic1( fnc_not, PrefixOpRole, this->read_expr() );
			term_add( result, t );
	    } else {
			break;
	    }
	}
	return result;
}

Term ReadStateClass::read_for() {
	Term bindings = this->read_bindings();
	Term condition  = this->read_conditions();
	this->check_token( fnc_do );
	Term body = this->read_stmnts_check( fnc_endfor );
	return term_new_basic3( fnc_for, bindings, condition, body );
}

//	One day this will be a nice error report.  Probably will need to pass
//	ifact through for line number information, too.
//
static void oops( Term ap ) {
	reset( "Invalid define header" );
}

//	ap is only passed in for error reporting.
static void squash( Term acc, Term ap, Term rhs ) {
	Functor fnc = term_functor( rhs );
	switch ( fnc ) {
		case fnc_skip: return;

		case fnc_comma: {
			squash( acc, ap, term_index( rhs, 0 ) );
			squash( acc, ap, term_index( rhs, 1 ) );
			return;
		}
		
		case fnc_id: {
			term_add( acc, rhs );
			return;
		}

		default: {
			oops( ap );
		}
	}
}

static void flatten( Term ap, Term *fn, Term *args ) {
	Term lhs, rhs;
	if (!( term_functor( ap ) == fnc_app && term_arity( ap ) == 2 )) oops( ap );
	lhs = term_index( ap, 0 );
	rhs = term_index( ap, 1 );
	if (!( term_functor( lhs ) == fnc_id )) oops( ap );
	*fn = lhs;
	*args = term_new_basic0( fnc_args );
	squash( *args, ap, rhs );
}

static Term just_args( Term args ) {
	Term acc = term_new_basic0( fnc_args );
	squash( acc, args, args );
	return acc;
}

Term ReadStateClass::prefix_processing() {
	ItemFactory ifact = this->item_factory;
	Item item = item_factory_read( ifact );
	Functor fnc = item->functor;
	Role role = item->role;

	if ( IsImmediate( role ) ) {
	 	return term_new_simple_item( item );
	} else if ( IsUnary( role ) ) {
		return term_new_basic1( fnc, role, this->read_expr_prec( item->precedence ) );
	} else if ( IsInLine( role ) ) {
		Term spc = term_new_simple_item( item );
		this->check_token( fnc_oparen );
		return(
		    term_new_basic2(
			    fnc_appspc,
			    spc,
			    this->try_token( fnc_cparen ) ? term_skip : this->read_expr_check( fnc_cparen )
		    )
		);
	}

	switch ( fnc ) {
		case fnc_id: {
			return term_new_id( item->nameString().c_str() );
		}
		case fnc_string: {
			printf( "Copying string %s\n", (char *)item->extra );
			return term_new_string( (char *)item->extra );
		}
		case fnc_charseq: {
			const char *r = item->nameString().c_str();
			int n = item->nameString().size();
			if ( n == 0 ) {
				return term_skip;
			} else {
				Term x = term_new_char( r[ n - 1 ] );	//	Get last item.
				int i;
				for ( i = n - 2; i >= 0; i-- ) {
					x = term_new_basic2( fnc_comma, term_new_char( r[ i ] ), x );
				}
				return x;
			}
		}
        case fnc_var : {
	        Term id = this->read_id();
	        this->check_token( fnc_eq );
	        return term_new_basic2( fnc_var, id, this->read_expr() );
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
			Term body;
			Term ap = this->read_expr_prec( prec_arrow );
			Term fn, args;
			flatten( ap, &fn, &args );
			this->check_token( fnc_arrow );
			body = this->read_stmnts_check( fnc_enddefine );
			return term_new_basic3( fnc_define, fn, args, body );
		}
		case fnc_fn: {
			Term body;
			Term args = this->read_expr_prec( prec_arrow );
			this->check_token( fnc_arrow );
			args = just_args( args );
			body = this->read_stmnts_check( fnc_endfn );
			return term_new_fn( term_anon, args, body );
		}
		default:
			;
	}
	item_factory_unread( ifact );
    return boost::shared_ptr< TermClass >();
}

Term ReadStateClass::read_opt_expr_prec( int prec ) {
	ItemFactory ifact = this->item_factory;
	Term e = this->prefix_processing();
	if ( not e ) return boost::shared_ptr< TermClass >();
	for(;;){
	    int q;
		Item it = item_factory_peek( ifact );
		if ( it->item_is_neg_num() ) {
			Term t = term_new_basic0( fnc_add, AddOpRole );
			term_add( t, e );
			term_add(
				t,
				it->functor == fnc_int ?
				term_new_simple_item( it ) :
				( reset( "Only integers supported so far (%s)", it->nameString().c_str() ), boost::shared_ptr< TermClass >() )
			);
			fprintf( stderr, "DEBUG arity %d\n", term_arity( t ) );
			e = t;
			/*e =
				term_new_role(
					fnc_add,
					AddOpRole,
					e,
					(
						it->functor == fnc_int ?
						term_new_simple_item( it ) :
						( reset( "Only integers supported so far (%s)", it->nameString().c_str() ), boost::shared_ptr< TermClass >() )
					)
				);*/
			item_factory_drop( ifact );
		} else if ( it->item_is_postfix() ) {
	        q = it->precedence;
            if ( q >= prec ) break;
	        item_factory_drop( ifact );
	        e = this->postfix_processing( e, it, q );
		} else {
			break;
		}
    }
    return e;
}
