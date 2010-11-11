#include "hash.hpp"

unsigned int hash_function( const char *key ) {
    unsigned int hashvalue = 0;
    const char *cp;

    for ( cp = key; *cp; cp++ ) {
        hashvalue = ( hashvalue << 2 ) ^ ( ( int )( *cp ) );
    }

    return hashvalue;
}
