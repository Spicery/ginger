#include "printfn.hpp"
//#include "special.hpp"
#include "key.hpp"

#include <stdio.h>

//	r is a pointer to the key of a function object.
void printfn( Ref x ) {
	Ref *r = RefToPtr4( x );
	//	Ref K = r[ 0 ];
	int A = SmallToInt( r[ -1 ] );
	int N = ToInt( r[ -2 ] );
	int R = ToInt( r[ -3 ] );
	int L = ToInt( r[ -4 ] );
	printf( "define: %d args, %d locals, %d results\n", A, N, R );
	{
		Ref *pc = r + 1;
	    while ( pc <= r + L ) {
		    printf( "[ %d ]\t", pc - r );
		    pc = special_show( pc );
	    }
	}
	printf( "enddefine\n" );
}
