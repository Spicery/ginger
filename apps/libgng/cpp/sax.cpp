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
#include <cstdio>

#include "sax.hpp"
#include "mishap.hpp"

#include "mishap.hpp"



namespace Ginger {

using namespace std;

//- IStreamSaxInput ------------------------------------------------------------


IStreamSaxInput::IStreamSaxInput( std::istream & input ) : input( input ) { 	
	this->input >> noskipws; 
}


void IStreamSaxInput::mustReadChar( const char ch_want ) {
	char c_got;
	if ( not ( input.get( c_got ) ) ) throw Mishap( "Cannot read from file (unexpected end?)" );
	if ( c_got != ch_want ) throw Mishap( "Unexpected character" ).culprit( "Wanted", ch_want ).culprit( "Received", c_got );
}

bool IStreamSaxInput::tryReadChar( const char ch_want ) {
	char c_got;
	if ( not ( input.get( c_got ) ) ) return false;
	if ( c_got != ch_want ) input.putback( c_got );
	return c_got == ch_want;
}

char IStreamSaxInput::nextChar() {
	char c;
	if ( not input.get( c ) ) throw Mishap( "Cannot read from file (unexpected end?)" );
	return c;		
}

char IStreamSaxInput::peekChar() {
	char c = input.peek();
	if ( not input ) throw Mishap( "Cannot read from file (unexpected end?)" );
	return c;
}

void IStreamSaxInput::pushChar( const char ch ) {
	input.putback( ch );
}

bool IStreamSaxInput::isEof() {
	return input.eof();
}


//- FileStreamSaxInput ---------------------------------------------------------

FileStreamSaxInput::FileStreamSaxInput( FILE * file ) : file( file ) {
}

void FileStreamSaxInput::mustReadChar( const char ch_want ) {
	const int ch = fgetc( this->file );
	if ( ch == EOF ) throw Mishap( "Cannot read from file (unexpected end?)" );
	if ( ch != ch_want ) throw Mishap( "Unexpected character" ).culprit( "Wanted", ch_want ).culprit( "Received", (char)ch );
}

bool FileStreamSaxInput::tryReadChar( const char ch_want ) {
	const int ch = fgetc( this->file );
	if ( ch == EOF ) return false;
	if ( ch != ch_want ) ungetc( ch, this->file );
	return ch == ch_want;
}

char FileStreamSaxInput::nextChar() {
	const int ch = fgetc( this->file );
	if ( ch == EOF ) throw Mishap( "Cannot read from file (unexpected end?)" );
	return ch;		
}

char FileStreamSaxInput::peekChar() {
	const int ch = fgetc( this->file );
	if ( ch == EOF ) throw Mishap( "Cannot read from file (unexpected end?)" );
	ungetc( ch, this->file );
	return ch;
}

void FileStreamSaxInput::pushChar( const char ch ) {
	ungetc( ch, this->file );
}

bool FileStreamSaxInput::isEof() {
	return feof( this->file ) != 0;
}


//- SAX Parser -----------------------------------------------------------------


void SaxParser::eatWhiteSpace() {
	for (;;) {
		char ch = input->nextChar();
		if ( not isspace( ch ) ) {
			input->pushChar( ch );
			break;
		}
	}
}

static bool is_name_char( const char ch ) {
	return isalnum( ch ) || ch == '-' || ch == '.';
}

void SaxParser::readName( std::string & name ) {
	for (;;) {
		char ch = input->nextChar();
		if ( not is_name_char( ch ) ) {
			input->pushChar( ch );
			break;
		}
		name.push_back( ch );
	}
}


void SaxParser::readAttributeValue( std::string & attr ) {
	const char qch = input->nextChar();
	if ( qch != '\'' && qch != '"' ) {
		throw Mishap( "Needed quote" ).culprit( "Character received", qch );
	}
	for (;;) {
		char ch = input->nextChar();
		if ( ch == qch ) break;
		if ( ch == '&' ) {
			std::string esc;
			while ( ch = input->nextChar(), ch != ';' ) {
				//std::cout << "char " << ch << endl;
				esc.push_back( ch );
				if ( esc.size() > 4 ) {
					throw Mishap( "Malformed escape sequence" ).culprit( "Sequence so far", esc );
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
			} else if ( esc.size() >= 2 && esc[ 0 ] == '#' ) {
				if ( esc[1] == 'x' ) {
					stringstream s( &esc[2] );
					unsigned int n;
					if ( s >> std::hex >> n && n == static_cast< char >( n ) ) {
						attr.push_back( static_cast< char >( n ) );
					} else {
						throw Mishap( "Unexpected numeric sequence after &#" ).culprit( "Sequence", esc );
					}					
				} else {
					stringstream s( &esc[1] );
					unsigned int n;
					if ( s >> n && n == static_cast< char >( n ) ) {
						attr.push_back( static_cast< char >( n ) );
					} else {
						throw Mishap( "Unexpected numeric sequence after &#" ).culprit( "Sequence", esc );
					}
				}
			} else {
				throw Mishap( "Unrecognised escape sequence" ).culprit( "Escape sequence", esc );
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
		char c = input->peekChar();
		if ( c == '/' || c == '>' ) {
			break;
		}
		std::string key;
		this->readName( key );
		this->eatWhiteSpace();
		input->mustReadChar( '=' );
		this->eatWhiteSpace();
		std::string value;
		this->readAttributeValue( value );
		attrs[ key ] = value;
	}
}


void SaxParser::read() {
	//this->input >> noskipws;
	
	this->eatWhiteSpace();
	if ( input->isEof() ) {
		if ( this->finished ) {
			throw Mishap( "Unexpected end of file" );
		}
		this->finished = true;
		return;
	}
	input->mustReadChar( '<' );
		
	char ch = input->nextChar();
	if ( ch == '/' ) {
		std::string end_tag;
		this->readName( end_tag );
		this->eatWhiteSpace();
		input->mustReadChar( '>' );
		this->parent.endTag( end_tag );
		this->level -= 1;
		return;
	} else if ( ch == '!' ) {
		if ( '-' != input->nextChar() || '-' != input->nextChar() ) throw Mishap( "Invalid XML comment syntax" );
		char ch = input->nextChar();
		for (;;) {
			char prev_ch = ch;
			ch = input->nextChar();
			if ( prev_ch == '-' && ch == '-' ) break;
		}
		if ( '>' != input->nextChar() ) throw Mishap( "Invalid XML comment syntax" );
		this->read();
		return;
	} else if ( ch == '?' ) {
		for (;;) {
			char prev = ch;
			ch = input->nextChar();
			if ( prev == '?' && ch == '>' ) break;
		}
		this->read();
		return;
	} else {
		//cout << "Ungetting '" << ch << "'" << endl;
		input->pushChar( ch );
	}
	
	std::string name;
	this->readName( name );
	
	std::map< std::string, std::string > attributes;
	this->processAttributes( attributes );
	
	this->eatWhiteSpace();
	
	
	ch = input->nextChar();
	if ( ch == '/' ) {
		input->mustReadChar( '>' );
		this->parent.startTag( name, attributes );
		this->parent.endTag( name );
		return;
	} else if ( ch == '>' ) {
		this->parent.startTag( name, attributes );
		this->level += 1;
		return;
	} else {
		throw Mishap( "Invalid continuation" );
	}
			
}

void SaxParser::readElement() { 
	do {
		this->read();
	} while ( this->level != 0 );
}



} // namespace
