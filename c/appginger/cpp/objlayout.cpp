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

#include "objlayout.hpp"

#include "common.hpp"
#include "key.hpp"

unsigned long sizeAfterKeyOfRecord( Ref * key ) {
	return ( ToULong( *key ) & LENGTH_MASK ) >> KIND_WIDTH >> TAGG;
}

unsigned long sizeAfterKeyOfVector( Ref * key ) {
	return ToULong( *( key - 1 ) ) >> TAG;
}

//	Add 1 for null.
unsigned long sizeAfterKeyOfString( Ref * key ) {
	return ( sizeAfterKeyOfVector( key ) + sizeof( long ) - 1 + 1 ) / sizeof( long );
}

unsigned long sizeAfterKeyOfFnLength( Ref * key ) {
	return ToULong( *( key - OFFSET_FROM_FN_LENGTH_TO_KEY ) ) >> TAGGG;
}

void findObjectLimits( Ref * object, Ref * & start, Ref * & end ) {
	
}
