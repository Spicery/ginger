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

#include "sysprint.hpp"
#include "syskey.hpp"
#include "objlayout.hpp"
#include "mishap.hpp"
#include "sysmap.hpp"


void refPrint( Ref r ) {
	refPrint( std::cout, r );
}

void refPtrPrint( Ref * r ) {
	refPrint( Ptr4ToRef( r ) );
}

void refPtrPrint( std::ostream & out, Ref * r ) {
	refPrint( out, Ptr4ToRef( r ) );
}

//	Relies on null termination, which is potentially dodgy - except this
//	is only for formatting printable characters.
static void refStringPrint( std::ostream & out, Ref * str_K ) {
	char *s = ToChars( str_K + 1 );
	out << s;	
}

static void refListPrint( std::ostream & out, Ref sofar ) {
	bool sep = false;
	out << "[";
	while ( IsPair( sofar ) ) {
		if ( sep ) { out << ","; } else { sep = true; }
		refPrint( out, *( RefToPtr4( sofar ) + 1 ) );
		sofar = *( RefToPtr4( sofar ) + 2 );
	}
	out << "]";
}

static void refVectorPrint( std::ostream & out, Ref * vec_K ) {
	bool sep = false;
	out << "{";
	long len = RefToLong( vec_K[ -1 ] );
	for ( long i = 1; i <= len; i++ ) {
		if ( sep ) { out << ","; } else { sep = true; }
		refPrint( vec_K[ i ] ); 
	}
	out << "}";
}

static void refRecordPrint( std::ostream & out, Ref * rec_K ) {
	unsigned long len = lengthOfRecord( rec_K );
	bool sep = false;
	out << "record{";
	for ( unsigned long i = 1; i <= len; i++ ) {
		if ( sep ) { out << ","; } else { sep = true; }
		refPrint( rec_K[ i ] ); 
	}
	out << "}";
}

void refPrint( std::ostream & out, Ref r ) {
	if ( IsObj( r ) ) {
		Ref * obj_K = ObjToPtr4( r );
		Ref key = *obj_K;
		if ( IsFunctionKey( key ) ) {
			out << "<function>";
		} else if ( IsSimpleKey( key ) ) {
			switch ( KindOfSimpleKey( key ) ) {
				case VECTOR_KIND: {
					refVectorPrint( out, obj_K );
					break;
				}
				case PAIR_KIND: {
					refListPrint( out, r );
					break;
				}
				case MAP_KIND: {
					refMapPrint( out, obj_K );
					break;
				}
				case RECORD_KIND: {
					refRecordPrint( out, obj_K );
					break;
				}
				case STRING_KIND: {
					refStringPrint( out, obj_K );
					break;
				}				
				default: {
					throw "unimplemented (other)";
				}
			}
		} else if ( IsObj( key ) ) {
			//	Compound keys not implemented yet.
			throw "unimplemented (compound keys)";
		} else {
			throw "unimplemented";
		}
	} else {
		Ref k = refKey( r );
		if ( k == sysSmallKey ) {
			out << SmallToLong( r );
		} else if ( k == sysBoolKey ) {
			out << ( r == sys_false ? "false" : "true" );
		} else if ( k == sysAbsentKey ) {
			out << "absent";
		} else if ( k == sysCharKey ) {
			out << CharacterToChar( r );
		} else if ( k == sysNilKey ) {
			refListPrint( out, r );
		} else if ( k == sysVectorKey ) {
			bool sep = false;
			out << "{";
			Ref * p = RefToPtr4( r );
			long len = RefToLong( p[ -1 ] );
			for ( int i = 1; i <= len; i++ ) {
				if ( sep ) { out << ","; } else { sep = true; }
				refPrint( p[ i ] ); 
			}
			out << "}";
		} else {
			out << "?(" << std::hex << ToULong( r ) << std::dec << ")";
		}
	}
}

Ref * sysRefPrint( Ref * pc, class MachineClass * vm ) {
	for ( int i = vm->count - 1; i >= 0; i-- ) {
		Ref r = vm->fastSubscr( i );
		refPrint( r );		
	}
	vm->fastDrop( vm->count );
	return pc;
}

Ref * sysRefPrintln( Ref * pc, class MachineClass * vm ) {
	pc = sysRefPrint( pc, vm );
	std::cout << std::endl;
	return pc;
}

