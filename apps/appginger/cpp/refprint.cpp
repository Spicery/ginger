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


#include <sstream>

#include "mnx.hpp"

#include "sys.hpp"    

#include "sysprint.hpp"
#include "syskey.hpp"
#include "misclayout.hpp"
#include "mishap.hpp"
#include "sysmap.hpp"
#include "syssymbol.hpp"
#include "sysinstance.hpp"
#include "wrecordlayout.hpp"
#include "sysdouble.hpp"
#include "functionlayout.hpp"
#include "refprint.hpp"
#include "externalkind.hpp"

const char * OPEN_LIST = "[%";
const char * CLOSE_LIST = "%]";

const char * OPEN_VECTOR = "[";
const char * CLOSE_VECTOR = "]";

const char * STRING_QUOTE = "\"";
const char * CHAR_QUOTE = "'";
const char * SYMBOL_QUOTE = "`";

void RefPrint::output( const char ch ) {
	if ( this->html_escaping && not this->showing ) {
		Ginger::mnxRenderChar( this->out, ch );
	} else {
		this->out << ch;
	}
}

void RefPrint::output( const char * s ) {
	if ( this->html_escaping && not this->showing ) {
		Ginger::mnxRenderText( this->out, s );
	} else {
		this->out << s;
	}
}

void RefPrint::output( const std::string & s ) {
	if ( this->html_escaping && not this->showing 	) {
		Ginger::mnxRenderText( this->out, s );
	} else {
		this->out << s;
	}
}

//	Relies on null termination, which is potentially dodgy - except this
//	is only for formatting printable characters.
void RefPrint::refStringPrint( Ref * str_K ) {
	char *s = ToChars( str_K + 1 );
	if ( this->showing ) this->output( STRING_QUOTE );
	this->output( s );
	if ( this->showing ) this->output( STRING_QUOTE );
}

void RefPrint::refListPrint( Ref sofar ) {
	bool sep = false;
	this->output( OPEN_LIST );
	while ( IsPair( sofar ) ) {
		if ( sep ) { this->output( "," ); } else { sep = true; }
		this->refPrint( *( RefToPtr4( sofar ) + 1 ) );
		sofar = *( RefToPtr4( sofar ) + 2 );
	}
	this->output( CLOSE_LIST );
}

void RefPrint::refVectorPrint( Ref * vec_K ) {
	bool sep = false;
	this->output( OPEN_VECTOR );
	long len = RefToLong( vec_K[ -1 ] );
	for ( long i = 1; i <= len; i++ ) {
		if ( sep ) { this->output( ',' ); } else { sep = true; }
		this->refPrint( vec_K[ i ] ); 
	}
	this->output( CLOSE_VECTOR );
}

void RefPrint::refMixedPrint( Ref * mix_K ) {
	bool sep = false;
	this->output( "mixed[" );
	long len = numFieldsOfMixedLayout( mix_K );
	for ( long i = 1; i <= len; i++ ) {
		if ( sep ) { this->output( ',' ); } else { sep = true; }
		this->refPrint( mix_K[ i ] ); 
	}	
	this->output( "]{" );
	sep = false;
	len = RefToLong( mix_K[ MIXED_LAYOUT_OFFSET_LENGTH ] );
	for ( long i = 1; i <= len; i++ ) {
		if ( sep ) { this->output( ',' ); } else { sep = true; }
		this->refPrint( mix_K[ i ] ); 
	}
	this->output( "}" );
}


//	WARNING: KNOWN ERRORS. There is not the proper quoting of characters 
//	in the name, key or value.
void RefPrint::refAttrMapPrint( Ref * attrmap_K, enum Tag flag ) {
	std::string name( refToString( attrmap_K[ 1 ] ) );
	if ( flag == TAG_CLOSE ) {
		this->out << "</";
		Ginger::mnxRenderText( this->out, name );
		this->out << ">";
	} else {
		this->out << "<";
		Ginger::mnxRenderText( this->out, name );
		long length = SmallToULong( attrmap_K[ MIXED_LAYOUT_OFFSET_LENGTH ] );
		for ( long i = 0; i < length; i += 2 ) {
			this->out << " ";
			std::string key( refToString( attrmap_K[ 2 + i ] ) );
			Ginger::mnxRenderText( this->out, key );
			this->out << "=\"";
			std::string value( refToString( attrmap_K[ 3 + i ] ) );
			Ginger::mnxRenderText( this->out, value );
			this->out << "\"";
		}
		this->out << ( flag == TAG_OPEN ? ">" : "/>" );
	}
}

void RefPrint::refElementPrint( Ref * mix_K ) {
	Ref attrmap = mix_K[ 1 ];
	if ( ! IsAttrMap( attrmap ) ) {
		throw Ginger::Mishap( "Attribute map needed" ).culprit( "Item", refToString( attrmap ) );
	}
	Ref * attrmap_K = RefToPtr4( attrmap );
	long length = SmallToULong( mix_K[ MIXED_LAYOUT_OFFSET_LENGTH ] );
	if ( length > 0 ) {
		this->refAttrMapPrint( attrmap_K, TAG_OPEN );
		for ( long i = 0; i < length; i++ ) {
			RefPrint xhtml( *this );
			xhtml.html_escaping = true;
			xhtml.refPrint( mix_K[ 2 + i ] );
		}
		this->refAttrMapPrint( attrmap_K, TAG_CLOSE );
	} else {
		this->refAttrMapPrint( attrmap_K, TAG_EMPTY );
	}
}

void RefPrint::refRecordPrint( Ref * rec_K ) {
	unsigned long len = lengthOfRecordLayout( rec_K );
	bool sep = false;
	this->output( "record{" );
	for ( unsigned long i = 1; i <= len; i++ ) {
		if ( sep ) { this->output( ',' ); } else { sep = true; }
		this->refPrint( rec_K[ i ] ); 
	}
	this->output( '}' );
}

//	TODO: This looks very wrong!!!!
void RefPrint::refWRecordPrint( Ref * rec_K ) {
	unsigned long len = lengthOfWRecordLayout( rec_K );
	bool sep = false;
	this->output( "wrecord{" );
	for ( unsigned long i = 1; i <= len; i++ ) {
		if ( sep ) { this->output( ',' ); } else { sep = true; }
		this->refPrint( rec_K[ i ] ); 	//	TODO: NO!!!
	}
	this->output( '}' );
}

//	<TYPE FIELD ...>
//	
void RefPrint::refInstancePrint( Ref * rec_K ) {
	unsigned long len = lengthOfInstance( rec_K );
	//std::cout << "Length of object " << len << std::endl;
	//bool sep = false;
	this->output( "<" );
	this->refPrint( titleOfInstance( rec_K ) );
	//this->out << "[" << len << "]{";
	for ( unsigned long i = 1; i <= len; i++ ) {
		//if ( sep ) { this->out << ","; } else { sep = true; }
		this->output( ' ' );
		this->refPrint( rec_K[ i ] ); 
	}
	this->output( ">" );
}

void RefPrint::refDoublePrint( const Ref r ) {
	std::stringstream str;
	gngdouble_t d = gngFastDoubleValue( r );
	str << d;
	this->output( str.str() );
}

void RefPrint::refKeyPrint( const Ref r ) {
	const char * name = keyName( r );
	this->output( "<class " );
	this->output( name );
	this->output( ">" );
}

void RefPrint::refFunctionPrint( const Ref fn ) {
	Ref * fn_K = RefToPtr4( fn );
	Ref key = fn_K[ 0 ];

	const std::string name( nameOfFn( fn_K ) );

	if ( IsCoreFunctionKey( key ) ) {
		this->output( "<sysfn" );
	} else if ( IsMethodKey( key ) ) {
		this->output( "<method" );
	} else {
		this->output( "<function" );
	}
	if ( name.size() > 0 ) {
		this->output( ' ' );
		this->output( name );
	}
	this->output( '>' );
}

void RefPrint::refExternalPrint( const Ref * obj_K ) {
	Ginger::External * e = reinterpret_cast< Ginger::External * >( obj_K[ EXTERNAL_KIND_OFFSET_VALUE ] );
	e->print( this->out );
}

void RefPrint::refPrint( const Ref r ) {
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
			refFunctionPrint( r );
		} else if ( key == sysMapletKey ) {
			refPrint( fastMapletKey( r ) );
			this->output( " => " );
			refPrint( fastMapletValue( r ) );
		} else if ( IsSimpleKey( key ) ) {
			#ifdef DBG_SYSPRINT
				cerr << "-- printing simple key = " << hex << ToULong( key ) << dec << endl;
				cerr << "-- Kind of simple key = " << hex << KindOfSimpleKey( key ) << dec << endl;
			#endif
			switch ( KindOfSimpleKey( key ) ) {
				case VECTOR_KIND: {
					this->refVectorPrint( obj_K );
					break;
				}
				case ATTR_KIND:
				case MIXED_KIND: {
					if ( sysElementKey == key ) {
						this->refElementPrint( obj_K );
					} else {
						this->refMixedPrint( obj_K );
					}
					break;
				}
				case PAIR_KIND: {
					this->refListPrint( r );
					break;
				}
				case MAP_KIND: {
					gngPrintMapPtr( this->out, obj_K );
					break;
				}
				case RECORD_KIND: {
					this->refRecordPrint( obj_K );
					break;
				}
				case EXTERNAL_KIND: {
					this->refExternalPrint( obj_K );
					break;
				}
				case WRECORD_KIND: {
					//cout << "sysDoubleKey = " << hex << sysDoubleKey << endl;
					//cout << "*obj_K = " << hex << *obj_K << endl;
					if ( *obj_K == sysDoubleKey ) {
						this->refDoublePrint( r );
					} else {
						this->refWRecordPrint( obj_K );
					}
					break;
				}
				case STRING_KIND: {
					this->refStringPrint( obj_K );
					break;
				}
				default: {
					this->out << "<printing undefined>";
					break;
				}
			}
		} else if ( IsObj( key ) ) {
			//	Compound keys not implemented yet.
			this->refInstancePrint( obj_K );
		} else {
			this->output( "<printing undefined>" );
		}
	} else {
		Ref k = refDataKey( r );
		#ifdef DBG_SYSPRINT
			cerr << "--  primitive" << endl;
		#endif
		if ( k == sysSmallKey ) {
			this->out << SmallToLong( r );
		} else if ( k == sysBoolKey ) {
			this->output( r == SYS_FALSE ? "false" : "true" );
		} else if ( k == sysAbsentKey ) {
			this->output( "absent" );
		} else if ( k == sysPresentKey ) {
			this->output( "present" );
		} else if ( k == sysUndefinedKey ) {
			this->output( "undefined" );
		} else if ( k == sysIndeterminateKey ) {
			this->output( "indeterminate" );
		} else if ( k == sysTerminKey ) {
			this->output( "termin" );
		} else if ( k == sysCharKey ) {
			#ifdef DBG_SYSPRINT
				cerr << "--  character: " << (int)(CharacterToChar( r )) << endl;
			#endif
			//this->out << "<char " << r  << ">";
			if ( this->showing ) this->output( CHAR_QUOTE );
			this->out << CharacterToChar( r );
			if ( this->showing ) this->output( CHAR_QUOTE );
		} else if ( k == sysSymbolKey ) {
			if ( this->showing ) this->output( SYMBOL_QUOTE );
			this->out << symbolToStdString( r );
			if ( this->showing ) this->output( SYMBOL_QUOTE );
		} else if ( k == sysNilKey ) {
			this->refListPrint( r );
		} else if ( k == sysClassKey ) {
			this->refKeyPrint( r );
		} else {
			this->output( "?(" );
			this->out << std::hex << ToULong( r ) << std::dec;
			this->output( ")" );
		}
	}
}