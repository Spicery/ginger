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

/*Ref * findFunctionKey( Ref * inside_fn ) {
	//	Walk back down until you find a FnLength ptr.
	while ( not IsFnLength( *inside_fn ) ) {
		inside_fn++;
	}
	
}*/


#endif