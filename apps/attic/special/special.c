#include <stdio.h>
#include <stdlib.h>

#include "implementation.hpp"
#include "special.hpp"
#include "common.hpp"
#include "machine_private.hpp"
#include "key.hpp"
#include "mishap.hpp"
#include "sys.hpp"

//#define DBG_SPECIAL


void enter_error( int B, int A ) {
	reset( "Wrong number of arguments (found %d wanted %d)", B, A );
}

void call_error( Ref r ) {
	reset( "Function needed - got something else" );
}

#include "footer.c.auto"

Ref *special_show( Ref *pc ) {
	Special spc = ( Special )( *pc );
	const char *types = classify( spc );
	while ( *types != '\0' ) {
		switch ( *types++ ) {
			case 'i': {
				printf( "%s ", special_name( spc ) );
				break;
			}
			case 'r': {
				printf( "%d ", (int)( *pc ) );
				break;
			}
			case 'c': {
		        sys_print( *pc );		
		        printf( " " );
				break;
			}
			case 'v': {
				printf( "%s ", ((Ident)( *pc ))->getNameString().c_str() );
				break;
			}
			default:
				this_never_happens();
		}
		pc += 1;
	}
	printf( "\n" );
	return pc;
}
