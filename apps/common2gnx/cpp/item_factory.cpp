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

#include <iostream>
#include <cstdio>
#include <cassert>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>

#include "item_factory.hpp"
#include "item_map.hpp"
#include "item.hpp"
#include "role.hpp"
#include "mishap.hpp"

using namespace std;

int Source::peekchar() {
	if ( this->buffer.empty() ) {
		const int ch = getc( this->file );
		this->buffer.push_front( ch );
		return ch;
	} else {
		return this->buffer.front();
	}
}

int Source::nextchar() {
	int ch;
	if ( this->buffer.empty() ) {
		ch = getc( this->file );
	} else {
		ch = this->buffer.front();
		this->buffer.pop_front();		
	}
	if ( ch == '\n' ) {
		this->lineno += 1;
	}
	return ch;
}

void Source::pushchar( const int ch ) {
	if ( ch == '\n' ) {
		this->lineno -= 1;
	}
	this->buffer.push_front( ch );
}

int ItemFactoryClass::trychar( int ch ) {
    const int c = this->nextchar();
    if ( c == ch ) {
        return 1;
    } else {
    	this->pushchar( c );
        return 0;
    }
}

void ItemFactoryClass::drop() {
    this->peeked = false;
}

void ItemFactoryClass::unread() {
    this->peeked = true;
}

enum CharType {
	MiscCharType,
	LineBreakCharType,
	QuoteCharType,
	UnderbarCharType,
	//SeparatorCharType,
	BracketCharType,
	BracketDecorationCharType,
	SelfGlueCharType,
	SignCharType
};

CharType charType( const char ch ) {
	switch ( ch ) {
		case '\n':
		case '\r': 
			return LineBreakCharType;
		case '"':
		case '\'':
		case '`':
			return QuoteCharType;
		case '_': 
			return UnderbarCharType;
		//case ',':
		//case ';': 
		//	return SeparatorCharType;
		case '(': case ')':
		case '[': case ']':
		case '{': case '}':
			return BracketCharType;
		case '%':
			return BracketDecorationCharType;
		case ',':
		case ';':
		case '.':
		case '@':
			//	TODO: Should '.' and '@' be self-glue or sign?
			return SelfGlueCharType;
		default:
			return ispunct( ch ) ? SignCharType : MiscCharType;	
	}
}

bool isSignCharType( const char ch ) {
	return charType( ch ) == SignCharType;
}

int ItemFactoryClass::eatsWhiteSpaceAndComments() {
	int ch;
    for(;;) {
        ch = this->nextchar();
        if ( isspace( ch ) ) continue;
        if ( ch == '#' || ( this->cstyle_mode && ch == '/' && this->trychar( '/' ) ) ) {
            do
                ch = this->nextchar();
            while ( ch != '\n' && ch != '\r' );
        } else if ( ch == '/' && this->cstyle_mode && this->trychar( '*' ) ) {
        	this->nextchar();
        	for (;;) {
        		ch = this->nextchar();
        		if ( ch == '*' && this->trychar( '/' ) ) {
        			this->nextchar();
        			break;
        		}
        	}
        } else {
	        break;
	    }
    }
    return ch;
}

void ItemFactoryClass::readAtEndOfFile() {
	Item it = this->item = this->spare;
	it->role = EofRole;
	it->tok_type = tokty_eof;
	it->nameString() = "<eof>";
}

void ItemFactoryClass::readAtDigitOrSign( int ch ) {
	//
	//	Number
	//
	bool digit_seen = false;
	bool decimal_point_seen = false;

	do {
		this->text.push_back( (char)ch );
		if ( isdigit( ch ) ) digit_seen = true;
		ch = this->nextchar();
	} while ( isdigit( ch ) );

	if ( ch == '.' ) {
		//	IF the next character is a digit then we can proceed safely.
		//	Otherwise we treat the '.' as postfix function application.
		if ( isdigit( this->peekchar() ) ) {
			decimal_point_seen = true;
			do {
				this->text.push_back( (char)ch );
				if ( isdigit( ch ) ) digit_seen = true;
				ch = this->nextchar();
			} while ( isdigit( ch ) );
		}
	}

	if ( ch == '%' ) {
		//	Percentages count as doubles.
		this->text.push_back( (char)ch );
		ch = this->nextchar();
		decimal_point_seen = true;
	}
	
	this->pushchar( ch );

	if ( not digit_seen ) {
		this->item = this->itemMap.lookup( this->text );
	} else {
		Item it = this->item = this->spare;
		it->tok_type = decimal_point_seen ? tokty_double : tokty_int;
		it->role = LiteralRole;
		it->nameString() = this->text;
	}
}

void ItemFactoryClass::readAtAlpha( int ch ) {
	//	
	//	Variables and keywords
	//	
	do {
		this->text.push_back( ch );
		ch = this->nextchar();
	} while ( isalnum( ch ) || ch == '_' );
	this->pushchar( ch );
	this->item = this->itemMap.lookup( this->text );
	if ( this->item == NULL ) {
		Item it = this->item = this->spare;
		it->tok_type = tokty_id;
		it->role = PrefixRole;
		it->nameString() = this->text;
	}
	this->item->is_name = true;
}

void ItemFactoryClass::readAtUnderbar( int ch ) {
	//	
	//	Anonymous variables.
	//	
	assert( ch == '_' ); 
	do {
		this->text.push_back( ch );
		ch = this->nextchar();
	} while ( isalnum( ch ) || ch == '_' );
	this->pushchar( ch );
	Item it = this->item = this->spare;
	it->tok_type = tokty_anon;
	it->role = PrefixRole;
	it->nameString() = this->text;
}

void ItemFactoryClass::readAtQuoteCharType( int ch ) {
	//
	//	Strings and Character sequences
	//
	int open_quote = ch;
	while ( ch = this->nextchar(), ch != open_quote && ch != '\n' ) {
		if ( ch == '\\' ) {
			ch = this->nextchar();
			if ( ch == '\\' ) {
				ch = '\\';
			} else if ( ch == '"' ) {
				//	nothing
			} else if ( ch == 'n' ) {
				ch = '\n';
			} else if ( ch == 'r' ) {
				ch = '\r';
			} else if ( ch == 't' ) {
				ch = '\t';
			} else if ( ch == 's' ) {
				ch = ' ';
			} else if ( ch == '&' ) {
				//	HTML entity.
				string entity;
				for (;;) {
					ch = this->nextchar();
					if ( ch == ';' ) break;
					if ( isalpha( ch ) ) {
						entity.push_back( (char)ch );
					} else {
						throw CompileTimeError( "Unexpected character while reading HTML character entity" ).culprit( "Char", (char)ch );
					}
				}
				
				//	TO-DO: this needs a lot of expansion but should wait until
				//	the Unicode sprint is done.
				if ( entity == "lt" ) {
					this->text.append( "<" );
				} else if ( entity == "gt" ) {
					this->text.append( ">" );
				} else if ( entity == "amp" ) {
					this->text.append( "&" );
				} else {
					throw CompileTimeError( "Unrecognised HTML character entity" ).culprit( "Entity", entity );
				}
				continue;
			} else if ( ch == '(' ) {
				//	This should be a string interpolation. However, we are not
				//	implementing that in this first release, so what we will do
				//	is raise a warning.
				std::cerr << "STRING INTERPOLATION NOT IMPLEMENTED YET" << std::endl;
			} else {
				throw CompileTimeError( "Unexpected character after \\ in string" ).culprit( "Character", (char)ch );	// ch
			}
		}
		this->text.push_back( ch );
	}
	if ( ch == '\n' ) {
		throw CompileTimeError( "Unterminated string" ).culprit( "String so far", this->text );
	}
	Item it = this->item = this->spare;
	it->tok_type = ( open_quote == '"' ) ? tokty_string : ( open_quote == '\'' ) ? tokty_charseq : tokty_symbol;
	it->role = it->tok_type == tokty_string ? StringRole : it->tok_type == tokty_symbol ? SymbolRole : CharSeqRole;
	it->nameString() = this->text;
}

//  void ItemFactoryClass::readAtSeparatorCharType( int ch ) {
//  	//
//  	//	Self-glue character keywords.
//  	//
//  	this->text.push_back( ch );
//  	this->item = this->itemMap.lookup( this->text );
//  	if ( this->item == NULL ) {
//  		//	Never happens.
//  		throw CompileTimeError( "Invalid punctuation token" ); 
//  	}
//  }

void ItemFactoryClass::readAtBracketCharType( int ch ) {
	this->text.push_back( ch );
	if ( ( ch = this->nextchar() ) == '%' ) {
		this->text.push_back( ch );
	} else {
		this->pushchar( ch );
	}
	this->item = this->itemMap.lookup( this->text );
	if ( this->item == NULL ) {
		throw CompileTimeError( "Invalid punctuation token" ); 
	}
}

void ItemFactoryClass::readAtBracketDecorationCharType( int ch ) {
	this->text.push_back( ch );
	if ( strchr( "()[]{}", ( ch = this->nextchar() ) ) ) {    	
		this->text.push_back( ch );
	} else {
		this->pushchar( ch );
	}
	this->item = this->itemMap.lookup( this->text );
	if ( this->item == NULL ) {
		throw CompileTimeError( "Invalid punctuation token" ); 
	}
}

void ItemFactoryClass::readAtSelfGlueCharType( int ch ) {
	//	
	//	Signs made of a single repeating character.
	//		I wonder if semi-colon and comma should be in 
	//		this section?
	//
	this->text.push_back( ch );
	for (;;) {
		const int nextch = this->nextchar();
		if ( nextch == ch ) {
			this->text.push_back( ch );
		} else {
			this->pushchar( nextch );
			break;
		}
	}
	Item it = this->item = this->itemMap.lookup( this->text );
	if ( it == NULL ) {
		throw CompileTimeError( "Invalid repeated-character sign" ).culprit( "Token", this->text );
	}
}

void ItemFactoryClass::readAtSignCharType( int ch ) {
	Item it;
	//
	//	Other punctuation treated as multi-character keywords.
	//
	//  ispunct()
	//      checks  for  any printable character which is not a
	//      space or an alphanumeric character.
	//
	//	Note that >< is not a legal glue sequence; this allows us
	//	to write <foo></bar>, for example.
	int prev_ch;
	do {
		prev_ch = ch;
		this->text.push_back( ch );
		ch = this->nextchar();
	} while ( isSignCharType( ch ) && not( prev_ch == '>' && ch == '<') );
	this->pushchar( ch );
	it = this->item = this->itemMap.lookup( this->text );
	if ( it == NULL ) {
		throw CompileTimeError( "Invalid sign (combination of special characters)" ).culprit( "Sign", this->text );
	}
}

Item ItemFactoryClass::read() {
    if ( this->peeked ) {
        this->peeked = false;
        return this->item;
    }

	int ch = this->eatsWhiteSpaceAndComments();
    this->text.clear();

    if ( ch == EOF ) {
		this->readAtEndOfFile();
    } else if ( isdigit( ch ) || ( ( ch == '-' || ch == '+' ) && isdigit( this->peekchar() ) ) ) {
    	this->readAtDigitOrSign( ch );
	} else if ( isalpha( ch ) ) {
		this->readAtAlpha( ch );
	} else if ( ch == '_' ) {
		this->readAtUnderbar( ch );
    } else {
    	switch ( charType( ch ) ) {
    		case QuoteCharType:
				this->readAtQuoteCharType( ch );
				break;
    		//case SeparatorCharType:
    		//	this->readAtSeparatorCharType( ch );
			//	break;
			case BracketCharType:
    			this->readAtBracketCharType( ch );
				break;
    		case BracketDecorationCharType:
				this->readAtBracketDecorationCharType( ch );
				break;
			case SelfGlueCharType:
				this->readAtSelfGlueCharType( ch );
				break;			
    		case SignCharType:
    			this->readAtSignCharType( ch );
				break;
    		default:
				//	Definite error
				throw "Invalid character (%c)"; //  ch );
    	}
	}
    return this->item;
}

Item ItemFactoryClass::peek() {
    Item it = this->read();
    this->peeked = true;
    return it;
}

void Source::resetSource() {
	FILE *f = this->file;
	int fd = fileno( f );
	int fd1 = dup( fd );
	fclose( f );
	this->file = fdopen( fd1, "r" );
}

void ItemFactoryClass::reset() {
	this->resetSource();
	this->peeked = false;
	this->text.clear();
}
