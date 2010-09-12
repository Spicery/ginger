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

#include "key.hpp"

Ref & fastMapData( Ref r ) {
	return RefToPtr4( r )[ MAP_DATA_OFFSET ];
}

Ref & fastMapCount( Ref r ) {
	return RefToPtr4( r )[ MAP_COUNT_OFFSET ];
}

Ref & fastMapletKey( Ref r ) {
	return RefToPtr4( r )[ MAPLET_KEY_OFFSET ];
}

Ref & fastMapletValue( Ref r ) {
	return RefToPtr4( r )[ MAPLET_VALUE_OFFSET ];
}

Ref & fastMapEntryKey( Ref r ) {
	return RefToPtr4( r )[ BUCKET_KEY_OFFSET ];
}

Ref & fastMapEntryValue( Ref r ) {
	return RefToPtr4( r )[ BUCKET_VALUE_OFFSET ];
}

Ref & fastMapEntryNext( Ref r ) {
	return RefToPtr4( r )[ BUCKET_NEXT_OFFSET ];
}
