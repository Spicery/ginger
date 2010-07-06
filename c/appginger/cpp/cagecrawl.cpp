#include "cagecrawl.hpp"

#include "mishap.hpp"
#include "key.hpp"
#include "objlayout.hpp"

Ref * CageCrawl::next() {
	if ( this->current < this->cage.top ) {
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
			case RECORD_FLAVOUR: {
				return sizeAfterKeyOfRecord( k );
				break;
			}
			case VECTOR_FLAVOUR: {
				return sizeAfterKeyOfVector( k );
				break;
			}
			case STRING_FLAVOUR: {
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