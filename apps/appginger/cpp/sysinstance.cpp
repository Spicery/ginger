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

#include "sysinstance.hpp"

#include "key.hpp"
#include "classlayout.hpp"
#include "mishap.hpp"
#include "sysprint.hpp"

namespace Ginger {

unsigned long lengthOfInstance( Ref * obj_K ) {
	Ref key = *obj_K;
	if ( IsObj( key ) && *RefToPtr4( key ) == sysClassKey ) {
		Ref * key_K = RefToPtr4( key );
		int N = SmallToLong( key_K[ CLASS_OFFSET_NFIELDS ] );
		return N;
	} else {	
		throw Ginger::Mishap( "Instance needed" ).culprit( "Argument", refToShowString( Ptr4ToRef( obj_K ) ) );
	}
}

Ref titleOfInstance( Ref * obj_K ) {
	Ref key = *obj_K;
	if ( IsObj( key ) && *RefToPtr4( key ) == sysClassKey ) {
		Ref * key_K = RefToPtr4( key );
		return key_K[ CLASS_OFFSET_TITLE ];
	} else {	
		throw Ginger::Mishap( "Instance needed" ).culprit( "Argument", refToShowString( Ptr4ToRef( obj_K ) ) );
	}
}

} // namespace Ginger