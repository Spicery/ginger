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

#include "key.hpp"
#include "mishap.hpp"

bool isKey( Ref x ) {
	unsigned long u = ToULong( x );
	if ( ( u & TAG_MASK ) == OBJ_TAG ) {
		return *RefToPtr4( x ) == sysKeyKey;
	} else if ( ( u & TAGG_MASK ) == KEY_TAGG ) {
		return true;
	} else {
		return false;
	}
}


const char * keyName( Ref key ) {
	//	Keys fall into the following categories: FunctionKey, SimpleKey, Pointer to Keys
	if ( IsSimpleKey( key ) ) {
		switch ( SimpleKeyID( key ) ) {
			case 0: return "Absent";
			case 1: return "Bool";
			case 2: return "Key";
			case 3: return "Termin";
			case 4: return "Nil";
			case 5: return "Pair";
			case 6: return "Vector";
			case 7: return "String";
			case 8: return "Symbol";
			case 9: return "Small";
			case 10: return "Float";
			case 11: return "Unicode";
			case 12: return "Char";
			case 13: return "Maplet";
			case 14: return "Map";
			case 15: return "Assoc";
			case 16: return "Indirection";
			default: return "<Unknown>";
		}
	} else if ( IsFunctionKey( key ) ) {
		return "Function";
	} else if ( IsObj( key ) ) {
		//	Compound keys not implemented yet.
		return "Instance";
	} else if ( IsFwd( key ) ) {
		//	In fact can be reached if any 
		return "<Forwarded!>";
	} else {
		return "<Also Unknown>";
	}
}
