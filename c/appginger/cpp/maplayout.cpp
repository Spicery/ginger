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

#include "maplayout.hpp"

#include "common.hpp"
#include "key.hpp"

Ref & fastMapData( Ref r ) {
	return RefToPtr4( r )[ MAP_OFFSET_DATA ];
}

Ref & fastMapCount( Ref r ) {
	return RefToPtr4( r )[ MAP_OFFSET_COUNT ];
}

Ref & fastMapletKey( Ref r ) {
	return RefToPtr4( r )[ MAPLET_KEY_OFFSET ];
}

Ref & fastMapletValue( Ref r ) {
	return RefToPtr4( r )[ MAPLET_VALUE_OFFSET ];
}

Ref & fastAssocKey( Ref r ) {
	return RefToPtr4( r )[ ASSOC_KEY_OFFSET ];
}

Ref & fastAssocValue( Ref r ) {
	return RefToPtr4( r )[ ASSOC_VALUE_OFFSET ];
}

Ref & fastAssocNext( Ref r ) {
	return RefToPtr4( r )[ ASSOC_NEXT_OFFSET ];
}

/*
	Layout of the flags field.
	
		+-----------+--------------+-----------+
		|   Bit 8   |   Bits 2-7   |  Bit 0-1  |
		+-----------+--------------+-----------+
		| Dirty Bit | Width (0-63) | 2-bit tag |
		+-----------+--------------+-----------+
*/

unsigned long fastMapWidth( Ref map ) {
	return fastMapPtrWidth( RefToPtr4( map ) );
}

unsigned long fastMapPtrWidth( Ref * map_K ) {
	unsigned long flags = SmallToLong( map_K[ MAP_OFFSET_FLAGS ] );
	return flags & 0x3F;
}

bool fastMapDirtyBit( Ref map ) {
	return fastMapPtrDirtyBit( RefToPtr4( map ) );
}

bool fastMapPtrDirtyBit( Ref * map_K ) {
	unsigned long flags = SmallToLong( map_K[ MAP_OFFSET_FLAGS ] );
	return flags & 0x40;
}

void fastSetMapDirtyBit( Ref map, bool db ) {
	fastSetMapPtrDirtyBit( RefToPtr4( map ), db );
}

void fastSetMapPtrDirtyBit( Ref * map_K, bool db ) {
	Ref * loc = &map_K[ MAP_OFFSET_FLAGS ];
	if ( db ) {
		//*( unsigned long * )( loc ) |= 0x100;
		*reinterpret_cast< unsigned long * >( loc ) |= 0x100;
	} else {
		*reinterpret_cast< unsigned long * >( loc ) &= ~0x100;
	}
}
