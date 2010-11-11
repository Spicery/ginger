#ifndef POOL_H
#define POOL_H

#include "common.hpp"

enum PoolChecks {
    DefaultPoolChecks,
    FullPoolChecks,
    NoPoolChecks
};

//	Vanilla: 	all capabilities enabled
//		- each segment includes a 1-cell overhead.
//		- however the implementation of reallocation is poor.
//		- double alignment neglected.
//		
//	Lite:		deallocation + reallocation disabled
//		- very lightweight, no overhead for each segment
//
//	Raw:		pool_free, pool_clear disabled
//		- a wrapper for malloc/realloc/free.


enum PoolPolicy {
    DefaultPoolPolicy,	
    VanillaPoolPolicy,  //	Every segment has a length header - all capabilities enabled.
    LitePoolPolicy,		//	The lightweight option - cannot implement dellocation.
    RawPoolPolicy       //	A wrapper for malloc/realloc/free - cannot implement pool_clear or pool_free.
};


typedef union Cell {
    void            *refpart;
    int             intpart;
    unsigned int    uintpart;
    union Cell      *blockpart;
} Cell;

typedef Cell *Block;

typedef class PoolClass *Pool;

Pool pool_renew( Pool pool );
Pool pool_clone( Pool pool );
Pool pool_new( const char *name, Pool parent, enum PoolPolicy, enum PoolChecks );
void pool_free( Pool pool );

//	Ref pool_allocate( Pool pool, unsigned int size );
//	Ref pool_reallocate( Pool pool, Ref thing, unsigned int size );
//	void pool_deallocate( Pool pool, Ref thing );

Ref pool_malloc( Pool pool, unsigned int size );
Ref pool_realloc( Pool pool, Ref thing, unsigned int size );
void pool_dealloc( Pool pool, Ref thing );

char *pool_strdup( Pool pool, const char * );

void pool_show( Pool pool, int detail );

void pool_set_default_policy( enum PoolPolicy, enum PoolChecks );

extern Pool pool_raw;

#endif
