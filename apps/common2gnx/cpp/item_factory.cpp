#include <string>
using namespace std;

#include <iostream>
//#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "item_factory.hpp"

#include "item_map.hpp"
#include "item.hpp"
#include "role.hpp"
#include "mishap.hpp"


int ItemFactoryClass::peekchar() {
	int c = getc( this->file );
	ungetc( c, this->file );
	return c;
}

int ItemFactoryClass::trychar( int ch ) {
    int c = getc( this->file );
    if ( c == ch ) {
        return 1;
    } else {
        ungetc( c, this->file );
        return 0;
    }
}

void ItemFactoryClass::drop() {
    this->peeked = false;
}

void ItemFactoryClass::unread() {
    this->peeked = true;
}

/*std::string ItemFactoryClass::readURL() {
    int ch;

    if ( this->peeked ) {
        throw Ginger::Mishap( "Trying to read URL after pushback" );
    }
    
    //  Get rid of white space and comments.
    for(;;) {
        ch = getc( this->file );
        if ( isspace( ch ) ) continue;
        if ( ch == '#' || ( ch == '/' && this->trychar( '/' ) ) ) {
            do
                ch = getc( this->file );
            while ( ch != '\n' && ch != '\r' );
            continue;
        }
        break;
    }
	
    this->text.clear();
    this->text.push_back( (char)ch );

    for(;;) {
        ch = getc( this->file );
        if ( isspace( ch ) ) break;
        this->text.push_back( (char)ch );
	}		
	
	return text;
}*/

enum CharType {
	MiscCharType,
	LineBreakCharType,
	QuoteCharType,
	UnderbarCharType,
	SeparatorCharType,
	BracketCharType,
	BracketDecorationCharType,
	SelfGlueCharType,
	SignCharType
};

CharType char_type( const char ch ) {
	switch ( ch ) {
		case '\n':
		case '\r': 
			return LineBreakCharType;
		case '"':
		case '\'': 
			return QuoteCharType;
		case '_': 
			return UnderbarCharType;
		case ',':
		case ';': 
			return SeparatorCharType;
		case '(': case ')':
		case '[': case ']':
		case '{': case '}':
			return BracketCharType;
		case '%':
			return BracketDecorationCharType;
		case '.':
		case '@':
			return SelfGlueCharType;
		default:
			return ispunct( ch ) ? SignCharType : MiscCharType;	
	}
}

bool isSignCharType( const char ch ) {
	return char_type( ch ) == SignCharType;
}

int ItemFactoryClass::eatsWhiteSpaceAndComments() {
	int ch;
    for(;;) {
        ch = getc( this->file );
        if ( isspace( ch ) ) continue;
        if ( ch == '#' || ( ch == '/' && this->trychar( '/' ) ) ) {
            do
                ch = getc( this->file );
            while ( ch != '\n' && ch != '\r' );
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

void ItemFactoryClass::readAtDigitOrMinus( int ch ) {
	//
	//	Number
	//
	do {
		this->text.push_back( (char)ch );
		ch = getc( this->file );
	} while ( isdigit( ch ) );
	ungetc( ch, this->file );
	if ( this->text.size() == 1 && this->text[ 0 ] == '-' ) {
		this->item = this->itemMap.lookup( this->text );
	} else {
		Item it = this->item = this->spare;
		it->tok_type = tokty_int;
		it->role = LiteralRole;
		it->nameString() = this->text;
	}
}

void ItemFactoryClass::readAtAlphaOrUnderbar( int ch ) {
	//	
	//	Variables and keywords
	//	
	do {
		this->text.push_back( ch );
		ch = getc( this->file );
	} while ( isalnum( ch ) || ch == '_' );
	ungetc( ch, this->file );
	this->item = this->itemMap.lookup( this->text );
	if ( this->item == NULL ) {
		Item it = this->item = this->spare;
		it->tok_type = tokty_id;
		it->role = PrefixRole;
		it->nameString() = this->text;
	}
}

void ItemFactoryClass::readAtQuoteCharType( int ch ) {
	//
	//	Strings and Character sequences
	//
	int open_quote = ch;
	while ( ch = getc( this->file ), ch != open_quote && ch != '\n' ) {
		if ( ch == '\\' ) {
			ch = getc( this->file );
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
					ch = getc( this->file );
					if ( ch == ';' ) break;
					if ( isalpha( ch ) ) {
						entity.push_back( (char)ch );
					} else {
						throw Ginger::Mishap( "Unexpected character while reading HTML character entity" ).culprit( "Char", (char)ch );
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
					throw Ginger::Mishap( "Unrecognised HTML character entity" ).culprit( "Entity", entity );
				}
				continue;
			} else if ( ch == '(' ) {
				//	This should be a string interpolation. However, we are not
				//	implementing that in this first release, so what we will do
				//	is raise a warning.
				std::cerr << "STRING INTERPOLATION NOT IMPLEMENTED YET" << std::endl;
			} else {
				throw Ginger::Mishap( "Unexpected character after \\ in string" ).culprit( "Character", (char)ch );	// ch
			}
		}
		this->text.push_back( ch );
	}
	if ( ch == '\n' ) {
		throw "unterminated string (%s)";
	}
	Item it = this->item = this->spare;
	it->tok_type = open_quote == '"' ? tokty_string : tokty_charseq;
	it->role = it->tok_type == tokty_string ? StringRole : CharSeqRole;
	it->nameString() = this->text;
}

void ItemFactoryClass::readAtSeparatorCharType( int ch ) {
	//
	//	Single character keywords
	//
	this->text.push_back( ch );
	this->item = this->itemMap.lookup( this->text );
	if ( this->item == NULL ) {
		//	Never happens.
		throw Ginger::Mishap( "Invalid punctuation token" ); 
	}
}

void ItemFactoryClass::readAtBracketCharType( int ch ) {
	do {
		this->text.push_back( ch );
	} while ( ( ch = getc( this->file ) ) == '%' );
	ungetc( ch, this->file );
	this->item = this->itemMap.lookup( this->text );
	if ( this->item == NULL ) {
		throw Ginger::Mishap( "Invalid punctuation token" ); 
	}
}

void ItemFactoryClass::readAtBracketDecorationCharType( int ch ) {
	do {
		this->text.push_back( ch );
	} while ( ( ch = getc( this->file ) ) == '%' );
	if ( strchr( "()[]{}", ch ) ) {    	
		this->text.push_back( ch );
	} else {
		ungetc( ch, this->file );
	}
	this->item = this->itemMap.lookup( this->text );
	if ( this->item == NULL ) {
		throw Ginger::Mishap( "Invalid punctuation token" ); 
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
		char nextch = getc( this->file );
		if ( nextch == ch ) {
			this->text.push_back( ch );
		} else {
			ungetc( nextch, this->file );
			break;
		}
	}
	Item it = this->item = this->itemMap.lookup( this->text );
	if ( it == NULL ) {
		throw Ginger::Mishap( "Invalid repeated-character sign" ).culprit( "Token", this->text );
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
		ch = getc( this->file );
	} while ( isSignCharType( ch ) && not( prev_ch == '>' && ch == '<') );
	ungetc( ch, this->file );
	it = this->item = this->itemMap.lookup( this->text );
	if ( it == NULL ) {
		throw Ginger::Mishap( "Invalid sign (combination of special characters)" ).culprit( "Sign", this->text );
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
    } else if ( isdigit( ch ) || ( ch == '-' && isdigit( this->peekchar() ) ) ) {
    	this->readAtDigitOrMinus( ch );
	} else if ( isalpha( ch ) || ch == '_' ) {
		this->readAtAlphaOrUnderbar( ch );
    } else {
    	switch ( char_type( ch ) ) {
    		case QuoteCharType:
				this->readAtQuoteCharType( ch );
				break;
    		case SeparatorCharType:
    			this->readAtSeparatorCharType( ch );
				break;
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

void ItemFactoryClass::reset() {

	{
	    //	Need to zap the input buffer.  This is a bit laborious
	    //	but the best I could find in 10 minutes ...
	    FILE *f = this->file;
	    int fd = fileno( f );
	    int fd1 = dup( fd );
	    fclose( f );
	    this->file = fdopen( fd1, "r" );
	}

	this->peeked = false;
	this->text.clear();
}
