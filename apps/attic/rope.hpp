#ifndef ROPE_H
#define ROPE_H

#include "pool.hpp"

typedef class RopeClass *Rope;

Rope rope_new( Pool pool, const char *text );
void rope_free( Rope r );
Rope rope_push( Rope r, char ch );
char rope_pop( Rope r );
Rope rope_clear( Rope r );
char *rope2str( Rope r );
char *nc_rope2str( Rope r );
Rope rope_append( Rope r, const char *str );
int rope_length( Rope r );
char rope_index( Rope r, int n );

#endif
