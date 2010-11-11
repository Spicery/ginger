#include <string>
using namespace std;

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "item_factory.hpp"

#include "item_map.hpp"
#include "common.hpp"
#include "mishap.hpp"
#include "item.hpp"

//	Need IntToSmall
#include "key.hpp"


static int trychar( ItemFactory ifact, int ch ) {
    int c = getc( ifact->file );
    if ( c == ch ) {
        return 1;
    } else {
        ungetc( c, ifact->file );
        return 0;
    }
}

void item_factory_drop( ItemFactory ifact ) {
    ifact->peeked = false;
}

void item_factory_unread( ItemFactory ifact ) {
    ifact->peeked = true;
}

Item item_factory_read( ItemFactory ifact ) {
    int ch;
	Item it;

    if ( ifact->peeked ) {
        ifact->peeked = false;
        return ifact->item;
    }

    //  Get rid of white space and comments.
    for(;;) {
        ch = getc( ifact->file );
        if ( isspace( ch ) ) continue;
        if ( ch == '#' || ( ch == '/' && trychar( ifact, '/' ) ) ) {
            do
                ch = getc( ifact->file );
            while ( ch != '\n' && ch != '\r' );
            continue;
        }
        break;
    }

    ifact->text.clear();

    if ( ch == EOF ) {
		//
		//	End of file
		//
		it = ifact->item = ifact->spare;
		it->role = EofRole;
		it->functor = fnc_eof;
		it->nameString() = "<eof>";
    } else if ( isdigit( ch ) || ch == '-' ) {
		//
		//	Number
		//
        do {
            ifact->text.push_back( (char)ch );
            ch = getc( ifact->file );
        } while ( isdigit( ch ) );
        ungetc( ch, ifact->file );
		if ( ifact->text.size() == 1 && ifact->text[ 0 ] == '-' ) {
        	ifact->item = itemMap.lookup( ifact->text );
		} else {
			it = ifact->item = ifact->spare;
			it->functor = fnc_int;
            it->role = ImmediateRole;
			it->nameString() = ifact->text;
			it->extra = IntToSmall( atoi( it->nameString().c_str() ) );
		}
    } else if ( ch == '"' || ch == '\'' ) {
		//
		//	Strings and Character sequences
		//
		int open_quote = ch;
        while ( ch = getc( ifact->file ), ch != open_quote && ch != '\n' ) {
			if ( ch == '\\' ) {
				ch = getc( ifact->file );
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
					reset( "Invalid character after \\ in string (%d)", ch );
				}
			}
            ifact->text.push_back( ch );
        }
        if ( ch == '\n' ) {
            warning( "unterminated string (%s)", ifact->text.c_str() );
        }
		it = ifact->item = ifact->spare;
		it->functor = open_quote == '"' ? fnc_string : fnc_charseq;
		it->role = it->functor == fnc_string ? StringRole : CharSeqRole;
		it->nameString() = ifact->text;
		it->extra = ToRef( it->nameString().c_str() );
    } else if ( isalpha( ch ) || ch == '_' ) {
		//	
		//	Variables and keywords
		//	
        do {
            ifact->text.push_back( ch );
            ch = getc( ifact->file );
        } while ( isalnum( ch ) || ch == '_' );
        ungetc( ch, ifact->file );
        ifact->item = itemMap.lookup( ifact->text );
        if ( ifact->item == NULL ) {
		    it = ifact->item = ifact->spare;
		    it->functor = fnc_id;
		    it->role = PrefixRole;
			it->nameString() = ifact->text;
			it->extra = ToRef( it->nameString().c_str() );
        }
    } else if ( strchr( ";,()[]{}", ch ) ) {
		//
		//	Single character keywords
		//
        ifact->text.push_back( ch );
        it = ifact->item = itemMap.lookup( ifact->text );
        if ( ifact->item == NULL ) {
            mishap( "Invalid separator token '%s'", ifact->text.c_str() );
        }
    } else if ( ispunct( ch ) ) {
		//
		//	Probable errors treated as single character keywords.
		//
        //  ispunct()
        //      checks  for  any printable character which is not a
        //      space or an alphanumeric character.
        //
        do {
            ifact->text.push_back( ch );
            ch = getc( ifact->file );
        } while ( ispunct( ch ) );
        ungetc( ch, ifact->file );
        it = ifact->item = itemMap.lookup( ifact->text );
        if ( it == NULL ) {
            mishap( "Invalid punctuation token '%s'", ifact->text.c_str() );
        }
    } else {
		//
		//	Definite error
		//
        reset( "Invalid character (%c)", ch );
    }
    return ifact->item;
}

Item item_factory_peek( ItemFactory ifact ) {
    Item it = item_factory_read( ifact );
    ifact->peeked = true;
    return it;
}

/*ItemFactory item_factory_new( FILE *file ) {
    ItemFactory ifact = new ItemFactoryClass();
    
	ifact->file = file;
    ifact->text = "";
    ifact->peeked = false;
	ifact->item = NULL;
	ifact->spare = new ItemClass();
    return ifact;
}*/


void item_factory_reset( ItemFactory ifact ) {

	{
	    //	Need to zap the input buffer.  This is a bit laborious
	    //	but the best I could find in 10 minutes ...
	    FILE *f = ifact->file;
	    int fd = fileno( f );
	    int fd1 = dup( fd );
	    fclose( f );
	    ifact->file = fdopen( fd1, "r" );
	}

	ifact->peeked = false;
	ifact->text.clear();
}