#include <iostream>

#include "print.hpp"
#include "sys.hpp"


#include <stdio.h>

void print_results( Machine vm, float time_taken ) {
	int i;
	int n = vm->vp - vm->vp_base;
	printf(
		"There %s %d result%s (%.2fs)\n",
		n == 1 ? "is" : "are",
		n,
		n == 1 ? "" : "s",
		time_taken
	);
	for ( i = 0; i < n; i++ ) {
		printf( "%d.\t", i+1 );
		sys_print( std::cout, vm->vp[ 1 + i - n ] );
		printf( "\n" );
	}
	printf( "\n" );
	vm->vp = vm->vp_base;
}
