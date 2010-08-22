#include <iostream>
using namespace std;

#include <stdio.h>
#include <stdlib.h>

#include "mishap.hpp"
#include "item_factory.hpp"
#include "read_expr.hpp"

int main( int argc, char **argv, char **envp ) {
	for (;;) {
		try {
			ItemFactoryClass ifact( stdin );
			ReadStateClass input( &ifact );
			for (;;) {
				Node n = input.read_opt_expr();
				if ( not n ) return EXIT_SUCCESS;
				n->render( cout );
				cout << endl;
				input.checkSemi();
			};
		} catch ( Mishap & m ) {
			m.report();
		}
	} 
}


