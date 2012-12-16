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

#ifndef MISC_LAYOUT_HPP
#define MISC_LAYOUT_HPP

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
#include "functionlayout.hpp"

////////////////////////////////////////////////////////////////////////////////
//	Generic layout parameters
////////////////////////////////////////////////////////////////////////////////

//	This is a hard limit on the numbers of fields that may be set to the 
//	left of the key.
#define MAX_OFFSET_FROM_START_TO_KEY	5


////////////////////////////////////////////////////////////////////////////////
//  Full records
////////////////////////////////////////////////////////////////////////////////

extern unsigned long sizeAfterKeyOfRecordLayout( Ref * key );
extern unsigned long lengthOfRecordLayout( Ref * key );

////////////////////////////////////////////////////////////////////////////////
//  Word records
////////////////////////////////////////////////////////////////////////////////

extern unsigned long sizeAfterKeyOfWRecordLayout( Ref * key );
extern unsigned long lengthOfWRecordLayout( Ref * key );


/*
////////////////////////////////////////////////////////////////////////////////
//	Full vectors
////////////////////////////////////////////////////////////////////////////////

//	Deprecate this in favour of VECTOR_LAYOUT_OFFSET_LENGTH.
#define VECTOR_OFFSET_LENGTH -1

//	This conforms to the current naming convention.
#define VECTOR_LAYOUT_OFFSET_LENGTH -1

extern unsigned long sizeAfterKeyOfVectorLayout( Ref * key );
extern unsigned long lengthOfVectorLayout( Ref * key );
extern Ref fastVectorLength( Ref r );
*/

////////////////////////////////////////////////////////////////////////////////
//	Mixed vectors
////////////////////////////////////////////////////////////////////////////////

#define MIXED_LAYOUT_OFFSET_LENGTH -1

extern unsigned long sizeAfterKeyOfMixedLayout( Ref * key );
extern unsigned long lengthOfMixedLayout( Ref * key );
extern unsigned long numFieldsOfMixedLayout( Ref * key );

/*
////////////////////////////////////////////////////////////////////////////////
//	Strings
////////////////////////////////////////////////////////////////////////////////

//	Deprecate this in favour of STRING_LAYOUT_OFFSET_LENGTH.
#define STRING_OFFSET_LENGTH -1

//	This conforms to the current naming convention.
#define STRING_LAYOUT_OFFSET_LENGTH -1

extern unsigned long sizeAfterKeyOfStringLayout( Ref * key );
extern unsigned long lengthOfStringLayout( Ref * key );
extern unsigned long fastStringLength( Ref r );
*/

////////////////////////////////////////////////////////////////////////////////
//	Objects
////////////////////////////////////////////////////////////////////////////////

extern void findObjectLimits( Ref * object, Ref * & start, Ref * & end );
extern Ref * findObjectKey( Ref * obj_A );
extern unsigned long lengthAfterObjectKey( Ref * obj_K );


////////////////////////////////////////////////////////////////////////////////

#endif
