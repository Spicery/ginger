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

#ifndef MAP_LAYOUT_HPP
#define MAP_LAYOUT_HPP

#include "common.hpp"

/*
+-------------+-------------+----------+
|  sysMapKey  | AssocChain  |  Count   |
+-------------+-------------+----------+
*/

#define MAP_DATA_OFFSET 		1
#define MAP_COUNT_OFFSET 		2
#define MAP_SIZE				3

/*
+----------------+-------------+-------------+
|  sysMapletKey  |  LHS (Key)  | RHS (Value) |
+----------------+-------------+-------------+
*/

#define MAPLET_KEY_OFFSET 		1
#define MAPLET_VALUE_OFFSET 	2
#define MAPLET_SIZE				3

/*
+---------------+-------------+-------------+----------+
|  sysAssocKey  |  LHS (Key)  | RHS (Value) |   Next   |
+---------------+-------------+-------------+----------+
*/

#define ASSOC_KEY_OFFSET 		1
#define ASSOC_VALUE_OFFSET 		2
#define ASSOC_NEXT_OFFSET 		3
#define ASSOC_SIZE				4

extern Ref & fastMapData( Ref r );
extern Ref & fastMapCount( Ref r );
extern Ref & fastMapletKey( Ref r );
extern Ref & fastMapletValue( Ref r );
extern Ref & fastAssocKey( Ref r );
extern Ref & fastAssocValue( Ref r );
extern Ref & fastAssocNext( Ref r );

#endif
