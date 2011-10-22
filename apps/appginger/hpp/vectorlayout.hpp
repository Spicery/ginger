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


#ifndef VECTOR_LAYOUT_HPP
#define VECTOR_LAYOUT_HPP

#include "common.hpp"

////////////////////////////////////////////////////////////////////////////////
//	Full vectors
////////////////////////////////////////////////////////////////////////////////

//	Deprecate this in favour of VECTOR_LAYOUT_OFFSET_LENGTH.
#define VECTOR_OFFSET_LENGTH -1

//	This conforms to the current naming convention.
#define VECTOR_LAYOUT_OFFSET_LENGTH -1

extern long sizeAfterKeyOfVectorLayout( Ref * key );
extern long lengthOfVectorLayout( Ref * key );
extern Ref refVectorLayoutLength( Ref r );

////////////////////////////////////////////////////////////////////////////////
//	End of file
////////////////////////////////////////////////////////////////////////////////

#endif
