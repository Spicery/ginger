#include "key.hpp"

bool isKey( Ref x ) {
	unsigned long u = ToULong( x );
	if ( ( u & TAG_MASK ) == PTR_TAG ) {
		return *RefToPtr4( x ) == sysKeyKey;
	} else if ( ( u & TAGG_MASK ) == KEY_TAGG ) {
		return true;
	} else {
		return false;
	}
}


const char * keyName( Ref key ) {
	//	Keys fall into the following categories: FnKey, SimpleKey, Pointer to Keys
	if ( IsSimpleKey( key ) ) {
		switch ( SimpleKeyID( key ) ) {
			case 0: return "Absent";
			case 1: return "Bool";
			case 2: return "Termin";
			case 3: return "Nil";
			case 4: return "Pair";
			case 5: return "Vector";
			case 6: return "String";
			case 7: return "Word";
			case 8: return "Small";
			case 9: return "Float";
			case 10: return "Key";
			case 11: return "Unicode";
			case 12: return "Char";
			case 13: return "Maplet";
			default: throw;
		}
	} else if ( IsFnKey( key ) ) {
		return "Function";
	} else if ( IsPtr4( key ) ) {
		//	Compound keys not implemented yet.
		throw "unimplemented";
	} else {
		throw;
	}
}