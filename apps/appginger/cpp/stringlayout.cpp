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


#include <cassert>

#include "common.hpp"
#include "key.hpp"
#include "stringlayout.hpp"

namespace Ginger {

////////////////////////////////////////////////////////////////////////////////
//	Strings
////////////////////////////////////////////////////////////////////////////////

//	Add 1 for null.
long sizeAfterKeyOfStringLayout( Ref * key ) {
	return ( SmallToLong( key[ STRING_LAYOUT_OFFSET_LENGTH ] ) + sizeof( long ) - 1 + 1 ) / sizeof( long );
}

long lengthOfStringLayout( Ref * key ) {
	return SmallToLong( key[ STRING_LAYOUT_OFFSET_LENGTH ] );
}

} // namespace Ginger