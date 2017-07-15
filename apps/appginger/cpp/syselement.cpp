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

#include <vector>

#include "cell.hpp"
#include "syselement.hpp"
#include "sysattrmap.hpp"
#include "mishap.hpp"
#include "sys.hpp"

namespace Ginger {
using namespace std;

Ref * sysElementName( Ref *pc, class MachineClass * vm ) {
	if ( vm->count != 1 ) throw Ginger::Mishap( "ArgsMismatch" );
	Ref element = vm->fastPeek();
	if ( !IsElement( element ) ) throw Ginger::Mishap( "TypeError" );
	Ref * element_K = RefToPtr4( element );
	vm->fastPeek() = RefToPtr4( element_K[ 1 ] )[ 1 ];
	return pc;
}
SysInfo infoElementName( 
	"nameElement", 
	Arity( 1 ), 
	Arity( 1 ), 
	sysElementName, 
	"Gets the name of an element" 
);

Ref * sysElementAttribute( Ref *pc, class MachineClass * vm ) {
	if ( vm->count != 2 ) throw Ginger::Mishap( "ArgsMismatch" );
	Ref idx = vm->fastPop();
	Ref element = vm->fastPeek();
	if ( !IsElement( element ) ) throw Ginger::Mishap( "TypeError" );
	Ref * element_K = RefToPtr4( element );
	vm->fastPeek() = element_K[ 1 ];
	vm->fastPush( idx );
	return sysAttrMapIndex( pc, vm );
}
SysInfo infoElementAttribute(
	"attributeElement", 
	Arity( 1 ), 
	Arity( 1 ), 
	sysElementAttribute, 
	"Gets the value of an attribute of an element" 
);

Ref * sysExplodeMapsAndMaplets( Ref *pc, class MachineClass * vm ) {
	vector< Cell > roots;

	long n = vm->count;
	for ( long i = 0; i < n; i++ ) {
		roots.push_back( vm->fastPeek( i ) );
	}

	vector< Cell > results;
	for ( vector< Cell >::iterator it = roots.begin(); it != roots.end(); ++it ) {
		if ( it->isHeapObject() ) {
			HeapObject h( *it );
			if ( h.isMapObject() ) {
				MapObject m( h );
				for ( MapObject::Generator g( m ); !!g; ++g ) {
					std::pair< Cell, Cell > p = *g;
					results.push_back( p.first );
					results.push_back( p.second );
				}
			} else if ( h.isMapletObject() ) {
				MapletObject m( h );
				results.push_back( m.key() );
				results.push_back( m.value() );
			} else {
				throw Mishap( "Unexpected value while destructing maps/maplets" ).culprit( "Value", h.toPrintString() );
			}
		} else {
			throw Mishap( "Unexpected value while destructing maps/maplets" ).culprit( "Value", it->toPrintString() );
		}
	}

	//	Assume that we can expand the stack without incurring a garbage collection.
	vm->fastDrop( vm->count );
	vm->checkStackRoom( results.size() );

	for ( vector< Cell >::iterator it = results.begin(); it != results.end(); ++it ) {
		vm->fastPush( it->asRef() );
	}

	return pc;
}
SysInfo infoExplodeMapsAndMaplets( 
    FullName( "explodeMapsAndMaplets" ), 
    Arity( 0, true ), 
    Arity( 0, true ), 
    sysExplodeMapsAndMaplets, 
    "Explodes maps into maplets and maplets into 2-tuples" 
);

} // namespace Ginger
