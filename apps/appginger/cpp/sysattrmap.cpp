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

#include "sysattrmap.hpp"
#include "attrmaplayout.hpp"

#include "sysequals.hpp"
#include "sysprint.hpp"
#include "mishap.hpp"
#include "key.hpp"
#include "sys.hpp"

namespace Ginger {

Ref * sysAttrMapExplode( Ref *pc, class MachineClass * vm ) {
	if ( vm->count != 1) throw Ginger::Mishap( "ArgsMismatch" );
	const Ref amap = vm->fastPop();
	
	if ( !IsAttrMap( amap ) ) throw Ginger::Mishap( "Map needed" ).culprit( "Object", refToShowString( amap ) );
	
	Ref * amap_K = RefToPtr4( amap );
	const long N = SmallToLong( amap_K[ ATTR_MAP_OFFSET_LENGTH ] );
	vm->checkStackRoom( N );   // More than needed, strictly speaking.

	Ref * v = &amap_K[ ATTR_MAP_OFFSET_VECTOR ];
	for ( long i = 0; i < (N - 1); i += 2 ) {
		vm->fastPush( v[ i + 1 ] );
	}
	return pc;
}
//SysMap::value_type( "attrMapExplode", SysInfo( Arity( 1 ), Arity( 0, true ), sysAttrMapExplode, "Explodes an attribute-map into its members" ) ),
SysInfo infoAttrMapExplode( 
    FullName( "attrMapExplode" ), 
    Ginger::Arity( 1 ), 
    Ginger::Arity( 0, true ), 
    sysAttrMapExplode, 
    "Explodes an attribute-map into its members."
);

Ref * sysAttrMapIndex( Ref *pc, class MachineClass * vm ) {
	if ( vm->count != 2 ) throw Ginger::Mishap( "ArgsMismatch" );
	const Ref amap = vm->fastPop();
	const Ref idx = vm->fastPop();
	
	if ( !IsAttrMap( amap ) ) throw Ginger::Mishap( "Map needed" ).culprit( "Object", refToShowString( amap ) );
	
	Ref * amap_K = RefToPtr4( amap );
	const long N = RefToLong( amap_K[ ATTR_MAP_OFFSET_LENGTH ] );

	Ref * v = &amap_K[ ATTR_MAP_OFFSET_VECTOR ];
	for ( long i = 0; i < (N - 1); i += 2 ) {
		Ref key_i = v[ i ];
		if ( refEquals( key_i, idx ) ) {
			vm->fastPush( v[ i + 1 ] );
			return pc;
		}
	}
	vm->fastPush( SYS_ABSENT );
	return pc;
}
//SysMap::value_type( "attrMapIndex", SysInfo( Arity( 2 ), Arity( 1 ), sysAttrMapIndex, "Indexes an attribute-map" ) ),
SysInfo infoAttrMapIndex( 
    FullName( "attrMapIndex" ), 
    Ginger::Arity( 2 ), 
    Ginger::Arity( 1 ), 
    sysAttrMapIndex, 
    "Indexes an attribute-map."
);


} // namespace Ginger
