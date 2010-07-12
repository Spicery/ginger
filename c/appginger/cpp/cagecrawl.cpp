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

#include "cagecrawl.hpp"

#include "mishap.hpp"
#include "key.hpp"
#include "objlayout.hpp"

Ref * CageCrawl::next() {
	if ( this->current < this->cage->top ) {
		Ref * key = findKey( this->current );
		unsigned long len = lengthAfterKey( key );
		this->current = key + len + 1;
		return key;
	} else {
		return 0;
	}
}

Ref * CageCrawl::findKey( Ref * start ) {
	//	Cases are that 'start' are pointing at
	//		1.	FnLengthKey.
	//		2.	NonKey* Key series
	if ( IsFnLength( *start ) ) {
		//	We are at the start of a function.
		//	It has a fixed offset to the key.
		return start + OFFSET_FROM_FN_LENGTH_TO_KEY;
	} else {
		for ( int n = 0; n < MAX_OFFSET_FROM_START_TO_KEY; n++, start++ ) {
			if ( isKey( *start ) ) {
				return start;
			}
		}
		throw EmergencyExit();
	}
}

unsigned long CageCrawl::lengthAfterKey( Ref * k ) {
	//	Keys fall into the following categories: FnKey, SimpleKey, Pointer to Keys
	Ref key = *k;
	if ( IsSimpleKey( key ) ) {
		switch ( KindOfSimpleKey( key ) ) {
			case RECORD_KIND: {
				return sizeAfterKeyOfRecord( k );
				break;
			}
			case VECTOR_KIND: {
				return sizeAfterKeyOfVector( k );
				break;
			}
			case STRING_KIND: {
				return sizeAfterKeyOfString( k );
				break;
			}
			default: throw "Unreachable";
		}
	} else if ( IsFnKey( key ) ) {
		return sizeAfterKeyOfFnLength( k );
	} else if ( IsPtr4( key ) ) {
		//	Compound keys not implemented yet.
		throw "unimplemented";
	} else {
		throw;
	}
}