#include <stdio.h>
#include <stdlib.h>

#include "implementation.h"
#include "special.h"
#include "common.h"
#include "machine_private.h"
#include "key.h"
#include "mishap.h"       	
#include "sys.h"

//#define DBG_SPECIAL


static void enter_error( int B, int A ) {
	reset( "Wrong number of arguments (found %d wanted %d)", B, A );
}

static void call_error( Ref r ) {
	reset( "Function needed - got something else" );
}



#ifdef IMPLEMENTATION1
	#include "special1.c.auto"
#endif

#ifdef IMPLEMENTATION2
	#include "special2.c.auto"
#endif

#ifdef IMPLEMENTATION3
	#include "special3.c.auto"
#endif

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
				printf( "%s ", ((Ident)( *pc ))->name );
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
