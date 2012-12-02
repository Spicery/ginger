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
#include <sstream>
#include <iostream>

#include "mnx.hpp"

#include "sysprint.hpp"
#include "syskey.hpp"
#include "misclayout.hpp"
#include "mishap.hpp"
#include "sysmap.hpp"
#include "syssymbol.hpp"
#include "sysinstance.hpp"
#include "wrecordlayout.hpp"
#include "sysdouble.hpp"

//#define DBG_SYSPRINT

using namespace std;

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

static const char * open_list = "[%";
static const char * close_list = "%]";

static void refListPrint( std::ostream & out, Ref sofar ) {
	bool sep = false;
	out << open_list;
	while ( IsPair( sofar ) ) {
		if ( sep ) { out << ","; } else { sep = true; }
		refPrint( out, *( RefToPtr4( sofar ) + 1 ) );
		sofar = *( RefToPtr4( sofar ) + 2 );
	}
	out << close_list;
}

static const char * open_vector = "[";
static const char * close_vector = "]";

static void refVectorPrint( std::ostream & out, Ref * vec_K ) {
	bool sep = false;
	out << open_vector;
	long len = RefToLong( vec_K[ -1 ] );
	for ( long i = 1; i <= len; i++ ) {
		if ( sep ) { out << ","; } else { sep = true; }
		refPrint( vec_K[ i ] ); 
	}
	out << close_vector;
}

static void refMixedPrint( std::ostream & out, Ref * mix_K ) {
	bool sep = false;
	out << "mixed[";
	long len = numFieldsOfMixedLayout( mix_K );
	for ( long i = 1; i <= len; i++ ) {
		if ( sep ) { out << ","; } else { sep = true; }
		refPrint( mix_K[ i ] ); 
	}	
	out <<"]{";
	sep = false;
	len = RefToLong( mix_K[ MIXED_LAYOUT_OFFSET_LENGTH ] );
	for ( long i = 1; i <= len; i++ ) {
		if ( sep ) { out << ","; } else { sep = true; }
		refPrint( mix_K[ i ] ); 
	}
	out << "}";
}

enum Tag {
	TAG_OPEN,
	TAG_CLOSE,
	TAG_EMPTY
};

//	WARNING: KNOWN ERRORS. There is not the proper quoting of characters 
//	in the name, key or value.
static void refAttrMapPrint( std::ostream & out, Ref * attrmap_K, enum Tag flag ) {
	std::string name( refToString( attrmap_K[ 1 ] ) );
	if ( flag == TAG_CLOSE ) {
		out << "</";
		Ginger::mnxRenderText( out, name );
		out << ">";
	} else {
		out << "<";
		Ginger::mnxRenderText( out, name );
		long length = SmallToULong( attrmap_K[ MIXED_LAYOUT_OFFSET_LENGTH ] );
		for ( long i = 0; i < length; i += 2 ) {
			out << " ";
			std::string key( refToString( attrmap_K[ 2 + i ] ) );
			Ginger::mnxRenderText( out, key );
			out << "=\"";
			std::string value( refToString( attrmap_K[ 3 + i ] ) );
			Ginger::mnxRenderText( out, value );
			out << "\"";
		}
		out << ( flag == TAG_OPEN ? ">" : "/>" );
	}
}

static void refElementPrint( std::ostream & out, Ref * mix_K ) {
	Ref attrmap = mix_K[ 1 ];
	if ( ! IsAttrMap( attrmap ) ) {
		throw Ginger::Mishap( "Attribute map needed" ).culprit( "Item", refToString( attrmap ) );
	}
	Ref * attrmap_K = RefToPtr4( attrmap );
	long length = SmallToULong( mix_K[ MIXED_LAYOUT_OFFSET_LENGTH ] );
	if ( length > 0 ) {
		refAttrMapPrint( out, attrmap_K, TAG_OPEN );
		for ( long i = 0; i < length; i++ ) {
			refPrint( out, mix_K[ 2 + i ] );
		}
		refAttrMapPrint( out, attrmap_K, TAG_CLOSE );
	} else {
		refAttrMapPrint( out, attrmap_K, TAG_EMPTY );
	}
}

static void refRecordPrint( std::ostream & out, Ref * rec_K ) {
	unsigned long len = lengthOfRecordLayout( rec_K );
	bool sep = false;
	out << "record{";
	for ( unsigned long i = 1; i <= len; i++ ) {
		if ( sep ) { out << ","; } else { sep = true; }
		refPrint( rec_K[ i ] ); 
	}
	out << "}";
}

//	TODO: This looks very wrong!!!!
static void refWRecordPrint( std::ostream & out, Ref * rec_K ) {
	unsigned long len = lengthOfWRecordLayout( rec_K );
	bool sep = false;
	out << "wrecord{";
	for ( unsigned long i = 1; i <= len; i++ ) {
		if ( sep ) { out << ","; } else { sep = true; }
		refPrint( rec_K[ i ] ); 	//	TODO: NO!!!
	}
	out << "}";
}

static void refInstancePrint( std::ostream & out, Ref * rec_K ) {
	unsigned long len = lengthOfInstance( rec_K );
	//std::cout << "Length of object " << len << std::endl;
	bool sep = false;
	refPrint( out, titleOfInstance( rec_K ) );
	out << "[" << len << "]{";
	for ( unsigned long i = 1; i <= len; i++ ) {
		if ( sep ) { out << ","; } else { sep = true; }
		refPrint( rec_K[ i ] ); 
	}
	out << "}";
}

void refDoublePrint( std::ostream & out, const Ref r ) {
	gngdouble_t d = gngFastDoubleValue( r );
	out << d;
}

void refKeyPrint( std::ostream & out, const Ref r ) {
	const char * name = keyName( r );
	out << "<class " << name << ">";
}



void refPrint( std::ostream & out, const Ref r ) {
	#ifdef DBG_SYSPRINT
		cerr << "Printing ref: " << r << endl;
	#endif
	if ( IsObj( r ) ) {
		#ifdef DBG_SYSPRINT
			cerr << "-- is object" << endl;
		#endif
		Ref * obj_K = RefToPtr4( r );
		const Ref key = * obj_K;
		if ( IsFunctionKey( key ) ) {
			out << "<function>";
		} else if ( key == sysMapletKey ) {
			refPrint( fastMapletKey( r ) );
			out << " => ";
			refPrint( fastMapletValue( r ) );
		} else if ( IsSimpleKey( key ) ) {
			#ifdef DBG_SYSPRINT
				cerr << "-- printing simple key = " << hex << ToULong( key ) << endl;
				cerr << "-- Kind of simple key = " << hex << KindOfSimpleKey( key ) << endl;
			#endif
			switch ( KindOfSimpleKey( key ) ) {
				case VECTOR_KIND: {
					refVectorPrint( out, obj_K );
					break;
				}
				case ATTR_KIND:
				case MIXED_KIND: {
					if ( sysElementKey == key ) {
						refElementPrint( out, obj_K );
					} else {
						refMixedPrint( out, obj_K );
					}
					break;
				}
				case PAIR_KIND: {
					refListPrint( out, r );
					break;
				}
				case MAP_KIND: {
					gngPrintMapPtr( out, obj_K );
					break;
				}
				case RECORD_KIND: {
					refRecordPrint( out, obj_K );
					break;
				}
				case WRECORD_KIND: {
					//cout << "sysDoubleKey = " << hex << sysDoubleKey << endl;
					//cout << "*obj_K = " << hex << *obj_K << endl;
					if ( *obj_K == sysDoubleKey ) {
						refDoublePrint( out, r );
					} else {
						refWRecordPrint( out, obj_K );
					}
					break;
				}
				case STRING_KIND: {
					refStringPrint( out, obj_K );
					break;
				}
				default: {
					out << "<printing undefined>";
					break;
				}
			}
		} else if ( IsObj( key ) ) {
			//	Compound keys not implemented yet.
			refInstancePrint( out, obj_K );
		} else {
			out << "<printing undefined>";
		}
	} else {
		Ref k = refDataKey( r );
		#ifdef DBG_SYSPRINT
			cerr << "--  primitive" << endl;
		#endif
		if ( k == sysSmallKey ) {
			out << SmallToLong( r );
		} else if ( k == sysBoolKey ) {
			out << ( r == SYS_FALSE ? "false" : "true" );
		} else if ( k == sysAbsentKey ) {
			out << "absent";
		} else if ( k == sysPresentKey ) {
			out << "present";
		} else if ( k == sysUndefinedKey ) {
			out << "undefined";
		} else if ( k == sysIndeterminateKey ) {
			out << "indeterminate";
		} else if ( k == sysCharKey ) {
			#ifdef DBG_SYSPRINT
				cerr << "--  character: " << (int)(CharacterToChar( r )) << endl;
			#endif
			//out << "<char " << r  << ">";
			out << CharacterToChar( r );
		} else if ( k == sysSymbolKey ) {
			out << symbolToStdString( r );
		} else if ( k == sysNilKey ) {
			refListPrint( out, r );
		} else if ( k == sysClassKey ) {
			refKeyPrint( out, r );
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

std::string refToString( Ref ref ) {
	std::ostringstream s;
	refPrint( s, ref );
	return s.str();
}

