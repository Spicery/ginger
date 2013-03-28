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

#include <string>

#include "syskey.hpp"

#include "common.hpp"
#include "key.hpp"
#include "mishap.hpp"
//#include "vmi.hpp"
#include "machine.hpp"
#include "classlayout.hpp"

//#define DBG_DATA_KEY

#ifdef DBG_DATA_KEY
	#include <iostream>
#endif

using namespace std;


Ref refDataKey( Ref r ) {
	#ifdef DBG_DATA_KEY
		cerr << "Finding data key of: " << ToULong( r ) << endl;
	#endif
	unsigned long u = ( unsigned long )r;
	unsigned long tag, tagg, taggg;
	tag = u & TAG_MASK;
	if ( tag == INT_TAG ) return sysSmallKey;
	if ( tag == OBJ_TAG ) return *RefToPtr4(r);
	tagg = u & TAGG_MASK;
	if ( tagg == ( 0 | SIM_TAG ) ) return sysAbsentKey;
	if ( tagg == ToULong( SYS_FALSE ) || tagg == ToULong( SYS_TRUE ) ) return sysBoolKey;
	if ( tagg == FN_TAGG ) return sysClassKey;
	if ( tagg == KEY_TAGG ) return sysClassKey;
	taggg = u & TAGGG_MASK;
	if ( taggg == CHAR_TAGGG ) return sysCharKey;
	if ( taggg == SYMBOL_TAGGG ) return sysSymbolKey;
	if ( taggg == MISC_TAGGG ) {
		if ( r == SYS_NIL ) {
			return sysNilKey;
		} else if ( r == SYS_UNDEFINED ) {
			return sysUndefinedKey;
		} else if ( r == SYS_INDETERMINATE ) {
			return sysIndeterminateKey;
		} else if ( r == SYS_PRESENT ) {
			return sysPresentKey;
		} else if ( r == SYS_TERMIN ) {
			return sysTerminKey;
		} else {
			//	TODO: Improve error message.
			throw Ginger::Mishap( "Internal error (refDataKey): Unimplemented misc key" ).culprit( "Object", ToLong( r ) );
		}
	}
	//	TODO: Improve error message.
	throw Ginger::Mishap( "Internal error (refDataKey): Unimplemented key" ).culprit( "Object", ToLong( r ) );
}

Ref * sysObjectKey( Ref * pc, MachineClass * vm ) {
	if ( vm->count == 1 ) {
		vm->fastPeek() = refDataKey( vm->fastPeek() );
		return pc;
	} else {
		throw Ginger::Mishap( "Wrong number of arguments for dataClass" );
	}
}

Ref * sysKeyName( Ref * pc, MachineClass * vm ) {
	if ( vm->count == 1 ) {
		vm->fastPeek() = vm->heap().copyString( keyName( vm->fastPeek() ) );
		return pc;
	} else {
		throw Ginger::Mishap( "Wrong number of arguments for className" );
	}	
}