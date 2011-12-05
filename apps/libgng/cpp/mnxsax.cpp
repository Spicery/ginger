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

#include <iostream>
#include <cctype>
#include <string>
#include <map>
#include <sstream>

#include "mnxsax.hpp"
#include "mishap.hpp"

//#include "sys.hpp"
#include "mishap.hpp"
//#include "facet.hpp"


//- SAX Parser -----------------------------------------------------------------

namespace Ginger {

using namespace std;

char MnxSaxParser::nextChar() {
	char c;
	if ( not input.get( c ) ) throw Mishap( "Cannot read from file (unexpected end?)" );
	return c;
}

char MnxSaxParser::peekChar() {
	char c = input.peek();
	if ( not input ) throw Mishap( "Cannot read from file (unexpected end?)" );
	return c;
}

void MnxSaxParser::mustReadChar( const char ch_want ) {
	char c_got;
	if ( not ( input.get( c_got ) ) ) throw Mishap( "Cannot read from file (unexpected end?)" );
	if ( c_got != ch_want ) throw Mishap( "Unexpected character" ).culprit( "Wanted", ch_want ).culprit( "Received", c_got );
}

void MnxSaxParser::eatWhiteSpace() {
	while ( input.good() ) {
		char ch;
		if ( not input.get( ch ) ) break;
		if ( not isspace( ch ) ) {
			input.putback( ch );
			break;
		}
	}
}

static bool is_name_char( const char ch ) {
	return isalnum( ch ) || ch == '-' || ch == '.';
}

void MnxSaxParser::readName( std::string & name ) {
	char ch;
	while ( (ch = nextChar()) && is_name_char( ch ) ) {
		//std::cout << "pushing '" << ch << "'"<< endl;
		name.push_back( ch );
		//std::cout << "name so far '" << name << "'" << endl;
	}
	input.putback( ch );
}

void MnxSaxParser::readAttributeValue( std::string & attr ) {
	const char q = this->nextChar();
	if ( q != '"' && q != '\'' ) throw Mishap( "Attribute value not quoted" ).culprit( "Character", q );
	for (;;) {
		char ch = this->nextChar();
		if ( ch == q ) break;
		if ( ch == '&' ) {
			std::string esc;
			while ( ch = this->nextChar(), ch != ';' ) {
				//std::cout << "char " << ch << endl;
				esc.push_back( ch );
				if ( esc.size() > 4 ) {
					throw Mishap( "Malformed escape" );
				}
			}
			if ( esc == "lt" ) {
				attr.push_back( '<' );
			} else if ( esc == "gt" ) {
				attr.push_back( '>' );
			} else if ( esc == "amp" ) {
				attr.push_back( '&' );
			} else if ( esc == "quot" ) {
				attr.push_back( '"' );
			} else if ( esc == "apos" ) {
				attr.push_back( '\'' );
			} else {
				throw Mishap( "Unexpected escape sequence after &" ).culprit( "Character", esc );
			}
		} else {
			attr.push_back( ch );
		}
	}
}	


void MnxSaxParser::processAttributes(
	//std::map< std::string, std::string > & attrs
) {
	//	Process attributes
	for (;;) {
		this->eatWhiteSpace();
		char c = peekChar();
		if ( c == '/' || c == '>' ) {
			break;
		}
		std::string key;
		this->readName( key );
		this->eatWhiteSpace();
		this->mustReadChar( '=' );
		this->eatWhiteSpace();
		std::string value;
		this->readAttributeValue( value );
		this->parent.put( key, value );
	}
}


void MnxSaxParser::read() {
	if ( this->pending_end_tag ) {
		this->parent.endTag( this->tag_name );
		this->pending_end_tag = false;
		this->level -= 1;
		return;
	}
	
	this->input >> noskipws;
	
	this->eatWhiteSpace();
	if ( not this->input.good() ) {
		if ( this->finished ) {
			throw Mishap( "Unexpected end of file" );
		}
		this->finished = true;
		return;
	}
	this->mustReadChar( '<' );
		
	char ch = nextChar();
	if ( ch == '/' ) {
		std::string end_tag;
		this->readName( end_tag );
		this->eatWhiteSpace();
		this->mustReadChar( '>' );
		this->parent.endTag( end_tag );
		this->level -= 1;
		return;
	} else if ( ch == '!' ) {
		if ( '-' != nextChar() || '-' != nextChar() ) throw Mishap( "Invalid XML comment syntax" );
		char ch = nextChar();
		for (;;) {
			char prev_ch = ch;
			ch = nextChar();
			if ( prev_ch == '-' && ch == '-' ) break;
		}
		if ( '>' != nextChar() ) throw Mishap( "Invalid XML comment syntax" );
		this->read();
		return;
	} else if ( ch == '?' ) {
		for (;;) {
			char prev = ch;
			ch = nextChar();
			if ( prev == '?' && ch == '>' ) break;
		}
		this->read();
		return;
	} else {
		//cout << "Ungetting '" << ch << "'" << endl;
		input.putback( ch );
	}
	
	this->tag_name.resize( 0 );
	this->readName( this->tag_name );
	this->parent.startTagOpen( this->tag_name );
	
	//std::map< std::string, std::string > attributes;
	this->processAttributes();// attributes );
	this->parent.startTagClose( this->tag_name );
	
	this->eatWhiteSpace();
	
	
	ch = nextChar();
	if ( ch == '/' ) {
		this->mustReadChar( '>' );
		this->pending_end_tag = true;
		this->level += 1;
		//this->parent.endTag( name ); <- this code replaced
		return;
	} else if ( ch == '>' ) {
		this->level += 1;
		return;
	} else {
		throw Mishap( "Invalid continuation" );
	}
			
}

void MnxSaxParser::readElement() { 
	do {
		this->read();
	} while ( this->level != 0 );
}



} // namespace
