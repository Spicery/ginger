/******************************************************************************\
	Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
	of the GNU General Public License. This file is part of Ginger.

    Ginger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Ginger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Ginger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

#include "arity.hpp"
#include "mishap.hpp"
#include <string>
#include <sstream>

//#define DBG_ARITY

//	Just for debugging
#include <iostream>
#include <sstream>

namespace Ginger {

Arity::Arity( const std::string & s ) {
	int n;
	std::stringstream out( s );
	out >> n;
	this->arity = n;
	this->more = s.find( '+' ) != std::string::npos;
}

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

bool Arity::isExact( const int count ) const {
	return not( this->more ) && this->arity == count;
}

bool Arity::isntExact( const int count ) const {
	return this->more && this->arity == count;
}

int Arity::count() const {
	return this->arity;
}

void Arity::check( const int actual_nargs ) const {
	if ( ! this->isOK( actual_nargs ) ) {
		throw Mishap( "Wrong number of arguments" );
	}
}

std::string Arity::toString() const {
	std::stringstream ss;
	ss << this->arity << ( this->more ? "+" : "" );
	return ss.str();
}

void Arity::check( const Arity that ) const {
	if ( ! this->isOK( that ) ) {
		throw Mishap( "Wrong number of arguments" ).culprit( "Expected", this->toString() ).culprit( "Actual", that.toString() );
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

Arity Arity::unify( const Arity that ) const {
	const int count = this->arity <= that.arity ? this->arity : that.arity;
	const bool more = this->more || that.more || ( this->arity != that.arity );
	return Arity( count, more );
}

Arity Arity::join( const Arity that ) const {
	if ( ( this->arity == that.arity ) && ( this->more == that.more ) ) {
		return *this;
	} else {
		return Arity( 0, true );
	}
}



} // namespace
