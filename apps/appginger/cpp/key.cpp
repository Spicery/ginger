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
		return *RefToPtr4( x ) == sysClassKey;
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
			//	Add in the cases.
			#include "simplekey.cpp.auto"
			default: return "<Unknown>";
		}
	} else if ( IsFunctionKey( key ) ) {
		return "Function";
	} else if ( IsObj( key ) ) {
		//	TODO: Compound keys not implemented yet.
		return "Object";
	} else if ( IsFwd( key ) ) {
		//	In fact can be reached if any 
		return "<Forwarded!>";
	} else {
		return "<Also Unknown>";
	}
}
