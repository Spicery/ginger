#include <string.h>
#include <stdlib.h>


#include <stdio.h>

//	debugging only
//	#include <unistd.h>
//	#include <sys/mman.h>


#include "common.hpp"
#include "mishap.hpp"
#include "pool.hpp"
#include <string>
using namespace std;

typedef unsigned int unint;

class PolicyMethods {
public:
    void (*init)( Pool pool );
    void (*fini)( Pool pool );
	void (*show)( Pool pool, int detail );
    Ref (*allocate)( Pool pool, unint size );
    Ref (*reallocate)( Pool pool, Ref allocatedThing, unint newSize );
    void (*deallocate)( Pool pool, Ref thingToFree );
};

class PoolClass {
public:
    PolicyMethods   *class_ptr;
    enum PoolPolicy policy;
    enum PoolChecks checks;
    string			name;
    Pool           	parent;
    Pool           	prev;
    Pool           	next;
    Cell            private_cell;
    Block           data;
};


//------------------------------------------------------------------------

static char *sstrdup( const char *s ) {
    char *t = strdup( s );
    if ( t == NULL ) mishap( "Memory exhausted (strdup)" );
    return t;
}

static Block smalloc( unint size ) {
    Block r = (Block)malloc( size );
    if( r == NULL ) mishap( "Memory exhausted (malloc)" );
    return r;
}

/*
static Block smalloc( size_t size ) {
	size_t pg = getpagesize();
	char *data = (char *)malloc( size + pg + pg );
	char *d;
	//	Now page-align top end - assuming page size a power of 2.
	d = data + size;
	d = (char *)(((unsigned int)( d + ( pg - 1 ) )) & ~( pg - 1 ));
	mprotect( d, pg, PROT_READ );
	return (Block)( d - size );
}
*/

static Block srealloc( Block thing, unint size ) {
    Block r = (Block)realloc( thing, size );
    if( r == NULL ) mishap( "Memory exhausted (malloc)" );
    return r;
}

static void sfree( Block thing ) {
    free( thing );
}

//  Convert a size in bytes to a size in words.
static unint size_in_cells( unint size ) {
    return ( size + sizeof( Cell ) - 1 ) / sizeof( Cell );
}


//------------------------------------------------------------------------

#define VSIZE 	30
#define VSIZE1  ( VSIZE - 1 )

enum VanillaLayout {
    VanillaNext,
    VanillaCapacity,
    VanillaAvailable,
    VanillaHeaderSize
};

//  A negative length denotes freed store.
enum {
    VanillaLength,
    VanillaOverhead
};

static void vanilla_show( Pool pool, int detail ) {
	Block d = pool->data;
	int n = 0;
	while ( d != NULL ) {
		unint capacity  = d[ VanillaCapacity ].uintpart;
		unint available = d[ VanillaAvailable ].uintpart;
		n += 1;
#ifdef DBG_POOL
		printf( " // segment %d \\\\\n", n );
		printf( "    capacity : %u\n", capacity );
		printf( "    available: %u\n", available );
		printf( "    blocks   :" );
#endif
		{
			const char *gap = " ";
			Block item = d + VanillaHeaderSize;
			int stuff = capacity - available;
			while ( stuff > 0 ) {
				int s = item[ VanillaLength ].intpart + VanillaOverhead;
#ifdef DBG_POOL
				printf( "%s%d", gap, s );
#endif				
				gap = ", ";
				item += s;
				stuff -= s;
			}
		}
#if DGB_POOL
		printf( "\n" );
#endif
		d = d[ VanillaNext ].blockpart;
	}
	printf( " ---------------------\n" );
}

static Block new_vanilla_segment( Block next, unint ncells, bool exact ) {
	unint actual_size;
    Block r;
#ifdef DBG_POOL
	printf( "Grab new segment\n" );
#endif
    actual_size = ncells + VanillaHeaderSize + VanillaOverhead;
    if ( !exact ) actual_size = ( ( actual_size + VSIZE1 ) / VSIZE ) * VSIZE;
#ifdef DBG_POOL
	printf( "Allocating segment of %d cells (exact = %s)\n", actual_size, exact ? "true" : "false" );
#endif
    r = smalloc( actual_size * sizeof( Cell ) );
    r[ VanillaNext ].blockpart = next;
    r[ VanillaCapacity ].uintpart =
    	r[ VanillaAvailable ].uintpart =
			actual_size - VanillaHeaderSize;
    return r;
}

static void vanilla_init( Pool pool ) {
    //  Nothing.
}

static void vanilla_fini( Pool pool ) {
    //  Nothing.
}


static Ref vanilla_allocate( Pool pool, unint nbytes ) {
    Block block;
    unint ncells = size_in_cells( nbytes );
    unint ncells1 = ncells + VanillaOverhead;
    Block data = pool->data;
    unint a = ( data == NULL ? 0 : data[ VanillaAvailable ].uintpart );
	// printf( "Available = %d, required = %d\n", a, ncells1 );
    if ( a >= ncells1 ) {
        data[ VanillaAvailable ].uintpart -= ncells1;
        block = data + ( VanillaHeaderSize + data[ VanillaCapacity ].uintpart - a );
        block[ VanillaLength ].intpart = (int)ncells;
		// printf( "Remaining = %u\n", data[ VanillaAvailable ].uintpart );
		// printf( "Returning %x\n", ToUInt( block + VanillaOverhead ) );
        return block + VanillaOverhead;
    } else if ( ncells >= VSIZE ) {
#ifdef DBG_POOL
		printf( "Allocating oversized object (%d)\n", ncells );
#endif		
		{
        Block s = data[ VanillaNext ].blockpart = new_vanilla_segment( data[ VanillaNext ].blockpart, ncells, true );
        block = s + VanillaHeaderSize;
        block[ VanillaLength ].intpart = (int)ncells;
        return block + VanillaOverhead;
		}
    } else {
#ifdef DBG_POOL
		printf( "Exhausted current pool, allocating new pool\n" );
#endif
        pool->data = new_vanilla_segment( data, ncells, false );
#ifdef DBG_POOL
		pool_show( pool, 999 );
#endif
        return vanilla_allocate( pool, nbytes );
    }
}

static void vanilla_deallocate( Pool pool, Ref thing ) {
    Block t = (Block)thing- VanillaOverhead;
    t[ VanillaLength ].intpart *= -1;
}

static Ref vanilla_reallocate( Pool pool, Ref thing, unint nbytes ) {
    unint length;
    unint ncells;

    Block thung = (Block)thing - VanillaOverhead;
    length = thung[ VanillaLength ].intpart;
    ncells = size_in_cells( nbytes );
    if ( length > ncells ) {
        thung[ VanillaLength ].intpart = ncells;
        thung[ VanillaOverhead + ncells ].intpart = -( length - ncells );
        return thung;
    } else if ( length < ncells ) {
        //
        //  If we were doing this properly, at this point we would
        //  find out which segment "thung" comes from and whether or
        //  not there is room to expand into free/freed space.
        //
        //  This also has the benefits of detecting whether or not
        //  the thung was allocated from this pool.
        //
        //  However, in the interests of getting going, we are simply
        //  going to assume the worst-case.
        //
        Block block = (Block)vanilla_allocate( pool, nbytes );
        memcpy( block, thung, length );
        vanilla_deallocate( pool, thung );
        return block;
    } else {
        return thung;
    }
}

PolicyMethods vanilla_pool_policy =
    {
        vanilla_init,
        vanilla_fini,
		vanilla_show,
        vanilla_allocate,
        vanilla_reallocate,
        vanilla_deallocate
    };

//------------------------------------------------------------------------

#define LSIZE 	32
#define LSIZE1  ( LSIZE - 1 )

enum LiteLayout {
    LiteNext,
    LiteCapacity,
    LiteAvailable,
    LiteHeaderSize
};

static Block new_lite_segment( Block next, unint ncells, bool exact ) {
    Block r;
	unint actual_size = ncells + LiteHeaderSize;
    if ( !exact ) actual_size = ( ( actual_size + LSIZE1 ) / LSIZE ) * LSIZE;
    r = smalloc( actual_size * sizeof( Cell ) );
    r[ LiteNext ].blockpart = next;
    r[ LiteCapacity ].uintpart =
    	r[ LiteAvailable ].uintpart =
			actual_size - LiteHeaderSize;
    return r;
}

static void lite_init( Pool pool ) {
    //  Nothing.
}

static void lite_fini( Pool pool ) {
    //  Nothing.
}

static void lite_show( Pool pool, int detail ) {
	//	deferred
}

static Ref lite_allocate( Pool pool, unint nbytes ) {
    Block data;
    unint a;
    unint ncells = size_in_cells( nbytes );
    data = pool->data;
    a = ( data == NULL ? 0 : data[ LiteAvailable ].uintpart );
    if ( a >= ncells ) {
        data[ LiteAvailable ].uintpart -= ncells;
        return data + ( LiteHeaderSize + data[ LiteCapacity ].uintpart - a );
    } else if ( ncells >= LSIZE ) {
        Block s = data[ LiteNext ].blockpart = new_lite_segment( data[ LiteNext ].blockpart, ncells, true );
        return s + LiteHeaderSize;
    } else {
        pool->data = new_lite_segment( data, ncells, false );
        return lite_allocate( pool, nbytes );
    }
}

static Ref lite_reallocate( Pool pool, Ref thing, unint nbytes ) {
    mishap( "Reallocating a lightweight pool (%s)", pool->name.c_str() );
    return NULL;    //  Just to suppress the compiler complaints.
}

static void lite_deallocate( Pool pool, Ref thing ) {
    //  Nothing.  We do not store enough info to deallocate
    //  anything whatsoever.
}

PolicyMethods lite_pool_policy =
    {
		lite_init,
		lite_fini,
		lite_show,
		lite_allocate,
		lite_reallocate,
		lite_deallocate
	}
;


//------------------------------------------------------------------------

static void raw_init( Pool pool ) {
    pool->private_cell.intpart = 0;
}

static void raw_fini( Pool pool ) {
    if ( pool->private_cell.intpart != 0 ) {
        warning(
			"RAW_FINI: Space leak in pool %s (%d blocks outstanding)\n",
			pool->name.c_str(),
			pool->private_cell.intpart
		);
    }
}

static void raw_show( Pool pool, int detail ) {
	//	deferred
}

static Ref raw_allocate( Pool pool, unint nbytes ) {
    pool->private_cell.intpart += 1;
    return (Block)smalloc( nbytes );
}

static Ref raw_reallocate( Pool pool, Ref thing, unint nbytes ) {
    return (Ref)srealloc( (Block)thing, nbytes );
}

static void raw_deallocate( Pool pool, Ref thing ) {
    pool->private_cell.intpart -= 1;
    sfree( (Block)thing );
}

static PolicyMethods raw_pool_policy =
    {
		raw_init,
		raw_fini,
		raw_show,
		raw_allocate,
		raw_reallocate,
		raw_deallocate
	}
;


//------------------------------------------------------------------------

static PoolClass prime_pool =
    {
        &raw_pool_policy,
        RawPoolPolicy,
        FullPoolChecks,
        "RawPool",
        NULL, NULL, NULL,
        { NULL },
        NULL
    };

Pool pool_raw = &prime_pool;

//------------------------------------------------------------------------

void pool_set_default_policy( enum PoolPolicy policy, enum PoolChecks checks ) {
    if ( policy != DefaultPoolPolicy ) prime_pool.policy = policy;
    if ( checks != DefaultPoolChecks ) prime_pool.checks = checks;
}

//------------------------------------------------------------------------

Pool pool_renew( Pool pool ) {
	Pool r = pool_clone( pool );
	delete pool;
	return r;
}

Pool pool_clone( Pool pool ) {
	const char *name = pool->name.c_str();
	Pool parent = pool->parent;
	enum PoolPolicy policy = pool->policy;
	enum PoolChecks checks = pool->checks;
	return pool_new( name, parent, policy, checks );
}

Pool pool_new( const char *name, Pool parent, enum PoolPolicy policy, enum PoolChecks checks ) {
    Pool pool = new PoolClass(); //(Pool )malloc( sizeof( Pool ) );
    if ( pool == NULL ) mishap( "Memory exhausted (pool allocation)" );
    pool->name = sstrdup( name == NULL ? "<anon>" : name );
    pool->parent = parent;
    pool->data = NULL;

    pool->policy = ( policy = policy == DefaultPoolPolicy ? prime_pool.policy : policy );
    pool->checks = checks == DefaultPoolChecks ? prime_pool.checks : checks;

    pool->class_ptr =
        policy == LitePoolPolicy ? &lite_pool_policy :
        policy == RawPoolPolicy ? &raw_pool_policy :
        &vanilla_pool_policy;

    //  Now invoke the -init- function.
    pool->class_ptr->init( pool );

    //  Now patch into the chain.
    pool->prev = &prime_pool;
    pool->next = prime_pool.next;
    if ( pool->next != NULL ) pool->next->prev = pool;
    prime_pool.next = pool;

    return pool;
}

void pool_free( Pool pool ) {

    //  This pool can never be freed - but it is not a mistake to
    //  try to free it.
    if ( pool == pool_raw ) return;

    //  Remove from the pool chain.
    {
        Pool before = pool->prev;
        Pool after  = pool->next;
        before->next = after;
        if ( after != NULL ) after->prev  = before;
    }

    //  Invoke the -fini- function.
    pool->class_ptr->fini( pool );

    //  Free all the immediate children (implies recursive deletion).
    //  You have to be very careful here about how the chain is
    //  walked to avoid trashing the pointer into the chain.  In this
    //  case I just repeatedly try to scan the chain - one could try
    //  working with a hi-tide mark but that would require a bit more
    //  effort.
    {
        bool seen;
        do {
            Pool p = prime_pool.next;
            seen = false;
            while ( p != NULL ) {
                if ( p->parent == pool ) {
                    seen = true;    //  Arrange for re-scan.
                    pool_free( p );
                    //  p is now trashed so we have to break and re-scan.
                    break;
                }
                p = p->next;
            }
        //  Only quit if we've got all the way through cleanly.
        } while ( seen );
    }


    //  Free all the data segments.
    {
        Block ptr = pool->data;
        while ( ptr != NULL ) {
            Block next = ptr->blockpart;    //  data areas are chained on the 0th element
            free( ptr );
            ptr = next;
        }
    }

    //  Free the pool object.
    delete pool;
}



//------------------------------------------------------------------------


Ref pool_malloc( Pool pool, unint size ) {
    Ref r = pool->class_ptr->allocate( pool, size );
	//	printf( "pool malloc (%x)\n", ToUInt( r ) );
	return r;
}

Ref pool_realloc( Pool pool, Ref thing, unint size ) {
	Ref r =
 	    thing == NULL ?
        pool->class_ptr->allocate( pool, size ) :
        pool->class_ptr->reallocate( pool, thing, size );
	return r;
}

void pool_dealloc( Pool pool, Ref thing ) {
    if ( thing != NULL ) pool->class_ptr->deallocate( pool, thing );
}

char *pool_strdup( Pool pool, const char *str ) {
    char *result = (char *)pool_malloc( pool, strlen( str ) + 1 );
    return strcpy( result, str );
}


void pool_show( Pool pool, int detail ) {
	if ( detail > 0 ) {
	    enum PoolPolicy p = pool->policy;
	    if ( detail >= 0 ) {
		    printf( "POOL %s\n", pool->name.c_str() );
		    printf( "    parent: %s\n", pool->parent ? pool->parent->name.c_str() : "<root>" );
		    printf(
			    "    policy: %s\n",
                p == DefaultPoolPolicy ? "default (eh?)" :
                p == VanillaPoolPolicy ? "vanilla" :
                p == LitePoolPolicy ? "lite" :
			    p == RawPoolPolicy ? "raw" :
			    "<broken>"
		    );
	    }
	}
	if ( detail > 1 ) {
		pool->class_ptr->show( pool, detail );
	}
}

//------------------------------------------------------------------------
