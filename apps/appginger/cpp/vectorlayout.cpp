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

#include <cassert>

#include "vectorlayout.hpp"
#include "misclayout.hpp"
#include "classlayout.hpp"

#include "common.hpp"
#include "key.hpp"
#include "mishap.hpp"

namespace Ginger {

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

/// 1-indexed access to the elements of a vector-layout object.
Ref indexVectorLayout( Ref * obj_K, ptrdiff_t n ) {
    return obj_K[ n ];
}


} // namespace Ginger
