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

#include "sax.hpp"
#include "mishap.hpp"

//#include "sys.hpp"
#include "mishap.hpp"
//#include "facet.hpp"


//- SAX Parser -----------------------------------------------------------------

namespace Ginger {

using namespace std;

char SaxParser::nextChar() {
	char c;
	if ( not input.get( c ) ) throw;
	return c;
}

char SaxParser::peekChar() {
	char c = input.peek();
	if ( not input ) throw;
	return c;
}

void SaxParser::mustReadChar( const char ch_want ) {
	char c_got;
	if ( not ( input.get( c_got ) ) ) throw;
	if ( c_got != ch_want ) throw;
}

void SaxParser::eatWhiteSpace() {
	char ch;
	while ( input.get( ch ) && isspace( ch ) ) {
		//	Skip.
	}
	//cout << "Ungetting '" << ch << "'" << endl;
	input.putback( ch );
}

static bool is_name_char( const char ch ) {
	return isalnum( ch ) || ch == '-' || ch == '.';
}

void SaxParser::readName( std::string & name ) {
	char ch;
	while ( (ch = nextChar()) && is_name_char( ch ) ) {
		//std::cout << "pushing '" << ch << "'"<< endl;
		name.push_back( ch );
		//std::cout << "name so far '" << name << "'" << endl;
	}
	input.putback( ch );
}

void SaxParser::readAttributeValue( std::string & attr ) {
	this->mustReadChar( '\"' );
	for (;;) {
		char ch = this->nextChar();
		if ( ch == '\"' ) break;
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
				throw;
			}
		} else {
			attr.push_back( ch );
		}
	}
}	


void SaxParser::processAttributes(
	std::map< std::string, std::string > & attrs
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
		attrs[ key ] = value;
	}
}


void SaxParser::read() {
	this->input >> noskipws;
	
	this->eatWhiteSpace();
	if ( this->input.eof() ) return;
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
	
	std::string name;
	this->readName( name );
	
	std::map< std::string, std::string > attributes;
	this->processAttributes( attributes );
	
	this->eatWhiteSpace();
	
	
	ch = nextChar();
	if ( ch == '/' ) {
		this->mustReadChar( '>' );
		this->parent.startTag( name, attributes );
		this->parent.endTag( name );
		return;
	} else if ( ch == '>' ) {
		this->parent.startTag( name, attributes );
		this->level += 1;
		return;
	} else {
		throw;
	}
			
}

void SaxParser::readElement() { 
	do {
		this->read();
	} while ( this->level != 0 );
}



} // namespace
