/******************************************************************************\
	Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
	of the GNU General Public License. This file is part of Ginger.

    Ginger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Ginger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Ginger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

#ifndef STRING_LAYOUT_HPP
#define STRING_LAYOUT_HPP

namespace Ginger {

////////////////////////////////////////////////////////////////////////////////
//	Strings
////////////////////////////////////////////////////////////////////////////////

//	Deprecate this in favour of STRING_LAYOUT_OFFSET_LENGTH.
#define STRING_OFFSET_LENGTH -1

//	This conforms to the current naming convention.
#define STRING_LAYOUT_OFFSET_LENGTH -1

extern long sizeAfterKeyOfStringLayout( Ref * key );
extern long lengthOfStringLayout( Ref * key );

} // namespace Ginger

#endif
