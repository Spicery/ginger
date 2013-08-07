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
#include <iomanip>

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
#include "cell.hpp"

namespace Ginger {

int Numbering::number( const int depth ) {
	this->numbers.resize( depth, 0 );
	return this->numbers.back()++;
}

std::string Numbering::styledNumber( const int depth ) {
	std::ostringstream out;
	const int n = this->number( depth );
	if ( depth <= 1 ) {
		out << "[" << ( n + 1 ) << "]";
	} else if ( depth == 2 ) {
		this->alphabetical( out, n );
	} else {
		out << ( n + 1 ) << ".";
	}
	return out.str();
}

void Numbering::alphabetical( std::ostream & out, const int k ) {
	out << "(";
	out << static_cast< char >( 'a' + k % 25 );
	const int k1 = k / 25;
	if ( k1 > 0 ) {
		out << k1;
	}
	out << ")";
}

const char * OPEN_LIST = "[%";
const char * CLOSE_LIST = "%]";

const char * OPEN_VECTOR = "[";
const char * CLOSE_VECTOR = "]";

const char * OPEN_MAP = "{";
const char * CLOSE_MAP = "}";

const char * STRING_QUOTE = "\"";
const char * CHAR_QUOTE = "'";
const char * SYMBOL_QUOTE = "`";

void RefPrint::indent() {
	//this->out << "{" << this->indentation_level << "}";
	for ( int i = this->indentation_level - 1; i >= 0; i -= 1 ) {
		if ( i > 0 ) {
			this->out << "    ";
		} else {
			if ( this->list_style ) {
				const std::string num = this->numbering.styledNumber( this->indentation_level );
				this->out << std::left << std::setw( 4 ) << num;
			} else {
				this->out << "  * ";
			}
		}
	}
}

///	Suitable default for ch is anything except '\n'.
void RefPrint::indentIfNeeded( const char ch = '\0' ) {
	//this->out << "[" << this->column << "," << this->format << "]";
	if ( this->format == RefPrint::LIST ) {
		if ( this->column == 0 ) this->indent();
		this->column += 1;
		if ( ch == '\n' ) {
			this->column = 0;
		}
	}
}

void RefPrint::output( const char ch ) {
	if ( this->format == RefPrint::XHTML ) {
		Ginger::mnxRenderChar( this->out, ch );
	} else {
		this->indentIfNeeded( ch );
		this->out << ch;
	}
}

void RefPrint::output( const long n ) {
	if ( this->format == RefPrint::XHTML ) {
		this->out << n;
	} else {
		this->indentIfNeeded();
		this->out << n;
	}
}

void RefPrint::output( const char * s ) {
	if ( this->format == RefPrint::XHTML ) {
		Ginger::mnxRenderText( this->out, s );
	} else if ( this->format == RefPrint::LIST ) {
		while ( *s != '\0' ) {
			this->output( *s++ );
		}
	} else {
		this->out << s;
	}
}

void RefPrint::output( const std::string & s ) {
	if ( this->format == RefPrint::XHTML	) {
		Ginger::mnxRenderText( this->out, s );
	} else if ( this->format == RefPrint::LIST ) {
		this->output( s.c_str() );
	} else {
		this->out << s;
	}
}

//	Relies on null termination, which is potentially dodgy - except this
//	is only for formatting printable characters.
void RefPrint::refStringPrint( Ref * str_K ) {
	char * s = ToChars( str_K + 1 );
	if ( this->format == RefPrint::SHOW ) this->output( STRING_QUOTE );
	this->output( s );
	if ( this->format == RefPrint::SHOW ) this->output( STRING_QUOTE );
}


void RefPrint::startList( const bool ordered ) {
	if ( this->format == RefPrint::XHTML ) {
		this->out << ( ordered ? "<ol>" : "<ul>" );
	} else if ( this->format == RefPrint::LIST ) {
		//	Skip.
	} else {
		this->out << ( ordered ? OPEN_VECTOR : OPEN_LIST );
	}
}

void RefPrint::startListItem( const bool is_ordered, const int n ) {
	if ( this->format == RefPrint::XHTML ) {
		this->out << "<li>";
	} else if ( this->format == RefPrint::LIST ) {
		this->list_style = is_ordered;		
		//	This is an obscure idiom that implements separators.
		if ( n > 0 ) {
			this->output( '\n' );
		}
	} else if ( n > 0 ) {
		this->out << ",";
	}
}

void RefPrint::endListItem( const bool is_ordered, const int n ) {
	if ( this->format == RefPrint::XHTML ) {
		this->out << "</li>";
	} else if ( this->format == RefPrint::LIST ) {
		//	Skip.
	}
}

void RefPrint::endList( const bool ordered ) {
	if ( this->format == RefPrint::XHTML ) {
		this->out << ( ordered ? "</ol>" : "</ul>" );
	} else if ( this->format == RefPrint::LIST ) {
		//	Skip.
	} else {
		this->out << ( ordered ? CLOSE_VECTOR : CLOSE_LIST );
	}
}

void RefPrint::refListPrint( Ref sofar ) {
	RefPrint printer( *this, RefPrint::LIST );
	printer.indentation_level += 1;
	int n = 0;
	printer.startList( false );
	while ( IsPair( sofar ) ) {
		printer.startListItem( false, n );
		printer.refPrint( *( RefToPtr4( sofar ) + 1 ) );
		sofar = *( RefToPtr4( sofar ) + 2 );
		printer.endListItem( false, n++ );
	}
	printer.endList( false );
}

void RefPrint::refVectorPrint( Ref * vec_K ) {
	RefPrint printer( *this, RefPrint::LIST );
	printer.indentation_level += 1;
	long len = RefToLong( vec_K[ -1 ] );
	printer.startList( true );
	for ( long i = 1; i <= len; i++ ) {
		printer.startListItem( true, i - 1);
		printer.refPrint( vec_K[ i ] ); 
		printer.endListItem( true, i - 1 );
	}
	printer.endList( true );
}

void RefPrint::startMap() {
	if ( this->format == XHTML ) {
		this->out << "<dl>";
	} else if ( this->format == LIST ) {
		//	Skip.
	} else {
		this->out << OPEN_MAP;
	}
}

void RefPrint::startMapletKey( const int it_count ) {
	if ( this->format == XHTML ) {
		this->out << "<dt>";
	} else if ( this->format == LIST ) {
		//	This is an obscure idiom that implements separators.
		if ( it_count > 0 ) {
			this->output( '\n' );
		}
	} else {
		if ( it_count > 0 ) {
			this->output( "," );
		}
	}
}

void RefPrint::endMapletKey( const int count ) {
	if ( this->format == XHTML ) {
		this->out << "</dt>";
	} else if ( this->format == LIST ) {
		this->output( ": " );
	}
}

void RefPrint::startMapletValue( const int it_count ) {
	if ( this->format == XHTML ) {
		this->out << "<dd>";
	} else if ( this->format == LIST ) {
		//	Skip
	} else {
		this->out << " => ";
	}
}

void RefPrint::endMapletValue( const int count ) {
	if ( this->format == XHTML ) {
		this->out << "</dd>";
	} 
}

void RefPrint::endMap() {
	if ( this->format == XHTML ) {
		this->out << "</dl>";
	} else if ( this->format == LIST ) {
		//this->lineBreak();
	} else {
		this->out << CLOSE_MAP;
	}
}

void RefPrint::refMapPrint( Ref * r ) {
	RefPrint printer( *this, LIST );
	printer.indentation_level += 1;
	int iteration_count = 0;
	Ginger::MapObject m( r );
	printer.startMap();
	for ( Ginger::MapObject::Generator g( m ); !!g; ++g ) {
		std::pair< Ginger::Cell, Ginger::Cell > p = *g;
		printer.startMapletKey( iteration_count );
		printer.refPrint( p.first.asRef() );
		printer.endMapletKey( iteration_count );
		printer.startMapletValue( iteration_count );
		printer.refPrint( p.second.asRef() );
		printer.endMapletValue( iteration_count++ );
	}
	printer.endMap();
}

void RefPrint::refMapletPrint( Ref key, Ref value ) {
	this->refPrint( key );
	this->output( " => " );
	this->refPrint( value );	
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
	std::string name( refToPrintString( attrmap_K[ 1 ] ) );
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
			std::string key( refToPrintString( attrmap_K[ 2 + i ] ) );
			Ginger::mnxRenderText( this->out, key );
			this->out << "=\"";
			std::string value( refToPrintString( attrmap_K[ 3 + i ] ) );
			Ginger::mnxRenderText( this->out, value );
			this->out << "\"";
		}
		this->out << ( flag == TAG_OPEN ? ">" : "/>" );
	}
}

void RefPrint::refElementPrint( Ref * mix_K ) {
	Ref attrmap = mix_K[ 1 ];
	if ( ! IsAttrMap( attrmap ) ) {
		throw Ginger::Mishap( "Attribute map needed" ).culprit( "Item", refToShowString( attrmap ) );
	}
	Ref * attrmap_K = RefToPtr4( attrmap );
	long length = SmallToULong( mix_K[ MIXED_LAYOUT_OFFSET_LENGTH ] );
	if ( length > 0 ) {
		this->refAttrMapPrint( attrmap_K, TAG_OPEN );
		for ( long i = 0; i < length; i++ ) {
			RefPrint xhtml( *this, XHTML );
			if ( i != 0 && this->format == RefPrint::SHOW ) {
				this->out << ",";
			}
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
	this->output( "<" );
	this->refPrint( titleOfInstance( rec_K ) );
	for ( unsigned long i = 1; i <= len; i++ ) {
		this->output( ' ' );
		this->refPrint( rec_K[ i ] ); 
	}
	this->output( ">" );
}

void RefPrint::refDoublePrint( const Ref r ) {	
	gngdouble_t d = gngFastDoubleValue( r );
	if ( d.isFinite() ) {
		std::stringstream str;
		str << d;
		this->output( str.str() );
	} else {
		switch ( d.quadrachotomy() ) {
			case 'G': this->output( "+infinity" ); break;
			case 'L': this->output( "-infinity" ); break;
			case 'N': this->output( "nullity" ); break;
			default: throw Ginger::Mishap( "Unknown double value" ).culprit( "Value", d.asDouble() );
		}
	}
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
	this->indentIfNeeded();
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
			this->refMapletPrint( fastMapletKey( r ), fastMapletValue( r ) );
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
					this->refMapPrint( obj_K );
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
					this->output( "<printing undefined>" );
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
			this->output( SmallToLong( r ) );
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
			if ( this->format == RefPrint::SHOW ) this->output( CHAR_QUOTE );
			this->output( CharacterToChar( r ) );
			if ( this->format == RefPrint::SHOW ) this->output( CHAR_QUOTE );
		} else if ( k == sysSymbolKey ) {
			if ( this->format == RefPrint::SHOW ) this->output( SYMBOL_QUOTE );
			this->output( symbolToStdString( r ) );
			if ( this->format == RefPrint::SHOW ) this->output( SYMBOL_QUOTE );
		} else if ( k == sysNilKey ) {
			this->refListPrint( r );
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

} // namespace Ginger