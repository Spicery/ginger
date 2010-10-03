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

#ifndef FUNCTION_LAYOUT_HPP
#define FUNCTION_LAYOUT_HPP

#include "common.hpp"

//	Standard offsets from the key.
#define OFFSET_TO_NUM_INPUTS				-1
#define OFFSET_TO_NUM_SLOTS					-2
#define OFFSET_TO_NUM_OUTPUTS				-3
#define OFFSET_TO_FUNCTION_LENGTH			-4

//	The offset from the key to the method cache.
#define FUNCTION_OFFSET_OF_METHOD_CACHE		2


// Non-standard offsets (from PC or to-the-key)
#define OFFSET_TO_NSLOTS_FROM_PC			-2
#define OFFSET_TO_NSLOTS_TO_KEY				(-OFFSET_TO_NUM_INPUTS)
#define OFFSET_FROM_FN_LENGTH_TO_KEY		(-OFFSET_TO_FUNCTION_LENGTH)

extern unsigned long sizeAfterKeyOfFn( Ref * key );

//	Not sure whether the correct choice is signed or unsigned or what. I
//	strongly suspect the correct choice is for them to be an Arity!
extern long numOutputsOfFn( Ref * key );
extern long numSlotsOfFn( Ref * key );
extern long numInputsOfFn( Ref * key );

#endif
