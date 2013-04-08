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

#ifndef REF_PRINT_HPP
#define REF_PRINT_HPP

#include <vector>

#include "key.hpp"

class Numbering {
private:
	std::vector< int > numbers;
public:
	int number( const int depth );
	std::string styledNumber( const int depth );
private:
	void alphabetical( std::ostream & out, const int k );
};

class RefPrint {
public:
	enum FORMAT {
		PRINT,
		SHOW,
		LIST,
		XHTML
	};
private:
	std::ostream & out;
	enum FORMAT format;
	int indentation_level;
	int & column;
	bool list_style;
	Numbering numbering;

public:
	RefPrint( std::ostream & out, int & c, const enum FORMAT f = PRINT ) : 
		out( out ), 
		format( f ),
		indentation_level( 0 ),
		column( c ),
		list_style( false )		//	false = unordered
	{
		this->column = 0;
	}

	RefPrint( const RefPrint & that, const enum FORMAT f ) : 
		out( that.out ), 
		format( f ),
		indentation_level( that.indentation_level ),
		column( that.column ),
		list_style( false )		//	false = unordered
	{
		this->column = 0;
	}

private:
	void output( const char ch );
	void output( const long n );
	void output( const char * s );
	void output( const std::string & s );

	void indent();
	void indentIfNeeded( const char ch );

	void startList( const bool ordered );
	void startListItem( const bool is_ordered, const int count );
	void endListItem( const bool is_ordered, const int count );
	void endList( const bool ordered );

	void startMap();
	void startMapletKey( const int count );
	void endMapletKey( const int count );
	void startMapletValue( const int count );
	void endMapletValue( const int count );
	void endMap();

private:

	//	Relies on null termination, which is potentially dodgy - except this
	//	is only for formatting printable characters.
	void refStringPrint( Ref * str_K ) ;

	void refListPrint( Ref sofar );

	void refVectorPrint( Ref * vec_K );

	void refMapPrint( Ref * r );
	void refMapletPrint( Ref key, Ref value );

	void refMixedPrint( Ref * mix_K );

	enum Tag {
		TAG_OPEN,
		TAG_CLOSE,
		TAG_EMPTY
	};

	void refAttrMapPrint( Ref * attrmap_K, enum Tag flag );

	void refElementPrint( Ref * mix_K );

	void refRecordPrint( Ref * rec_K );

	void refExternalPrint( const Ref * obj_K );
	void refWRecordPrint( Ref * rec_K );

	//	<TYPE FIELD ...>
	void refInstancePrint( Ref * rec_K );

	void refDoublePrint( const Ref r );
	
	void refKeyPrint( const Ref r );

	void refFunctionPrint( const Ref fn );

public:
	void refPrint( const Ref r );

};	//	end class.

#endif