/******************************************************************************\
	Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
	of the GNU General Public License. This file is part of Ginger.

    Ginger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Ginger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Ginger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

//#include <iostream>
#include <vector>

//	For memcpy.
#include <string.h>	
#include <stddef.h>

#include "sysmap.hpp"


#include "sysprint.hpp"
#include "sysequals.hpp"
#include "key.hpp"
#include "machine.hpp"
#include "mishap.hpp"
#include "maplayout.hpp"
#include "misclayout.hpp"
#include "vectorlayout.hpp"
#include "maplayout.hpp"

namespace Ginger {
using namespace std;


typedef unsigned int ub4;

/* every byte is an arbitrary permutation of 0..255 */
static const ub4 gencrctab[256] = {
  0x46d1e192, 0x66edf9aa, 0x927fc9e5, 0xa53baacc, 0x29b47658, 0x5a411a01,
  0x0e66d5bd, 0x0dd5b1db, 0xcb38340e, 0x04d4ebb6, 0x98bc4f54, 0x36f20f2c,
  0x4a3047ed, 0x1ec1e0eb, 0x568c0c1f, 0x6a731432, 0x81367fc6, 0xe3e25237,
  0xe7f64884, 0x0fa59f64, 0x4f3109de, 0xf02d61f5, 0x5daec03b, 0x7f740e83,
  0x056ff2d8, 0x2026cc0a, 0x7ac2112d, 0x82c55605, 0xb0911ef2, 0xa7b88e4c,
  0x89dca282, 0x4b254d27, 0x7694a6d3, 0xd229eadd, 0x8e8f3738, 0x5bee7a55,
  0x012eb6ab, 0x08dd28c8, 0xb5abc274, 0xbc7931f0, 0xf2396ed5, 0xe4e43d97,
  0x943f4b7f, 0x85d0293d, 0xaed83a88, 0xc8f932fc, 0xc5496f20, 0xe9228173,
  0x9b465b7d, 0xfda26680, 0x1ddeab35, 0x0c4f25cb, 0x86e32faf, 0xe59fa13a,
  0xe192e2c4, 0xf147da1a, 0x67620a8d, 0x5c9a24c5, 0xfe6afde2, 0xacad0250,
  0xd359730b, 0xf35203b3, 0x96a4b44d, 0xfbcacea6, 0x41a165ec, 0xd71e53ac,
  0x835f39bf, 0x6b6bde7e, 0xd07085ba, 0x79064e07, 0xee5b20c3, 0x3b90bd65,
  0x5827aef4, 0x4d12d31c, 0x9143496e, 0x6c485976, 0xd9552733, 0x220f6895,
  0xe69def19, 0xeb89cd70, 0xc9bb9644, 0x93ec7e0d, 0x2ace3842, 0x2b6158da,
  0x039e9178, 0xbb5367d7, 0x55682285, 0x4315d891, 0x19fd8906, 0x7d8d4448,
  0xb4168a03, 0x40b56a53, 0xaa3e69e0, 0xa25182fe, 0xad34d16c, 0x720c4171,
  0x9dc3b961, 0x321db563, 0x8b801b9e, 0xf5971893, 0x14cc1251, 0x8f4ae962,
  0xf65aff1e, 0x13bd9dee, 0x5e7c78c7, 0xddb61731, 0x73832c15, 0xefebdd5b,
  0x1f959aca, 0xe801fb22, 0xa89826ce, 0x30b7165d, 0x458a4077, 0x24fec52a,
  0x849b065f, 0x3c6930cd, 0xa199a81d, 0xdb768f30, 0x2e45c64a, 0xff2f0d94,
  0x4ea97917, 0x6f572acf, 0x653a195c, 0x17a88c5a, 0x27e11fb5, 0x3f09c4c1,
  0x2f87e71b, 0xea1493e4, 0xd4b3a55e, 0xbe6090be, 0xaf6cd9d9, 0xda58ca00,
  0x612b7034, 0x31711dad, 0x6d7db041, 0x8ca786b7, 0x09e8bf7a, 0xc3c4d7ea,
  0xa3cd77a8, 0x7700f608, 0xdf3de559, 0x71c9353f, 0x9fd236fb, 0x1675d43e,
  0x390d9e9a, 0x21ba4c6b, 0xbd1371e8, 0x90338440, 0xd5f163d2, 0xb140fef9,
  0x52f50b57, 0x3710cf67, 0x4c11a79c, 0xc6d6624e, 0x3dc7afa9, 0x34a69969,
  0x70544a26, 0xf7d9ec98, 0x7c027496, 0x1bfb3ba3, 0xb3b1dc8f, 0x9a241039,
  0xf993f5a4, 0x15786b99, 0x26e704f7, 0x51503c04, 0x028bb3b8, 0xede5600c,
  0x9cb22b29, 0xb6ff339b, 0x7e771c43, 0xc71c05f1, 0x604ca924, 0x695eed60,
  0x688ed0bc, 0x3e0b232f, 0xf8a39c11, 0xbae6e67c, 0xb8cf75e1, 0x970321a7,
  0x5328922b, 0xdef3df2e, 0x8d0443b0, 0x2885e3ae, 0x6435eed1, 0xcc375e81,
  0xa98495f6, 0xe0bff114, 0xb2da3e4f, 0xc01b5adf, 0x507e0721, 0x6267a36a,
  0x181a6df8, 0x7baff0c0, 0xfa6d6c13, 0x427250b2, 0xe2f742d6, 0xcd5cc723,
  0x2d218be7, 0xb91fbbb1, 0x9eb946d0, 0x1c180810, 0xfc81d602, 0x0b9c3f52,
  0xc2ea456f, 0x1165b2c9, 0xabf4ad75, 0x0a56fc8c, 0x12e0f818, 0xcadbcba1,
  0x2586be56, 0x952c9b46, 0x07c6a43c, 0x78967df3, 0x477b2e49, 0x2c5d7b6d,
  0x8a637272, 0x59acbcb4, 0x74a0e447, 0xc1f8800f, 0x35c015dc, 0x230794c2,
  0x4405f328, 0xec2adba5, 0xd832b845, 0x6e4ed287, 0x48e9f7a2, 0xa44be89f,
  0x38cbb725, 0xbf6ef4e6, 0xdc0e83fa, 0x54238d12, 0xf4f0c1e3, 0xa60857fd,
  0xc43c64b9, 0x00c851ef, 0x33d75f36, 0x5fd39866, 0xd1efa08a, 0xa0640089,
  0x877a978b, 0x99175d86, 0x57dfacbb, 0xceb02de9, 0xcf4d5c09, 0x3a8813d4,
  0xb7448816, 0x63fa5568, 0x06be014b, 0xd642fa7b, 0x10aa7c90, 0x8082c88e,
  0x1afcba79, 0x7519549d, 0x490a87ff, 0x8820c3a0,
};

//	CRC hash from http://burtleburtle.net/bob/c/gencrc.c
static ub4 gencrc( const unsigned char * key, ub4 len ) {
	ub4 hash = len;
	for ( ub4 i = 0; i < len; ++i ) {
		hash = ( hash >> 8 ) ^ gencrctab[ ( hash & 0xff ) ^ key[ i ] ];
	}
	return hash;
}

unsigned int gngIdHash( Ref ref ) {
	return gencrc( reinterpret_cast< const unsigned char * >( &ref ), sizeof( Ref ) );
}


static long estimateMapSize( MachineClass * vm ) {
	long total = 0;
	long n = vm->count;
	for ( long i = 0; i < n; i++ ) {
		Ref r = *( vm->vp - i );
		if ( IsMaplet( r ) ) {
			total += 1;
		} else if ( IsMap( r ) ) {
			total += SmallToLong( fastMapCount( r ) );
		} else {
			throw Ginger::Mishap( "Invalid argument for newMap" );
		}
	}
	return total;
}

static const signed char LogTable256[256] = 
{
#define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
    -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
    LT(4), LT(5), LT(5), LT(6), LT(6), LT(6), LT(6),
    LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7)
};

static long log2( unsigned long v ) {
	unsigned long t, tt, w; 	// temporaries
	
	if (( w = v >> 32 )) {
		if (( tt = w >> 16 )) {
			return ( t = tt >> 8 ) ? 56 + LogTable256[ t ] : 48 + LogTable256[ tt ];
		} else {
			return ( t = w >> 8 ) ? 40 + LogTable256[ t ] : 32 + LogTable256[ w ];
		}
	} else if (( tt = v >> 16 )) {
	  	return ( t = tt >> 8 ) ? 24 + LogTable256[ t ] : 16 + LogTable256[ tt ];
	} else {
	  	return ( t = v >> 8 ) ? 8 + LogTable256[ t ] : LogTable256[ v ];
	}
}

Ref * MapCrawl::nextBucket() { 
	//std::cerr << "Updating next bucket: " << ( this->bucket != SYS_ABSENT ) << std::endl;
	if ( this->bucket != SYS_ABSENT ) {
		Ref * x = RefToPtr4( this->bucket );
		this->bucket = x[ ASSOC_OFFSET_NEXT ];
		return x;
	} else {
		//std::cerr << "Advancing: " << index_of_data << " < " << size_of_data << " ?" << std::endl;
		while ( index_of_data < size_of_data ) {
			this->bucket = this->data[ this->index_of_data++ ];
			//std::cerr << "  advance... " << ( this->bucket == SYS_ABSENT ) << std::endl;
			if ( this->bucket != SYS_ABSENT ) return this->nextBucket();
		}
		return NULL;
	}
}

bool MapCrawl::hasBeenCalled() const {
	return this->index_of_data > 0;
}


MapCrawl::MapCrawl( Ref * map_K ) :
	size_of_data( 1 << fastMapPtrWidth( map_K ) ),
	index_of_data( 0 ),
	data( RefToPtr4( map_K[1] ) + 1 ),
	bucket( SYS_ABSENT )
{}



class AddArgument {
private:
	XfrClass & xfr;
	Ref * data_refref;
	//long data_size;
	bool eq;
	long width;
	long num_buckets;
	
public:
	long numBuckets() {
		return this->num_buckets;
	}
	
private:
	unsigned long hash( Ref k ) {
		return ( this->eq ? gngEqHash( k ) : gngIdHash( k ) ) & ( ( 1 << width ) - 1 );
	}

	void addKeyValue( Ref k, Ref v ) {
		unsigned long hk = this->hash( k );
		Ref bucket0 = data_refref[ hk ];
		Ref bucket = bucket0;
		
		while ( bucket != SYS_ABSENT ) {
			Ref k1 = fastAssocKey( bucket );
			if ( refEquals( k, k1 ) ) {
				//	Overwrite.
				fastAssocValue( bucket ) = v;
				return;
			} else {
				bucket = fastAssocNext( bucket );
			}
		}
		
		xfr.setOrigin();
		xfr.xfrRef( sysAssocKey );
		xfr.xfrRef( k );
		xfr.xfrRef( v );
		xfr.xfrRef( bucket0 );
		data_refref[ hk ] = xfr.makeRef();
		
		this->num_buckets += 1;
	}

	void addMaplet( Ref r ) {
		this->addKeyValue( fastMapletKey( r ), fastMapletValue( r ) );
	}
	
	void addMap( Ref r ) {
		MapCrawl map_crawl( RefToPtr4( r ) );
		for (;;) {
			Ref * bucket = map_crawl.nextBucket();
			if ( bucket == NULL ) break;
			this->addKeyValue( bucket[ ASSOC_OFFSET_KEY ], bucket[ ASSOC_OFFSET_VALUE ] );
		}
	}

public:
	void add( Ref r ) {
		if ( IsMaplet( r ) ) {
			this->addMaplet( r );
		} else if ( IsMap( r ) ) {
			this->addMap( r );
		} else {
			throw Ginger::Mishap( "Invalid argument for newMap" );
		}
	}

	AddArgument( XfrClass & xfr, Ref * data_refref, long data_size, bool eq, long width ) :
		xfr( xfr ),
		data_refref( data_refref ),
		//data_size( data_size ),
		eq( eq ),
		width( width ),
		num_buckets( 0 )
	{
	}

};

/*
	- Takes N arguments.
	- Estimates size for an initial map by making a preliminary pass over the N arguments.
	- Builds the initial map.
	- Then adds the entries in a second pass over the N arguments.
*/
static Ref * newMap( Ref *pc, MachineClass * vm, Ref map_key ) {
	long count = estimateMapSize( vm );
	//std::cout << "Estimated size is " << count << std::endl;
	
	//	Calculate the total preflight size: we have a map record (3 long words),
	//	a data vector (2 + power of 2 greater than count), and count bucket 
	//	records (each 4 long words).
	long width = 1 + log2( count );
	long data_size = 1 << width;
	long preflight = MAP_SIZE + ( 2 + data_size ) + ( ASSOC_SIZE * count );
	//std::cout << "Estimated preflight size as " << preflight << std::endl;

	XfrClass xfr( vm->heap().preflight( pc, preflight ) );
	
	xfr.xfrRef( LongToSmall( data_size ) );
	xfr.setOrigin();
	xfr.xfrRef( sysHashMapDataKey );
	
	//	ToBeDone: We could really do with a memcpy style operation here. 
	for ( long i = 0; i < data_size; i++ ) {
		xfr.xfrRef( SYS_ABSENT );
	}
	
	Ref * data_refref = xfr.makeRefRef();
	
	xfr.setOrigin();
	xfr.xfrRef( map_key );	
	xfr.xfrRef( Ptr4ToRef( data_refref ) );
	xfr.xfrRef( LongToSmall( count ) );
	xfr.xfrRef( LongToSmall( width ) );			//	FLAGS.
	Ref map_ref = xfr.makeRef();
	
	//	Now add all the members. Need to perform the update in LEFT to RIGHT
	//	order, from vm->vp[ -( vm->count - 1 ) ] to vm->vp[ 0 ]
	AddArgument args( xfr, data_refref + 1, data_size, MapKeyEq( map_key ), width );
	for ( long i = vm->count - 1; i >= 0; i-- ) {
		Ref r = *( vm->vp - i );
		args.add( r );
	}
	
	RefToPtr4( map_ref )[ MAP_OFFSET_COUNT ] = LongToSmall( args.numBuckets() );
	
	vm->vp -= vm->count;
	*( ++vm->vp ) = map_ref;
	
	return pc;
}

Ref * sysNewHardEqMap( Ref *pc, MachineClass * vm ) {
	return newMap( pc, vm, sysHardEqMapKey );
}

Ref * sysNewWeakIdMap( Ref *pc, MachineClass * vm ) {
	return newMap( pc, vm, sysWeakIdMapKey );
}

Ref * sysNewHardIdMap( Ref *pc, MachineClass * vm ) {
	return newMap( pc, vm, sysHardIdMapKey );
}

Ref * sysNewCacheEqMap( Ref *pc, MachineClass * vm ) {
	return newMap( pc, vm, sysCacheEqMapKey );
}

Ref * sysMapValues( Ref *pc, class MachineClass * vm ) {
	if ( vm->count != 1 ) throw Ginger::Mishap( "ArgsMismatch" );
	Ref r = vm->fastPop();
	if ( !IsMap( r ) ) throw Ginger::Mishap( "Map needed" ).culprit( "Object", refToShowString( r ) );
	Ref * map_K = RefToPtr4( r );
	long len = SmallToLong( map_K[ MAP_OFFSET_COUNT ] );
	vm->checkStackRoom( len );
	
	MapCrawl map_crawl( map_K );
	for (;;) {
		Ref * bucket_K = map_crawl.nextBucket();
		if ( bucket_K == NULL ) break;
		Ref v = bucket_K[ ASSOC_OFFSET_VALUE ];
		vm->fastPush( v );
	}
	
	return pc;
}

Ref * sysMapExplode( Ref *pc, class MachineClass * vm ) {
	if ( vm->count != 1 ) throw Ginger::Mishap( "ArgsMismatch" );
	Ref r = vm->fastPeek();		// Don't remove from stack yet, may need to GC.
	if ( !IsMap( r ) ) throw Ginger::Mishap( "Map needed" ).culprit( "Object", refToShowString( r ) );

	//	See if we need a garbage collection.
	long count = SmallToLong( fastMapCount( r ) );
	XfrClass xfr( vm->heap().preflight( pc, count * MAPLET_SIZE ) );

	//	Now we can remove the map from the stack.
	vm->fastPop();
	
	//	And make sure we have enough room on the stack for the maplets.
	vm->checkStackRoom( count );
	
	Ref * map_K = RefToPtr4( r );
	MapCrawl map_crawl( map_K );
	for (;;) {
		Ref * bucket_K = map_crawl.nextBucket();
		if ( bucket_K == NULL ) break;

		xfr.setOrigin();
        xfr.xfrRef( sysMapletKey );
        Ref k = bucket_K[ ASSOC_OFFSET_KEY ];
        Ref v = bucket_K[ ASSOC_OFFSET_VALUE ];
        xfr.xfrRef( k );
        xfr.xfrRef( v );
        vm->fastPush( xfr.makeRef() );
	}
	
	return pc;
}


Ref * sysMapIndex( Ref * pc, class MachineClass * vm ) {
	if ( vm->count != 2 ) throw Ginger::Mishap( "ArgsMismatch" );
	const Ref map = vm->fastPop();
	const Ref idx = vm->fastPop();
	
	if ( !IsMap( map ) ) throw Ginger::Mishap( "Map needed" ).culprit( "Object", refToShowString( map ) );
	Ref * map_K = RefToPtr4( map );
	const Ref map_key = *map_K;
	
	const long width = fastMapPtrWidth( map_K );
	const bool eq = MapKeyEq( map_key );
	//{
	//	std::cout << "> KEY " << map_key << "; EQ  " << MapKeyEq( map_key ) << std::endl;
	//}

	const unsigned long hk = ( eq ? gngEqHash( idx ) : gngIdHash( idx ) ) & ( ( 1 << width ) - 1 );
	const Ref data = map_K[ MAP_OFFSET_DATA ];
	Ref bucket = RefToPtr4( data )[ hk + 1 ];
	
	while ( bucket != SYS_ABSENT ) {
		Ref k1 = fastAssocKey( bucket );
		if ( eq ? refEquals( idx, k1 ) : idx == k1 ) {
			//	Found it.
			vm->fastPush( fastAssocValue( bucket ) );
			return pc;
		} else {
			bucket = fastAssocNext( bucket );
		}
	}
	
	vm->fastPush( SYS_ABSENT );
	return pc;
}

static const char * MAP_OPEN = "{";
static const char * MAP_CLOSE = "}";
static const char * MAP_ARROW = "=>";

void gngPrintMapPtr( std::ostream & out, Ref * map_K ) {
	MapCrawl map_crawl( map_K );
	bool sep = false;
	out << MAP_OPEN;
	for (;;) {
		Ref * bucket_K = map_crawl.nextBucket();
		if ( bucket_K == NULL ) break;
		if ( sep ) { out << ","; } else { sep = true; }
		Ref k = bucket_K[ ASSOC_OFFSET_KEY ];
		Ref v = bucket_K[ ASSOC_OFFSET_VALUE ];
		refPrint( out, k );
		out << MAP_ARROW;
		refPrint( out, v );
	}
	out << MAP_CLOSE;
}


#define ASSERT( x ) 			if ( not( x ) ) throw;
#define ASSERT_EQUAL( x, y )	if ( x != y ) throw;

static int verifyAssocChain( Ref chain ) {
	int count = 0;
	for (;;) {
		ASSERT( chain == SYS_ABSENT || IsAssoc( chain ) );
		if ( chain == SYS_ABSENT ) break;
		count += 1;
		chain = fastAssocNext( chain );
	}
	return count;
}

static void verifyMapIntegrity( Ref map ) {
	ASSERT( IsMap( map ) );
	Ref * map_K = RefToPtr4( map );
	ASSERT_EQUAL( sysHardIdMapKey, map_K[ 0 ] );
	Ref data = map_K[ MAP_OFFSET_DATA ];
	ASSERT( IsVector( data ) );
	Ref * data_K = RefToPtr4( data );
	int bit_width = fastMapPtrWidth( map_K );
	int length = SmallToLong( data_K[ VECTOR_OFFSET_LENGTH ] );
	
	//cout << "length = " << length << endl;
	//cout << "bit_width = " << bit_width << endl;

	ASSERT_EQUAL( length, 1 << bit_width );
	
	int sofar = 0;
	for ( int n = 1; n <= length; n++ ) {
		Ref chain = data_K[ n ];
		sofar += verifyAssocChain( chain );
	}
	
	long count = SmallToLong( map_K[ MAP_OFFSET_COUNT ] );
	ASSERT_EQUAL( count, (long)sofar );
}

/**
	gngRehashMapPtr has the job of rehashing identity based hash maps 
	after a garbage collection. In the case of a weak map, it also has to
	adjust the count, as that may have become smaller.
*/
void gngRehashMapPtr( Ref * map_K ) {
	const int width = fastMapPtrWidth( map_K );
	unsigned long mask = ( 1L << width ) - 1;
	
	Ref data = map_K[ MAP_OFFSET_DATA ];
	Ref * data_K = RefToPtr4( data );
	const long data_length = SmallToLong( data_K[ VECTOR_OFFSET_LENGTH ] );
	//std::cout << "REHASHING!!!! length " << data_length << "; width " << width << std::endl;
	
	//	Fill vdata with SYS_ABSENT.
	vector< Ref > vdata( data_length, SYS_ABSENT );
	
	long count = 0;
	
	for ( long i = 0; i < data_length; i++ ) {
		Ref & chain = data_K[ i + 1 ];
		//std::cout << "[" << i << "] " << chain << endl; 
		while ( chain != SYS_ABSENT ) {
			Ref assoc = chain;
			Ref lhs = fastAssocKey( chain );
			chain = fastAssocNext( chain );
			
			unsigned long hash = gngIdHash( lhs ) & mask;
			
			//cout << "Rehashing " << lhs << endl;
			//cout << "Moving from " << i << " to " << hash << endl;
			
			fastAssocNext( assoc ) = vdata[ hash ];
			vdata[ hash ] = assoc;
			count += 1;
		}
	}	
	
	//for  ( long i = 0; i < data_length; i++ ) {
	//	data_K[ i + 1 ] = vdata[ i ];
	//}
	
	memcpy( data_K + 1, &vdata[0], sizeof( Ref ) * data_length );
	
	const long count_current = SmallToLong( map_K[ MAP_OFFSET_COUNT ] );
	if ( map_K[0] == sysWeakIdMapKey && count < count_current ) {
		map_K[ MAP_OFFSET_COUNT ] = LongToSmall( count );
	} else if ( count != count_current ) {
		throw SystemError( "Map count differs after GC" );
	}
	
	if ( SAFE_MODE ) {
		Ref r = Ptr4ToRef( map_K );
		verifyMapIntegrity( r );
	}
}

} // namespace Ginger

