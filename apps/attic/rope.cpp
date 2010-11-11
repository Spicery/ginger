#include <string.h>

#include "rope.hpp"
#include "mishap.hpp"
#include "pool.hpp"

enum {
	BUMP = 16
};

class RopeClass {
public:
	Pool 	pool;
    int 	size;
    int 	used;
    char 	*data;
};


// Ensure there is room for at least n more bytes
// in the rope's buffer.
//
static Rope bump( Rope r, int n ) {
    int size = r->size;
    int used = r->used;
    int newused = used + n;
    if ( newused > size ) {
	    int newsize = newused + BUMP;
	    r->data = (char *)pool_realloc( r->pool, r->data, newsize );
	    r->size = newsize;
    }
    return r;
}

// Null terminate the rope.
//
static Rope nullify( Rope r ) {
    bump( r, 1 );
    r->data[ r->used ] = '\0';
    return r;
}

Rope rope_append( Rope r, const char *str ) {
    size_t n = strlen( str );
    bump( r, n );
    strncpy( r->data + r->used, str, n );
    r->used += n;
    return r;
}

Rope rope_push( Rope r, char ch ) {
    bump( r, 1 );
    r->data[ r->used ] = ch;
    r->used += 1;
    return r;
}

char rope_pop( Rope r ) {
	if ( r->used >= 1 ) {
		r->used -= 1;
		return r->data[ r->used ];
	} else {
		mishap( "Trying to pop from an empty rope" );		
		return 0;
	}
}

Rope rope_new( Pool pool, const char *s ) {
    Rope r = (Rope )pool_malloc( pool, sizeof( RopeClass ) );
	r->pool = pool;
	r->size = 0;
	r->used = 0;
	r->data = NULL;
	if ( s != NULL ) {
		rope_append( r, s );
	}
    return r;
}

void rope_free( Rope r ) {
	pool_dealloc( r->pool, r->data );
	pool_dealloc( r->pool, r );
}	

Rope rope_clear( Rope r ) {
    r->used = 0;
    return r;
}

char *rope2str( Rope r ) {
    return pool_strdup( r->pool, nullify( r )->data );
}

char *nc_rope2str( Rope r ) {
    return nullify( r )->data;
}

int rope_length( Rope r ) {
    return r->used;
}

char rope_index( Rope r, int n ) {
	if (!( 0 <= n && n < r->used )) {
		mishap( "Rope index (%d) out of range (0-%d)", n, r->used );
	}
	return r->data[ n ];
}
