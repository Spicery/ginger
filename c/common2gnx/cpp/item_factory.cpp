#include <string>
using namespace std;

#include <stdio.h>
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
        throw Mishap( "Trying to read URL after pushback" );
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

Item ItemFactoryClass::read() {
    int ch;
	Item it;

    if ( this->peeked ) {
        this->peeked = false;
        return this->item;
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

    if ( ch == EOF ) {
		//
		//	End of file
		//
		it = this->item = this->spare;
		it->role = EofRole;
		it->tok_type = tokty_eof;
		it->nameString() = "<eof>";
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
        	this->item = itemMap.lookup( this->text );
		} else {
			it = this->item = this->spare;
			it->tok_type = tokty_int;
            it->role = LiteralRole;
			it->nameString() = this->text;
		}
    } else if ( ch == '"' || ch == '\'' ) {
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
				} else {
					throw "Invalid character after \\ in string";	// ch
				}
			}
            this->text.push_back( ch );
        }
        if ( ch == '\n' ) {
            throw "unterminated string (%s)";
        }
		it = this->item = this->spare;
		it->tok_type = open_quote == '"' ? tokty_string : tokty_charseq;
		it->role = it->tok_type == tokty_string ? StringRole : CharSeqRole;
		it->nameString() = this->text;
		//it->extra = ToRef( it->nameString().c_str() );
    } else if ( isalpha( ch ) || ch == '_' ) {
		//	
		//	Variables and keywords
		//	
        do {
            this->text.push_back( ch );
            ch = getc( this->file );
        } while ( isalnum( ch ) || ch == '_' );
        ungetc( ch, this->file );
        this->item = itemMap.lookup( this->text );
        if ( this->item == NULL ) {
		    it = this->item = this->spare;
		    it->tok_type = tokty_id;
		    it->role = PrefixRole;
			it->nameString() = this->text;
		}
    } else if ( strchr( ";,", ch ) ) {
		//
		//	Single character keywords
		//
        this->text.push_back( ch );
        it = this->item = itemMap.lookup( this->text );
        if ( this->item == NULL ) {
        	//	Never happens.
            throw Mishap( "Invalid punctuation token" ); 
        }
    } else if ( strchr( "()[]{}", ch ) ) {
	    this->text.push_back( ch );
    	if ( ( ch = getc( this->file ) ) == '%' ) {
    		this->text.push_back( ch );
    	} else {
    		ungetc( ch, this->file );
    	}
		it = this->item = itemMap.lookup( this->text );
		if ( this->item == NULL ) {
            throw Mishap( "Invalid punctuation token" ); 
		}
    } else if ( ch == '%' ) {
    	this->text.push_back( ch );
   		ch = getc( this->file );
    	if ( strchr( "()[]{}", ch ) ) {    	
			this->text.push_back( ch );
		} else {
			ungetc( ch, this->file );
		}
		it = this->item = itemMap.lookup( this->text );
		if ( this->item == NULL ) {
            throw Mishap( "Invalid punctuation token" ); 
		}
    } else if ( strchr( ".@", ch ) ) {
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
    	it = this->item = itemMap.lookup( this->text );
    	if ( it == NULL ) {
            throw Mishap( "Invalid repeated-character sign" ).culprit( "Token", this->text );
        }
    } else if ( ispunct( ch ) ) {
		//
		//	Other punctuation treated as multi-character keywords.
		//
        //  ispunct()
        //      checks  for  any printable character which is not a
        //      space or an alphanumeric character.
        //
        do {
            this->text.push_back( ch );
            ch = getc( this->file );
        } while ( ispunct( ch ) );
        ungetc( ch, this->file );
        it = this->item = itemMap.lookup( this->text );
        if ( it == NULL ) {
            throw Mishap( "Invalid sign (combination of special characters)" ).culprit( "Sign", this->text );
        }
    } else {
		//
		//	Definite error
		//
        throw "Invalid character (%c)"; //  ch );
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
