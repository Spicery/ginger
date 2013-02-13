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

//#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "item_factory.hpp"

#include "item.hpp"
#include "mishap.hpp"

using namespace std;
using namespace Ginger;


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

Item ItemFactoryClass::read() {
    int ch;

    if ( this->peeked ) {
        this->peeked = false;
        return this->item;
    }

    //  Get rid of white space and comments.
    //	There's no very good reason for it being coded in this strange way - just creep.
    for(;;) {
        ch = getc( this->file );
        if ( isspace( ch ) ) continue;
        if ( ch == '/' && this->trychar( '/' ) ) {
            do
                ch = getc( this->file );
            while ( ch != '\n' && ch != '\r' );
            continue;
        }
        if ( ch == '/' && this->trychar( '*' ) ) {
            do
                ch = getc( this->file );
            while ( ch != '*' || !this->trychar( '/' ) );
            continue;
        }
        break;
    }

    this->text.clear();

    if ( ch == EOF ) {
		//
		//	End of file
		//
		this->item->tok_type = tokty_eof;
		this->item->name = "<eof>";
    } else if ( isdigit( ch ) || ( ch == '-' && isdigit( this->peekchar() ) ) ) {
		//
		//	Number
		//
        do {
            this->text.push_back( (char)ch );
            ch = getc( this->file );
        } while ( isdigit( ch ) );
        ungetc( ch, this->file );
		if ( this->text.size() == 1 && this->text[ 0 ] == '-' ) {
			this->item->tok_type = tokty_name;
			this->item->name = this->text;
		} else {
			this->item->tok_type = tokty_number;
            this->item->name = this->text;
		}
    } else if ( ch == '"' || ch == '\'' || ch == '`' ) {
		//
		//	Strings and Character sequences
		//
		int open_quote = ch;
        while ( ch = getc( this->file ), ch != open_quote && ch != '\n' ) {
			if ( ch == '\\' ) {
				ch = getc( this->file );
				if ( ch == '\\' ) {
					ch = '\\';
				} else if ( ch == '"' || ch == '\'' || ch == '`' ) {
					//	nothing
				} else if ( ch == 'n' ) {
					ch = '\n';
				} else if ( ch == 'r' ) {
					ch = '\r';
				} else if ( ch == 't' ) {
					ch = '\t';
				} else if ( ch == 's' ) {
					ch = ' ';
				} else {
					throw "Invalid character after \\ in string";	// ch
				}
			}
            this->text.push_back( ch );
        }
        if ( ch == '\n' ) {
            throw "unterminated string (%s)";
        }
		this->item->tok_type = ch == '\''  ? tokty_charseq : ch == '`' ? tokty_symbol : tokty_string;
		this->item->name = this->text;
    } else if ( isalpha( ch ) || ch == '_' ) {
		//	
		//	Variables and keywords
		//	
        do {
            this->text.push_back( ch );
            ch = getc( this->file );
        } while ( isalnum( ch ) || ch == '_' );
        ungetc( ch, this->file );
		this->item->tok_type = tokty_name;
		this->item->name = this->text;
    } else if ( strchr( "?,=<>()[]{}:", ch ) ) {
		//
		//	Single character keywords
		//
	    this->text.push_back( ch );
		this->item->tok_type = tokty_sign;
		this->item->name = this->text;
    } else {
		//
		//	Definite error
		//
        throw Mishap( "Invalid character" );
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
