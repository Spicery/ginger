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

#ifndef ARITY_HPP
#define ARITY_HPP

#include "term.hpp"
#include "dontknow.hpp"
#include <string>

//int arity_add( const int a, const int b );
//int arity_join( const int a, const int b );
//int arity_analysis( Term term );

class Arity {
private:
	//	These fields should fit into 1 word (4 bytes)
	bool more;
	short arity;
	
public:
	std::string toString() const;
	Arity add( const Arity that ) const;
	Arity join( const Arity that ) const;
	void check( const Arity that ) const;
	void check( const int actual_nargs ) const;
	bool isOK( const Arity that ) const;
	bool isOK( const int actual_nargs ) const;
	bool isZero() const;
	bool isntZero() const;
	bool isExact() const;
	bool isntExact() const;
	int count() const;
	
public:
	Arity( int a ) : more( false ), arity( a ) {}
	Arity( int a, bool m ) : more( m ), arity( a ) {}
	Arity( Term t );
};

#endif
