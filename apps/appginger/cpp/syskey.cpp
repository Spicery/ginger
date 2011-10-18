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

#include "syskey.hpp"

#include "common.hpp"
#include "key.hpp"
#include "mishap.hpp"
#include "vmi.hpp"
#include "machine.hpp"
#include "classlayout.hpp"

Ref refKey( Ref r ) {
	unsigned long u = ( unsigned long )r;
	unsigned long tag, tagg, taggg;
	tag = u & TAG_MASK;
	if ( tag == INT_TAG ) return sysSmallKey;
	if ( tag == OBJ_TAG ) return *RefToPtr4(r);
	tagg = u & TAGG_MASK;
	if ( tagg == ( 0 | SIM_TAG ) ) return sysAbsentKey;
	if ( tagg == ToULong( SYS_FALSE ) || tagg == ToULong( SYS_TRUE ) ) return sysBoolKey;
	if ( tagg == FN_TAGG ) return sysFunctionKey;
	if ( tagg == KEY_TAGG ) return sysKeyKey;
	taggg = u & TAGGG_MASK;
	if ( taggg == CHAR_TAGGG ) return sysCharKey;
	if ( taggg == SYMBOL_TAGGG ) return sysSymbolKey;
	if ( taggg == MISC_TAGGG ) {
		if ( r == SYS_NIL ) {
			return sysNilKey;
		} else if ( r == SYS_UNDEF ) {
			return sysUndefKey;
		} else {
			throw;
		}
	}
	throw;
}

Ref * sysObjectKey( Ref * pc, MachineClass * vm ) {
	if ( vm->count == 1 ) {
		vm->fastPeek() = refKey( vm->fastPeek() );
		return pc;
	} else {
		throw Ginger::Mishap( "Wrong number of arguments for objectKey" );
	}
}
