#include <iostream>

#include <cstdio>
#include <cstdlib>

#include <syslog.h>

#include "mishap.hpp"
#include "item_factory.hpp"
#include "read_expr.hpp"

using namespace std;

#define APP_TITLE 	"common2gnx"
#define LOG_FACILITY	LOG_LOCAL2

int main( int argc, char **argv, char **envp ) {
	openlog( APP_TITLE, 0, LOG_FACILITY );
	setlogmask( LOG_UPTO( LOG_INFO ) );

	try {
		FILE * in;
		if ( argc == 2 ) {
			in = fopen( argv[1], "r" ); 
			if ( in == NULL ) throw Mishap( "Cannot open file" ).culprit( "Filename", argv[1] );
			syslog( LOG_INFO, "Converting %s", argv[1] );
		} else {
			in = stdin;
			syslog( LOG_INFO, "Converting standard input (%s)", argv[1] );
		}
		for (;;) {
			//try {
				ItemFactoryClass ifact( in );
				ReadStateClass input( &ifact );
				for (;;) {
					Node n = input.read_opt_expr();
					if ( not n ) return EXIT_SUCCESS;
					n->render( cout );
					cout << endl;
					input.checkSemi();
				};
			//} catch ( Mishap & m ) {
			//	m.report();
			//}
		} 
	} catch ( Mishap & m ) {
		m.gnxReport();
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


