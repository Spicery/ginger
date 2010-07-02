#ifndef ARITY_HPP
#define ARITY_HPP

#include "term.hpp"
#include "dontknow.hpp"

int arity_add( const int a, const int b );
int arity_join( const int a, const int b );
int arity_analysis( Term term );

class Arity {
private:
	//	These fields should fit into 1 word (4 bytes)
	bool more;
	short arity;
	
public:
	Arity add( const Arity that ) const;
	Arity join( const Arity that ) const;
	void check( const Arity that ) const;
	void check( const int actual_nargs ) const;
	bool isOK( const Arity that ) const;
	bool isOK( const int actual_nargs ) const;
	bool isZero() const;
	bool isntZero() const;
	
public:
	Arity( int a ) : more( false ), arity( a ) {}
	Arity( int a, bool m ) : more( m ), arity( a ) {}
	Arity( Term t );
};

#endif
