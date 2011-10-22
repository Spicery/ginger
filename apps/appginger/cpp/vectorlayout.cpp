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

#include "vectorlayout.hpp"
#include "misclayout.hpp"
#include "classlayout.hpp"

#include "common.hpp"
#include "key.hpp"
#include "mishap.hpp"


////////////////////////////////////////////////////////////////////////////////
//	Full vectors
////////////////////////////////////////////////////////////////////////////////

long sizeAfterKeyOfVectorLayout( Ref * key ) {
	return SmallToLong( key[ VECTOR_OFFSET_LENGTH ] );
	//return ToULong( *( key - 1 ) ) >> TAG;
}

long lengthOfVectorLayout( Ref * key ) {
	return SmallToULong( key[ VECTOR_OFFSET_LENGTH ] );
}

Ref refVectorLayoutLength( Ref r ) {
	return RefToPtr4( r )[ VECTOR_OFFSET_LENGTH ];
}

////////////////////////////////////////////////////////////////////////////////
//	End of file
////////////////////////////////////////////////////////////////////////////////
