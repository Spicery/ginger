/******************************************************************************\
	Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
	of the GNU General Public License. This file is part of AppGinger.

    AppGinger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AppGinger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AppGinger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

#include "sysmap.hpp"

#include "sysequals.hpp"
#include "key.hpp"
#include "machine.hpp"
#include "mishap.hpp"

static inline Ref & FastMapData( Ref r ) {
	return ObjToPtr4( r )[ 1 ];
}

static inline Ref & FastMapCount( Ref r ) {
	return ObjToPtr4( r )[ 2 ];
}

static inline Ref & FastMapletKey( Ref r ) {
	return ObjToPtr4( r )[ 1 ];
}

static inline Ref & FastMapletValue( Ref r ) {
	return ObjToPtr4( r )[ 2 ];
}

static inline Ref & FastMapEntryKey( Ref r ) {
	return ObjToPtr4( r )[ 1 ];
}

static inline Ref & FastMapEntryValue( Ref r ) {
	return ObjToPtr4( r )[ 2 ];
}

static inline Ref & FastMapEntryNext( Ref r ) {
	return ObjToPtr4( r )[ 3 ];
}



static long estimateMapSize( MachineClass * vm ) {
	long total = 0;
	long n = vm->count;
	for ( long i = 0; i < n; i++ ) {
		Ref r = *( vm->vp - i );
		if ( IsMaplet( r ) ) {
			total += 1;
		} else if ( IsMap( r ) ) {
			total += SmallToLong( FastMapCount( r ) );
		} else {
			throw Mishap( "Invalid argument for newMap" );
		}
	}
	return total;
}

static const char LogTable256[256] = 
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


class AddArgument {
public:
	XfrClass & xfr;
	Ref * data_refref;
	long data_size;
	long width;
	
	void addMaplet( Ref r ) {
		this->addMaplet( r );
		Ref k = FastMapletKey( r );
		unsigned long hk = refHash( k ) & ~( 1 << width - 1 );
		Ref bucket = data_refref[ hk ];
		
		while ( bucket != sys_absent ) {
			Ref k1 = FastMapEntryKey( bucket );
			if ( refEquals( k, k1 ) ) {
				//	Overwrite.
				FastMapEntryValue( bucket ) = FastMapletValue( r );
			} else {
				bucket = FastMapEntryNext( bucket );
			}
		}
		
		xfr.setOrigin();
		xfr.xfrRef( sysMapEntryKey );
		xfr.xfrRef( k );
		xfr.xfrRef( FastMapletValue( r ) );
		xfr.xfrRef( bucket );
		data_refref[ hk ] = xfr.makeRef();
	}
	
	void addMap( Ref r ) {
		throw ToBeDone();
	}

	void add( Ref r ) {
		if ( IsMaplet( r ) ) {
			this->addMaplet( r );
		} else if ( IsMap( r ) ) {
			this->addMap( r );
		} else {
			throw Mishap( "Invalid argument for newMap" );
		}
	}

	AddArgument( XfrClass & xfr, Ref * data_refref, long data_size, long width ) :
		xfr( xfr ),
		data_refref( data_refref ),
		data_size( data_size ),
		width( width )
	{
	}

};

/*
	- Takes N arguments.
	- Estimates size for an initial map by making a preliminary pass 
	over the N arguments.
	- Builds the initial map.
	- Then adds the entries in a second pass over the N arguments.
*/
Ref * sysNewMap( Ref *pc, MachineClass * vm ) {
	long count = estimateMapSize( vm );
	std::cout << "Estimated size is " << count << std::endl;
	
	//	Calculate the total preflight size: we have a map record (3 long words),
	//	a data vector (2 + power of 2 greater than count), and count bucket 
	//	records (each 4 long words).
	long width = 1 + log2( count );
	long data_size = 1 << width;
	long preflight = 3 + ( 2 + data_size ) + ( 4 * count );
	std::cout << "Estimated preflight size as " << preflight << std::endl;

	XfrClass xfr( vm->heap().preflight( pc, preflight ) );
	
	xfr.xfrRef( LongToSmall( data_size ) );
	xfr.setOrigin();
	xfr.xfrRef( sysVectorKey );
	for ( long i = 0; i < data_size; i++ ) {
		xfr.xfrRef( sys_absent );
	}
	Ref * data_refref = xfr.makeRefRef();
	
	xfr.setOrigin();
	xfr.xfrRef( sysMapKey );	
	xfr.xfrRef( Ptr4ToRef( data_refref ) );
	xfr.xfrRef( LongToSmall( count ) );
	Ref map_ref = xfr.makeRef();
	
	//	Now add all the members. Need to perform the update in LEFT to RIGHT
	//	order, from vm->vp[ -( vm->count - 1 ) ] to vm->vp[ 0 ]
	AddArgument args( xfr, data_refref, data_size, width );
	for ( long i = vm->count - 1; i >= 0; i-- ) {
		Ref r = *( vm->vp - i );
		args.add( r );
	}
	
	vm->vp -= vm->count;
	*( ++vm->vp ) = map_ref;
	
	return pc;
}