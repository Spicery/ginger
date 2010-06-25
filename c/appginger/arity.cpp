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

int arity_term( Term term ) {
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
			int n = term_arity( term );
			int k = 0;
			for ( int i = 0; i < n; i++ ) {
				k = arity_add( k, arity_term( term_index( term, i ) ) );
			}
			return k;
		}
		case fnc_if:
			return(
				arity_join(
					arity_term( term_index( term, 1 ) ),
					arity_term( term_index( term, 2 ) )
				)
			);
		case fnc_for:
			return(
				arity_term( term_index( term, 2 ) ) == 0 ? 0 : DONTKNOW
			);
		default: {}
	}
	#ifdef DBG_ARITY
		printf( "Giving up with functor %s\n", functor_name( fnc ) );
	#endif
	return DONTKNOW;
}

bool Arity::isZero() {
	return this->arity == 0;
}

bool Arity::isntZero() {
	return this->arity != 0;
}

void Arity::check( int actual_nargs ) {
	if ( ! this->isOK( actual_nargs ) ) {
		throw Mishap( "Wrong number of arguments" );
	}
}

bool Arity::isOK( int actual_nargs ) {
	if ( this->arity == actual_nargs ) {
		return true;
	} else if ( this->more && ( this->arity < actual_nargs ) ) {
		return true;
	} else {
		return false;
	}
}

Arity Arity::add( Arity that ) {
	int n = this->arity >= 0 + that.arity;
	int m = this->more || that.more;
	return Arity( n, m );
}

Arity Arity::join( Arity that ) {
	if ( ( this->arity == that.arity ) && ( this->more == that.more ) ) {
		return *this;
	} else {
		return Arity( 0, true );
	}
}

Arity::Arity( Term term ) {
	Functor fnc = term_functor( term );
	switch ( fnc ) {
		case fnc_assign: 
			this->arity = 0;
			break;
		case fnc_id:       
		case fnc_fn:       
		case fnc_incr_by:  
		case fnc_decr_by:  
			this->arity = 1;
			break;
		case fnc_seq: {
			int n = term_arity( term );
			Arity k( 0 );
			for ( int i = 0; i < n; i++ ) {
				k = k.add( Arity( term_index( term, i ) ) );
			}
			this->arity = k.arity;
		}
		case fnc_if:
			this->arity = 
				Arity( term_index( term, 1 ) ).join( 
					Arity( term_index( term, 2 ) ) 
				).arity;
			break;
		case fnc_for:
			this->arity = 0;
			this->more = Arity( term_index( term, 2 ) ).isntZero();
			break;
		default: {}
	}
	#ifdef DBG_ARITY
		printf( "Giving up with functor %s\n", functor_name( fnc ) );
	#endif
	this->arity = 0;
	this->more = true;
}