#ifndef ARITY_HPP
#define ARITY_HPP

#include "term.hpp"
#include "dontknow.hpp"

int arity_add( int a, int b );
int arity_join( int a, int b );
int arity_term( Term term );

class Arity {
private:
	//	These fields should fit into 1 word (4 bytes)
	bool more;
	short arity;
	
public:
	Arity add( Arity that );
	Arity join( Arity that );
	void check( int actual_nargs );
	bool isOK( int actual_nargs );
	bool isZero();
	bool isntZero();
	
public:
	Arity( int a ) : more( false ), arity( a ) {}
	Arity( int a, bool m ) : more( m ), arity( a ) {}
	Arity( Term t );
};

#endif
