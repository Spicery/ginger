#include "chain.hpp"
#include "mishap.hpp"
#include <vector>
using namespace std;

#include "chain.hpp"

//#include "pool.hpp"

#include <stdarg.h>

#ifndef NULL
#define NULL 0
#endif

enum {
    BUMP = 16
};

class ChainClass {
public:
    std::vector< Ref > alt_data;
};


// Null terminate the chain.
//
static Chain nullify( Chain r ) {
	r->alt_data.push_back( static_cast< Ref >( 0 ) );
    return r;
}

/*  Chain chain_append( Chain r, const char *str ) {                      */
/*      size_t n = strlen( str );                                           */
/*      bump( r, n );                                                       */
/*      strncpy( r->data + r->used, str, n );                               */
/*      r->used += n;                                                       */
/*      return r;                                                           */
/*  }                                                                       */

Chain chain_push( Chain r, Ref ch ) {
	r->alt_data.push_back( ch );
    return r;
}

Ref chain_pop( Chain r ) {
	if ( r->alt_data.empty() ) return static_cast< Ref >( 0 );
	Ref d = r->alt_data.back();
	r->alt_data.pop_back();
	return d;
}

Chain chain_new() {
	return new ChainClass();
}

void chain_free( Chain r ) {
	//	Deallocate contents
}

Chain chain_clear( Chain r ) {
	r->alt_data.clear();
    return r;
}

int chain_length( Chain r ) {
	return static_cast< int >( r->alt_data.size() );
}

Ref chain_index( Chain r, int n ) {
	return r->alt_data[ n ];
}

Ref *chain_index_ref( Chain r, int n ) {
	return &r->alt_data[ n ];
}

Ref *nc_chain2vector( Chain r ) {
	nullify( r );
	return r->alt_data.data();
}

Ref chain_last( Chain c ) {
	return c->alt_data.back();
}
