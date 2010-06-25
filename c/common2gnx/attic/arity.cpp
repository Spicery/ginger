#include "arity.hpp"
#include "role.hpp"

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
	Role role = term_role( term );
	int r = RoleToOutArity( role );
	if ( r != DONTKNOW ) return r;
	switch ( fnc ) {
		//case fnc_skip:		//	should now be superfluous
		case fnc_assign:    //	MAKE superfluous
			return 0;
		case fnc_id:        //	MAKE superfluous
		case fnc_fn:        //	MAKE superfluous
        //case fnc_pow:       //	probably superfluous
        //case fnc_mul:       //	probably superfluous
        //case fnc_div:       //	probably superfluous
        //case fnc_add:       //	probably superfluous
        //case fnc_sub:       //	probably superfluous
		case fnc_incr_by:   //	MAKE superfluous
		case fnc_decr_by:   //	MAKE superfluous
        //case fnc_append:    //	probably superfluous
        //case fnc_eq:        //	probably superfluous
        //case fnc_lt:        //	probably superfluous
        //case fnc_gt:        //	probably superfluous
        //case fnc_lte:       //	probably superfluous
        //case fnc_gte:       //	probably superfluous
        //case fnc_not:       //	probably superfluous
        //case fnc_and:       //	probably superfluous
        //case fnc_or:        //	probably superfluous
			return 1;
		case fnc_comma:
			return(
				arity_add(
					arity_term( term_index( term, 0 ) ),
					arity_term( term_index( term, 1 ) )
				)
			);
		case fnc_semi:
			return arity_term( term_index( term, 1 ) );
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
	printf( "Giving up with functor %s\n", functor_name( fnc ) );
	return DONTKNOW;
}
