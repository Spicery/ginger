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
