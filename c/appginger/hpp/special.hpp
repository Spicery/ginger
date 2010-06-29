#ifndef SPECIAL_HPP
#define SPECIAL_HPP

typedef Ref *Special1( Ref *pc, Machine vm );
typedef void Special2( void );
typedef void * Special3

typedef union {
	Special1	special1;
	Special2	special2;
	Special3	special3;
} Special;


#include "implementation.hpp"

void enter_error( int B, int A );
void call_error( Ref r );
const char *special_name( Special );
Ref *special_show( Ref * );

#ifdef IMPLEMENTATION3
	void special_interpreter( int mode, Ref *pc, Machine vm );
#endif

#include "special.h.auto"

#endif
