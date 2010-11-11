#ifndef CHAIN_H
#define CHAIN_H

#include "common.hpp"

typedef class ChainClass *Chain;

Chain chain_new();
void chain_free( Chain c );
Chain chain_push( Chain c, Ref ch );
Ref chain_pop( Chain c );
Chain chain_clear( Chain c );
int chain_length( Chain c );
Ref chain_index( Chain c, int n );
Ref *chain_index_ref( Chain c, int n );
Ref *nc_chain2vector( Chain c );
Ref chain_last( Chain c );

#endif
