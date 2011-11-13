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

#include "arity.hpp"
#include "mishap.hpp"
#include <string>
#include <sstream>

//#define DBG_ARITY

//	Just for debugging
#include <iostream>


bool Arity::isZero() const {
	return this->arity == 0;
}

bool Arity::isntZero() const {
	return this->arity != 0;
}

bool Arity::isExact() const {
	return not( this->more );
}

bool Arity::isntExact() const {
	return this->more;
}

int Arity::count() const {
	return this->arity;
}

void Arity::check( const int actual_nargs ) const {
	if ( ! this->isOK( actual_nargs ) ) {
		throw Ginger::Mishap( "Wrong number of arguments" );
	}
}

std::string Arity::toString() const {
	std::stringstream ss;
	ss << this->arity << ( this->more ? "+" : "" );
	return ss.str();
}

void Arity::check( const Arity that ) const {
	if ( ! this->isOK( that ) ) {
		throw Ginger::Mishap( "Wrong number of arguments" ).culprit( "Expected", this->toString() ).culprit( "Actual", that.toString() );
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
	#ifdef DBG_ARITY
		std::cout << "> Analysing " << functor_name( term_functor( term ) ) << std::endl;
	#endif
	Functor fnc = term_functor( term );
	switch ( fnc ) {
		case fnc_assign: {
			this->arity = 0;
			this->more = false;
			break;
		}
		case fnc_assert_single:
		case fnc_assert_bool:
		case fnc_int:
		case fnc_bool:
		case fnc_char:
		case fnc_id:       
		case fnc_fn:       
		case fnc_incr_by:  
		case fnc_decr_by: {
			this->arity = 1;
			this->more = false;
			break;
		}
		case fnc_block:
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
			Term t = term->child( 1 );
			Arity a( t );
			this->arity = 0;
			this->more = a.isntZero();
			break;
		}
		default: {
			this->arity = 0;
			this->more = true;
			break;
		}
	}
	#ifdef DBG_ARITY
		std::cout << "< Result for " << functor_name( term_functor( term ) ) << " = " << this->toString() << std::endl; 
	#endif
}
