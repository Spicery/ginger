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

#ifndef OBJ_LAYOUT_HPP
#define OBJ_LAYOUT_HPP

/******************************************************************************\
|
|	Object Layout describes the different kinds of objects in the system
|	and encapsulates the information about how they are formatted in 
|	memory. See the spreadsheet object_layout.ods for a precise description
|	of what goes where.
|
|	The different kinds of objects in the heap are:
|		Full records
|		Full vectors
|		Strings (immutable)
|		Function-objects
|
\******************************************************************************/

#include "common.hpp"

////////////////////////////////////////////////////////////////////////////////
//	Generic layout parameters
////////////////////////////////////////////////////////////////////////////////


#define MAX_OFFSET_FROM_START_TO_KEY	4

////////////////////////////////////////////////////////////////////////////////
//	Full records
////////////////////////////////////////////////////////////////////////////////

unsigned long sizeAfterKeyOfRecord( Ref * key );



////////////////////////////////////////////////////////////////////////////////
//	Full vectors
////////////////////////////////////////////////////////////////////////////////

unsigned long sizeAfterKeyOfVector( Ref * key );


////////////////////////////////////////////////////////////////////////////////
//	Strings
////////////////////////////////////////////////////////////////////////////////


unsigned long sizeAfterKeyOfString( Ref * key );

////////////////////////////////////////////////////////////////////////////////
//	Objects
////////////////////////////////////////////////////////////////////////////////

void findObjectLimits( Ref * object, Ref * & start, Ref * & end );

////////////////////////////////////////////////////////////////////////////////
//	Function Objects
////////////////////////////////////////////////////////////////////////////////

// field before the function key.
#define OFFSET_TO_NSLOTS_FROM_PC			-2

#define OFFSET_FROM_FN_LENGTH_TO_KEY		4

unsigned long sizeAfterKeyOfFnLength( Ref * key );




#endif