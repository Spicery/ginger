#include "arity.hpp"
#include "mishap.hpp"

//#define DBG_ARITY

//	Just for debugging
#include <stdio.h>

int arity_add( int a, int b ) {
	return a >= 0 && b >= 0 ? a + b : DONTKNOW;
}

int arity_join( int a, int b ) {
	return a == b ? a : DONTKNOW;
}

/*
 * Refactor!
 */
int arity_analysis( Term term ) {
	Functor fnc = term_functor( term );
	switch ( fnc ) {
		case fnc_assign:   
			return 0;
		case fnc_id:       
		case fnc_fn:       
		case fnc_incr_by:  
		case fnc_decr_by:  
			return 1;
		case fnc_seq: {
			int n = term->count();
			int k = 0;
			for ( int i = 0; i < n; i++ ) {
				k = arity_add( k, arity_analysis( term->child( i ) ) );
			}
			return k;
		}
		case fnc_if:
			return(
				arity_join(
					arity_analysis( term->child( 1 ) ),
					arity_analysis( term->child( 2 ) )
				)
			);
		case fnc_for:
			return(
				arity_analysis( term->child( 2 ) ) == 0 ? 0 : DONTKNOW
			);
		default: {}
	}
	#ifdef DBG_ARITY
		printf( "Giving up with functor %s\n", functor_name( fnc ) );
	#endif
	return DONTKNOW;
}

bool Arity::isZero() const {
	return this->arity == 0;
}

bool Arity::isntZero() const {
	return this->arity != 0;
}

void Arity::check( const int actual_nargs ) const {
	if ( ! this->isOK( actual_nargs ) ) {
		throw Mishap( "Wrong number of arguments" );
	}
}

void Arity::check( const Arity that ) const {
	if ( ! this->isOK( that ) ) {
		throw Mishap( "Wrong number of arguments" );
	}
}

bool Arity::isOK( const int actual_nargs ) const {
	if ( this->arity == actual_nargs ) {
		return true;
	} else if ( this->more && ( this->arity < actual_nargs ) ) {
		return true;
	} else {
		return false;
	}
}

bool Arity::isOK( const Arity actual_params ) const {
	if ( this->arity == actual_params.arity ) {
		return true;
	} else if ( this->arity < actual_params.arity ) {
		return this->more;
	} else if ( this->arity > actual_params.arity ) {
		return actual_params.more;
	} else {
		//	Unreachable.
		throw;
	}
}

Arity Arity::add( const Arity that ) const {
	int n = this->arity + that.arity;
	int m = this->more || that.more;
	return Arity( n, m );
}

Arity Arity::join( const Arity that ) const {
	if ( ( this->arity == that.arity ) && ( this->more == that.more ) ) {
		return *this;
	} else {
		return Arity( 0, true );
	}
}

Arity::Arity( Term term ) {
	Functor fnc = term_functor( term );
	switch ( fnc ) {
		case fnc_assign: {
			this->arity = 0;
			this->more = false;
			break;
		}
		case fnc_id:       
		case fnc_fn:       
		case fnc_incr_by:  
		case fnc_decr_by: {
			this->arity = 1;
			this->more = false;
			break;
		}
		case fnc_seq: {
			int n = term_count( term );
			Arity k( 0 );
			for ( int i = 0; i < n; i++ ) {
				k = k.add( Arity( term->child( i ) ) );
			}
			this->arity = k.arity;
			this->more = k.more;
			break;
		}
		case fnc_if: {
			Arity thenpart( term->child( 1 ) );
			Arity elsepart( term->child( 2 ) );
			Arity j = thenpart.join( elsepart );
			this->arity = j.arity;
			this->more = j.more;
			break;
		}
		case fnc_for: {
			this->arity = 0;
			this->more = Arity( term->child( 2 ) ).isntZero();
			break;
		}
		default: {
			this->arity = 0;
			this->more = true;
			break;
		}
	}
	#ifdef DBG_ARITY
		printf( "Giving up with functor %s\n", functor_name( fnc ) );
	#endif
}