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
#include <istream>
#include <stdexcept>
#include <sstream>
#include <typeinfo>
#include <deque>

#include <stddef.h>

#include "source.hpp"
#include "gson.hpp"
#include "mishap.hpp"


using namespace std;

namespace Ginger {

// - MapGSONData ---------------------------------------------------------------

GSON MapGSONData::index( GSON key ) const {
    MapGSONData::Map::const_iterator it = this->val.find( key );
    if ( it == this->val.end() ) {
        return GSON();
    } else {
        return it->second;
    }
}

// - Parser --------------------------------------------------------------------


class Tokeniser {
public:
    enum TOKEN_TYPE { 
        COMMA_TYPE,
        COLON_TYPE,
        EQUAL_TYPE,
        OPEN_BRACKET_TYPE, 
        CLOSE_BRACKET_TYPE, 
        OPEN_BRACE_TYPE, 
        CLOSE_BRACE_TYPE, 
        LONG_TYPE,
        STRING_TYPE, 
        CONSTANT_TYPE,
        END_OF_INPUT_TYPE 
    };
private:
    bool ready;
    Source & source;
    enum TOKEN_TYPE token_type;
    std::string token_text;
    std::string string_value;
public:
    Tokeniser( Source & src ) : ready( false ), source( src ) {}

private:
    static int toHex( const char ch ) {
        if ( '0' <= ch && ch <= '9' ) {
            return ch - '0';
        } else if ( 'A' <= ch && ch <= 'F' ) {
            return ch - 'A' + 10;
        } else if ( 'a' <= ch && ch <= 'f' ) {
            return ch - 'a' + 10;
        } else {
            throw Mishap( "Unexpected character, hex digit needed" ).culprit( "Character", string( "" ) + ch );
        }
    }

    long getHex() {
        long hex = 0;
        for ( int i = 0; i < 4; i++ ) {
            if ( not this->source.isValid() ) {
                throw Mishap( "Unexpected end of input in hexadecimal escape" ).culprit( "Token so far", this->token_text );
            }
            hex <<= 4;
            hex |= toHex( this->source.current() );
            this->token_text.push_back( this->source.current() );
            this->source.advance();
        }
        return hex;
    }

    enum TOKEN_TYPE readString() {
        //cerr << "readString" << endl;
        int n = 0;
        while ( source.isValid() && n < 2 ) {
            const char ch = source.next();
            this->token_text.push_back( ch );
            if ( ch == '\\' && source.isValid() ) {
                const char esc = source.next();
                this->token_text.push_back( esc );
                switch ( esc ) {
                    case '"':
                    case '\\':
                    case '/':
                        this->string_value.push_back( esc );
                        break;
                    case 'b':
                        this->string_value.push_back( '\b' );
                        break;
                    case 'f':
                        this->string_value.push_back( '\f' );
                        break;
                    case 'n':
                        this->string_value.push_back( '\n' );
                        break;
                    case 'r':
                        this->string_value.push_back( '\r' );
                        break;
                    case 't':
                        this->string_value.push_back( '\t' );
                        break;
                    case 'u': {
                        //  Read in the next 4 chars which must be hex digits.
                        this->string_value.push_back( static_cast< char >( this->getHex() ) );
                        break;
                    }
                    default:
                        throw Mishap( "Unexpected character after escape" ).culprit( "Character", string( "" ) + esc ).culprit( "Token so far", this->token_text );
                }
            } else if ( ch == '"' ) {
                n += 1;
            } else {
                this->string_value.push_back( ch );
            }
        }
        return STRING_TYPE;
    }

    enum TOKEN_TYPE readPunctuation() {
        const char ch = source.next();
        //cerr << "Punc: " << ch << endl;
        this->token_text.push_back( ch );
        return (
            ch == '[' ? OPEN_BRACKET_TYPE : 
            ch == ']' ? CLOSE_BRACKET_TYPE :
            ch == '{' ? OPEN_BRACE_TYPE : 
            ch == '}' ? CLOSE_BRACE_TYPE :
            ch == ':' ? COLON_TYPE :
            ch == '=' ? EQUAL_TYPE :
            COMMA_TYPE
        );
    }

    enum TOKEN_TYPE readLong( const char ch ) {
        this->token_text.push_back( ch );
        this->source.advance();
        while ( this->source.isValid() && isdigit( this->source.current() ) ) {
            this->token_text.push_back( ch );
            this->source.advance();
        }
        return LONG_TYPE;
    }

    /// Consumes any white space.
    bool eatWhiteSpace() {
        bool again = false;
        // Remove white spaces.
        while ( this->source.isValid() && isspace( this->source.current() ) ) {
            this->source.advance();
            again = true;
        }
        return again;
    }

    /// Consumes C-style & C++ style comments.
    bool eatCComments() {
        bool again = false;
        if ( this->source.isValid() && this->source.current() == '/' ) {
            this->source.advance();
            again = true;

            const char ch = this->source.next();
            if ( ch == '/' ) {
                //  C++ end of line comment. // .....
                while ( this->source.isValid() && this->source.next() != '\n' ) {                
                }
            } else if ( ch == '*' ) {
                //  C-style comment /* ... */
                bool closed = false;
                while ( this->source.isValid() ) {
                    if ( this->source.next() == '*' ) {
                        if ( this->source.isValid() && this->source.next() == '/' ) {
                            closed = true;
                            break;
                        }
                    } 
                }
                if ( not closed ) {
                    throw Mishap( "End of input encountered while inside /*.. */ comment" );
                }
            } else {
                throw Mishap( "Malformed comment" );
            }      
        }
        return again;
    }

    /// Consumes Ginger end of line comments.
    bool eatGingerComments() {
        bool again = false;
        if ( this->source.isValid() && this->source.current() == '#' ) {
            this->source.advance();
            again = true;
            while ( this->source.isValid() && this->source.next() != '\n' ) {                
            }
        }
        return again;
    }


    /// Removes white space and any comments. Three comment styles are
    /// supported, C-style /* ... */, C++ style end of line comments //, 
    /// and Ginger # comments.
    ///
    void skipWhiteSpace() {
        for (;;) {
            this->eatWhiteSpace();  //  Ignore return value.
            if ( eatCComments() ) continue;
            if ( eatGingerComments() ) continue;
            break;
        }
    }

    enum TOKEN_TYPE readConstant( const char ch ) {
        this->token_text.push_back( ch );
        this->source.advance();
        while ( this->source.isValid() && ( isalnum( this->source.current() ) || this->source.current() == '_' ) ) {
            this->token_text.push_back( this->source.next() );
        }
        return CONSTANT_TYPE;
    }

    enum TOKEN_TYPE readNextToken() {
        //cerr << "readNextToken" << endl;
        this->skipWhiteSpace();
        if ( this->source.isValid() ) {
            const char ch = source.current();
            if ( ch == '"' ) {
                return this->readString();
            } else if ( ch == '[' || ch == ']' || ch == ',' || ch == ';' || ch == '=' || ch == ':' || ch == '{' || ch == '}' ) {
                //  Note that ',' and ';' both map into a comma token.
                return this->readPunctuation();
            } else if ( isdigit( ch ) || ch == '-' ) {
                return this->readLong( ch );
            } else if ( isalpha( ch ) || ch == '_' ) {
                return this->readConstant( ch );
            } else {
                throw Mishap( "Unexpected character at start of token" ).culprit( "Character", string() + ch );
            }
        } else {
            //  Defensive, not intended to call current() if not hasNext().
            return END_OF_INPUT_TYPE;
        }
    }
    
public:
    void advance() {
        this->ready = false;
    }
    enum TOKEN_TYPE current() {
        if ( not this->ready ) {
            this->string_value.clear();  
            this->token_text.clear();  
            this->token_type = this->readNextToken();
            //cerr << "Token grabbed: '" << this->token_text << "'" << ", " << this->token_type << endl;
            this->ready = true;
        }
        return this->token_type;
    }
    enum TOKEN_TYPE next() {
        enum TOKEN_TYPE t = this->current();
        this->advance();
        return t;
    }
    bool hasNext() {
        if ( this->ready ) return true;
        this->skipWhiteSpace();
        return source.isValid();
    }
    bool hasntNext() {
        return not this->hasNext();
    }
    std::string getTokenText() {
        this->current();                //  Bring up to date.
        return this->token_text;
    }
    std::string getStringValue() {
        this->current();                //  Bring up to date.
        return this->string_value;
    }
    long getLong() {
        stringstream s( this->token_text );
        long lng;
        if ( not ( s >> lng ) ) {
            throw Mishap( "Problem converting to number" ).culprit( "Number?", this->token_text );
        }
        return lng;
    }
};



class DriveReadTerm {
private:
    GSONDriver & driver;
    Tokeniser & tokens;
    bool allow_terminating_comma;
public:
    DriveReadTerm( GSONDriver & w, Tokeniser & _tokens, bool allow = false ) : 
        driver( w ),
        tokens( _tokens ), 
        allow_terminating_comma( allow ) 
    {}

    //  Forward declaration.
    //GSON readTerm( Tokeniser & tokens );

    void readKeyValuePair() {
        this->driver.beginMaplet();
        
        this->driver.beginKeyMaplet();
        if ( tokens.hasNext() && tokens.current() == Tokeniser::CONSTANT_TYPE ) {
            std::string s( tokens.getTokenText() );
            tokens.advance();
            if ( tokens.hasNext() && tokens.current() == Tokeniser::EQUAL_TYPE ) {
                tokens.advance();
                this->driver.stringValue( s );
            } else {
                throw Mishap( "Expected '=' in map entry" ).culprit( "Token encountered", tokens.getTokenText() );
            }
        } else {
            this->readTerm();
            //  Now we require a ':' symbol.
            if ( tokens.hasntNext() || tokens.current() != Tokeniser::COLON_TYPE ) {
                throw Mishap( "Missing ':' after map key" ).culprit( "Token encountered", tokens.getTokenText() );
            } 
            tokens.advance();
        }
        this->driver.endKeyMaplet();
        
        this->driver.beginValueMaplet();
        this->readTerm();
        this->driver.endValueMaplet();

        this->driver.endMaplet();
    }


    void readMapEntries( bool require_comma = true ) {
        bool need_comma = false;
        while ( tokens.hasNext() && tokens.current() != Tokeniser::CLOSE_BRACE_TYPE ) {
            //cerr << "AT (pre-comma): '" << tokens.getString() << "'" << endl;
            if ( need_comma ) {
                 if ( tokens.current() == Tokeniser::COMMA_TYPE ) {
                    tokens.advance();
                 } else if ( require_comma ) {
                    throw Mishap( "Comma expected between map entries" ).culprit( "Token found", tokens.getTokenText() );
                }           
            } else {
                need_comma = true;
            }
            //cerr << "AT: '" << tokens.getString() << "'" << endl;
         
            if ( 
                this->allow_terminating_comma &&
                tokens.current() == Tokeniser::CLOSE_BRACE_TYPE
            ) break;


            this->readKeyValuePair();
        }
    }

    void readMap() {
        this->driver.beginMap();
        tokens.advance();   //  Skip over the '{'
        this->readMapEntries();
        if ( tokens.hasNext() ) {
            tokens.advance();    //  Skip over the '}'
        } else {
            throw Mishap( "Missing '}' at end of list, end of input encountered" );
        }
        this->driver.endMap();
    }

    void readList() {
        this->driver.beginList();
        tokens.advance();   //  Skip over the '['
        bool need_comma = false;
        while ( tokens.hasNext() && tokens.current() != Tokeniser::CLOSE_BRACKET_TYPE ) {

            if ( need_comma ) {
                if ( tokens.next() != Tokeniser::COMMA_TYPE ) {
                    throw Mishap( "Comma expected between list elements" ).culprit( "Token found", tokens.getTokenText() );
                }
            } else {
                need_comma = true;
            }

            if ( 
                this->allow_terminating_comma &&
                tokens.current() == Tokeniser::CLOSE_BRACKET_TYPE
            ) break;

            this->driver.beginMemberList();
            this->readTerm();
            this->driver.endMemberList();

        }
        if ( tokens.hasNext() ) {
            tokens.advance();    //  Skip over the ']'
        } else {
            throw Mishap( "Missing ']' at end of list, end of input encountered" );
        }
        this->driver.endList();
    }

    void readString() {
        this->driver.stringValue( tokens.getStringValue() );
        tokens.advance();
    }

    void readLong() {
        this->driver.longValue( tokens.getLong() );
        tokens.advance();
    }

    void readConstant() {
        std::string s( tokens.getTokenText() );
        tokens.advance();
        if ( s == "true" ) {
            this->driver.boolValue( true );
        } else if ( s == "false" ) {
            this->driver.boolValue( false );
        } else if ( s == "absent" ) {
            this->driver.absentValue();
        } else {
            //  Convert to bool etc ...
            throw Mishap( "Unknown constant" ).culprit( "Name", s );
        }
    }

    void readTerm() {
        if ( tokens.hasNext() ) {
            switch ( tokens.current() ) {
                case Tokeniser::STRING_TYPE: 
                    this->readString();
                    return;
                case Tokeniser::LONG_TYPE:
                    this->readLong();
                    return;
                case Tokeniser::OPEN_BRACKET_TYPE:
                    this->readList();
                    return;
                case Tokeniser::OPEN_BRACE_TYPE:
                    this->readMap();
                    return;
                case Tokeniser::CONSTANT_TYPE:
                    this->readConstant();
                    return;
                default:
                    throw Mishap( "Unexpected token at start of term" ).culprit( "Token found", tokens.getTokenText() );
            }
        }
    }

};



class ReadTerm {
private:
    Tokeniser & tokens;
    bool allow_terminating_comma;
public:
    ReadTerm( Tokeniser & _tokens, bool allow = false ) : 
        tokens( _tokens ), 
        allow_terminating_comma( allow ) 
    {}

    //  Forward declaration.
    //GSON readTerm( Tokeniser & tokens );

    std::pair< GSON, GSON > readKeyValuePair() {
        GSON p;
        if ( tokens.hasNext() && tokens.current() == Tokeniser::CONSTANT_TYPE ) {
            std::string s( tokens.getTokenText() );
            tokens.advance();
            if ( tokens.hasNext() && tokens.current() == Tokeniser::EQUAL_TYPE ) {
                tokens.advance();
                p = GSON( new StringGSONData( s ) );
            } else {
                throw Mishap( "Expected '=' in map entry" ).culprit( "Token encountered", tokens.getTokenText() );
            }
        } else {
            p = GSON( this->readTerm() );
            //  Now we require a ':' symbol.
            if ( tokens.hasntNext() || tokens.current() != Tokeniser::COLON_TYPE ) {
                throw Mishap( "Missing ':' after map key" ).culprit( "Token encountered", tokens.getTokenText() );
            } 
            tokens.advance();
        }
        if ( not p ) {
            throw Mishap( "Missing map entry" );
        }
        
        GSON q( this->readTerm() );
        if ( not q ) {
            throw Mishap( "Incomplete map entry" );
        }

        return(
            std::pair< GSON, GSON >(
                p,
                q
            )
        );
    }


    GSON readMapEntries( bool require_comma = true ) {
        MapGSONData * ptr = new MapGSONData();
        GSON map( ptr );
        bool need_comma = false;
        while ( tokens.hasNext() && tokens.current() != Tokeniser::CLOSE_BRACE_TYPE ) {
            //cerr << "AT (pre-comma): '" << tokens.getString() << "'" << endl;
            if ( need_comma ) {
                 if ( tokens.current() == Tokeniser::COMMA_TYPE ) {
                    tokens.advance();
                 } else if ( require_comma ) {
                    throw Mishap( "Comma expected between map entries" ).culprit( "Token found", tokens.getTokenText() );
                }           
            } else {
                need_comma = true;
            }
            //cerr << "AT: '" << tokens.getString() << "'" << endl;
         
            if ( 
                this->allow_terminating_comma &&
                tokens.current() == Tokeniser::CLOSE_BRACE_TYPE
            ) break;


            std::pair< GSON, GSON > p( this->readKeyValuePair() );
            ptr->put( p.first, p.second );
        }
        return map;    
    }

    GSON readMap() {
        tokens.advance();   //  Skip over the '{'
        GSON map( this->readMapEntries() );
        if ( tokens.hasNext() ) {
            tokens.advance();    //  Skip over the '}'
        } else {
            throw Mishap( "Missing '}' at end of list, end of input encountered" );
        }
        return map;
    }

    GSON readList() {
        ListGSONData * ptr = new ListGSONData();
        GSON list( ptr );
        tokens.advance();   //  Skip over the '['
        bool need_comma = false;
        while ( tokens.hasNext() && tokens.current() != Tokeniser::CLOSE_BRACKET_TYPE ) {

            if ( need_comma ) {
                if ( tokens.next() != Tokeniser::COMMA_TYPE ) {
                    throw Mishap( "Comma expected between list elements" ).culprit( "Token found", tokens.getTokenText() );
                }
            } else {
                need_comma = true;
            }

            if ( 
                this->allow_terminating_comma &&
                tokens.current() == Tokeniser::CLOSE_BRACKET_TYPE
            ) break;


            GSON p( this->readTerm() );
            if ( not p ) {
                throw Mishap( "Unexpected end of list" );
            }
            
            ptr->add( p );
        }
        if ( tokens.hasNext() ) {
            tokens.advance();    //  Skip over the ']'
        } else {
            throw Mishap( "Missing ']' at end of list, end of input encountered" );
        }
        return list;
    }

    GSON readString() {
        GSON s( new StringGSONData( tokens.getStringValue() ) );
        tokens.advance();
        return s;   
    }

    GSON readLong() {
        GSON lng( new LongGSONData( tokens.getLong() ) );
        tokens.advance();
        return lng;
    }

    GSON readConstant() {
        std::string s( tokens.getTokenText() );
        tokens.advance();
        if ( s == "true" ) {
            return GSON( new BoolGSONData( true ) );
        } else if ( s == "false" ) {
            return GSON( new BoolGSONData( false ) );
        } else if ( s == "absent" ) {
            return GSON( new AbsentGSONData() );
        } else {
            //  Convert to bool etc ...
            throw Mishap( "Unknown constant" ).culprit( "Name", s );
        }
    }

    GSON readTerm() {
        if ( tokens.hasNext() ) {
            switch ( tokens.current() ) {
                case Tokeniser::STRING_TYPE: 
                    return this->readString();
                case Tokeniser::LONG_TYPE:
                    return this->readLong();
                case Tokeniser::OPEN_BRACKET_TYPE:
                    return this->readList();
                case Tokeniser::OPEN_BRACE_TYPE:
                    return this->readMap();
                case Tokeniser::CONSTANT_TYPE:
                    return this->readConstant();
                default:
                    throw Mishap( "Unexpected token at start of term" ).culprit( "Token found", tokens.getTokenText() );
            }
        } else {
            return GSON();
        }
    }


};


GSON GSON::read( const string fname ) {
    FileSource f( fname );
    Tokeniser t( f );
    ReadTerm r( t );
    return r.readTerm();
}

GSON GSON::read( std::istream & input ) {
    IStreamSource f( input );
    Tokeniser t( f );
    ReadTerm r( t );
    return r.readTerm();
}

GSON GSON::read( FILE * input, bool close_on_exit ) {
    FilePtrSource f( input, close_on_exit );
    Tokeniser t( f );
    ReadTerm r( t );
    return r.readTerm();
}

GSON GSON::readSettingsFile( FILE * input, bool close_on_exit ) {
    FilePtrSource f( input, close_on_exit );
    Tokeniser t( f );
    GSONBuilder b;
    DriveReadTerm r( b, t, true );
    b.beginMap();
    r.readMapEntries( false );
    b.endMap();
    if ( t.hasNext() ) {
        throw Mishap( "End of settings found before the end of input" );
    }
    return b.newGSON();   
}

GSON GSON::readSettingsFile( std::istream & input ) {
    IStreamSource f( input );
    Tokeniser t( f );
    GSONBuilder b;
    DriveReadTerm r( b, t, true );
    b.beginMap();
    r.readMapEntries( false );
    b.endMap();
    if ( t.hasNext() ) {
        throw Mishap( "End of settings found before the end of input" );
    }
    return b.newGSON();   
}

GSON GSON::readSettingsFile( const string fname ) {
    FileSource f( fname );
    Tokeniser t( f );
    GSONBuilder b;
    DriveReadTerm r( b, t, true );
    b.beginMap();
    r.readMapEntries( false );
    b.endMap();
    if ( t.hasNext() ) {
        throw Mishap( "End of settings found before the end of input" );
    }
    return b.newGSON();   
}



//  - lessThan -----------------------------------------------------------------

bool AbsentGSONData::lessThan( const GSONData & other ) const {
    const type_info & tc( typeid( *this ) );
    const type_info & otc( typeid( other ) );
    if ( tc != otc ) return tc.before( otc );
    return false;
}

bool BoolGSONData::lessThan( const GSONData & other ) const {
    const type_info & tc( typeid( *this ) );
    const type_info & otc( typeid( other ) );
    if ( tc != otc ) return tc.before( otc );
    return this->val < static_cast< const BoolGSONData & >( other ).val;
}

bool LongGSONData::lessThan( const GSONData & other ) const {
    const type_info & tc( typeid( *this ) );
    const type_info & otc( typeid( other ) );
    if ( tc != otc ) return tc.before( otc );
    return this->val < static_cast< const LongGSONData & >( other ).val;
}

bool StringGSONData::lessThan( const GSONData & other ) const {
    const type_info & tc = typeid( *this );
    const type_info & otc = typeid( other );
    if ( tc != otc ) return tc.before( otc );
    return this->val < static_cast< const StringGSONData & >( other ).val;
}

bool ListGSONData::lessThan( const GSONData & other ) const {
    const type_info & tc = typeid( *this );
    const type_info & otc = typeid( other );
    if ( tc != otc ) return tc.before( otc );
    return this->val < static_cast< const ListGSONData & >( other ).val;
}

bool MapGSONData::lessThan( const GSONData & other ) const {
    const type_info & tc = typeid( *this );
    const type_info & otc = typeid( other );
    if ( tc != otc ) return tc.before( otc );
    return this->val < static_cast< const MapGSONData & >( other ).val;
}

//  - Visiting -----------------------------------------------------------------


void AbsentGSONData::visit( GSONDataVisitor & visitor ) {
    visitor.visitAbsent( *this );
}

void BoolGSONData::visit( GSONDataVisitor & visitor ) {
    visitor.visitBool( *this );
}

void LongGSONData::visit( GSONDataVisitor & visitor ) {
    visitor.visitLong( *this );
}

void StringGSONData::visit( GSONDataVisitor & visitor ) {
    visitor.visitString( *this );
}

void ListGSONData::visit( GSONDataVisitor & visitor ) {
    visitor.visitList( *this );
}

void MapGSONData::visit( GSONDataVisitor & visitor ) {
    visitor.visitMap( *this );
}

//  - Walking - ----------------------------------------------------------------

void StringGSONData::walk( GSONDataWalker & walker ) {
    walker.stringValue( this->val );
}

void AbsentGSONData::walk( GSONDataWalker & walker ) {
    walker.absentValue();
}

void BoolGSONData::walk( GSONDataWalker & walker ) {
    walker.boolValue( this->val );
}

void LongGSONData::walk( GSONDataWalker & walker ) {
    walker.longValue( this->val );
}

void ListGSONData::walk( GSONDataWalker & walker ) {
    walker.beginList( *this );
    int index = 0;
    for ( BabyList::iterator it = this->val.begin(); it != val.end(); ++it, index += 1 ) {
        GSON & p = *it;
        walker.beforeMemberList( *this, index );
        p->walk( walker );
        walker.afterMemberList( *this, index );
    }
    walker.endList( *this );
}

void MapGSONData::walk( GSONDataWalker & walker ) {
    walker.beginMap( *this );
    int index = 0;
    for ( 
        std::map< GSON, GSON >::iterator it = this->val.begin(); 
        it != val.end(); 
        ++it, index += 1 
    ) {
        walker.beginKeyMap( *this, index );
        it->first->walk( walker );
        walker.endKeyMap( *this, index );
        walker.beginValueMap( *this, index );
        it->second->walk( walker );
        walker.endValueMap( *this, index );
    }
    walker.endMap( *this );
}

//  - Rendering ----------------------------------------------------------------

class Renderer : public GSONDataWalker {
public:
    enum MODE {
        SHOW_MODE,
        PRINT_MODE
    };
private:
    std::ostream & out;
    enum MODE mode;
public:
    Renderer( std::ostream & _out = std::cout, const enum MODE _mode = SHOW_MODE ) : out( _out ), indent( 0 ), mode( _mode ) {}
public:
    void boolValue( const bool b ) {
        this->out << ( b ? "true" : "false" );
    }

    void absentValue() {
        this->out << "absent";
    }

    void longValue( const long n ) {
        this->out << n;
    }

    void stringValue( const std::string & s ) {
        if ( this->mode == SHOW_MODE ) {
            this->out << '"';
        }
        for ( string::const_iterator it = s.begin(); it != s.end(); ++it ) {
            const char ch = *it;
            switch ( ch ) {
                case '\\':
                case '"':
                    this->out << '\\' << ch;
                    break;
                case '\b':
                    this->out << "\\b";
                    break;
                case '\f':
                    this->out << "\\f";
                    break;
                case '\t':
                    this->out << "\\t";
                    break;
                case '\n':
                    this->out << "\\n";
                    break;
                case '\r':
                    this->out << "\\r";
                    break;
                default:
                    if ( isprint( ch ) ) {
                        this->out << ch;
                    } else {
                        this->out << "\\u00" << hex << static_cast< int >( ch ) << dec;
                    }
            }
        }
        if ( this->mode == SHOW_MODE ) {
            this->out << '"';
        }
    }

    void beginList( ListGSONData & b ) {
        this->out << "[";
    }

    void beforeMemberList( ListGSONData & b, const int index ) {
        if ( index != 0 ) {
            this->out << ",";
        }
    }

    void afterMemberList( ListGSONData & b, const int index ) {
    }

    void endList( ListGSONData & b ) {
        this->out << "]";
    }

    void beginMap( MapGSONData & b ) {
        this->out << "{";
    }

    void beginKeyMap( MapGSONData & b, const int index ) {
        if ( index != 0 ) {
            this->out << ",";
        }
    }

    void endKeyMap( MapGSONData & b, const int index ) {
         this->out << ":";
    }

    void beginValueMap( MapGSONData & b, const int index ) {
    }

    void endValueMap( MapGSONData & b, const int index ) {
    }

    void endMap( MapGSONData & b ) {
        this->out << "}";
    }

};

void GSONData::render() {
    Renderer r;
    this->walk( r );
}

//------------------------------------------------------------------------------
//  GSON
//------------------------------------------------------------------------------

//  ~ GSON delegated operations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void GSON::render() { 
    this->self()->render(); 
}

void GSON::visit( GSONDataVisitor & visitor ) { 
    this->self()->visit( visitor ); 
}

void GSON::walk( GSONDataWalker & walker ) { 
    this->self()->walk( walker ); 
}

const std::string GSON::typeOf() const { 
    return typeid( this->self() ).name(); 
}

bool GSON::lessThan( const GSON other ) const { 
    return this->self()->lessThan( *other ); 
}

//  ~ GSON Absent operations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool GSON::isAbsent() const { 
    return dynamic_cast< const AbsentGSONData * >( this->self() ) != NULL; 
}


//  ~ GSON Bool operations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool GSON::isBool() const { 
    return dynamic_cast< const BoolGSONData * >( this->self() ) != NULL; 
}

bool GSON::getBool() const {
    const BoolGSONData * str = dynamic_cast< const BoolGSONData * >( this->self() );
    if ( str != NULL ) {
        return str->getBool();
    } else {
        throw Mishap( "Bool needed" ).culprit( "Type of GSON", this->typeOf() );
    }
}

//  ~ GSON Long operations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool GSON::isLong() const { 
    return dynamic_cast< const LongGSONData * >( this->self() ) != NULL; 
}

long GSON::getLong() const {
    const LongGSONData * str = dynamic_cast< const LongGSONData * >( this->self() );
    if ( str != NULL ) {
        return str->getLong();
    } else {
        throw Mishap( "Long needed" ).culprit( "Type of GSON", this->typeOf() );
    }
}

//  ~ GSON String operations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool GSON::isString() const { 
    return dynamic_cast< const StringGSONData * >( this->self() ) != NULL; 
}

std::string GSON::getString() const {
    const StringGSONData * str = dynamic_cast< const StringGSONData * >( this->self() );
    if ( str != NULL ) {
        return str->getString();
    } else {
        throw Mishap( "String needed" ).culprit( "Type of GSON", this->typeOf() );
    }
}

//  ~ GSON List operations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

GSON GSON::at( const int n ) const {
    const ListGSONData * list = dynamic_cast< const ListGSONData * >( this->self() );
    if ( list != NULL ) {
        return list->at( n );
    } else {
        throw Mishap( "List needed" ).culprit( "Type of GSON", this->typeOf() );
    }
}

GSON GSON::first() const {
    const ListGSONData * list = dynamic_cast< const ListGSONData * >( this->self() );
    if ( list != NULL ) {
        return list->first();
    } else {
        throw Mishap( "List needed" ).culprit( "Type of GSON", this->typeOf() );
    }
}

GSON GSON::last() const {
    const ListGSONData * list = dynamic_cast< const ListGSONData * >( this->self() );
    if ( list != NULL ) {
        return list->last();
    } else {
        throw Mishap( "List needed" ).culprit( "Type of GSON", this->typeOf() );
    }
}

size_t GSON::size() const {
    const ListGSONData * list = dynamic_cast< const ListGSONData * >( this->self() );
    if ( list != NULL ) {
        return list->size();
    } else {
        throw Mishap( "Trying to take size of a non-list" ).culprit( "Type of GSON", this->typeOf() );;
    }
}

bool GSON::isList() const { 
    return dynamic_cast< const ListGSONData * >( this->self() ) != NULL; 
}

bool GSON::isEmpty() const {
    const ListGSONData * list = dynamic_cast< const ListGSONData * >( this->self() );
    if ( list != NULL ) {
        return list->isEmpty();
    } else {
        throw Mishap( "List needed" ).culprit( "Type of GSON", this->typeOf() );;
    }
}

bool GSON::isntEmpty() const {
    return not this->isEmpty();
}

//  ~ GSON Map operations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool GSON::isMap() const { 
    return dynamic_cast< const MapGSONData * >( this->self() ) != NULL; 
}

GSON GSON::index( const std::string & s ) const {
    const MapGSONData * map = dynamic_cast< const MapGSONData * >( this->self() );
    if ( map != NULL ) {
        GSON k( new StringGSONData( s ) );
        return map->index( k );
    } else {
        throw Mishap( "Map needed" ).culprit( "Type of GSON", this->typeOf() );
    }
}

//  -- FormatUsing -------------------------------------------------------------


void GSON::print( std::ostream & out ) {
    Renderer r( out, Renderer::PRINT_MODE );
    this->walk( r );
}

void GSON::formatUsing( std::ostream & out, const std::string & control_string ) {
    //  Iterate over the control string character by character but with the
    //  ability to skip ahead.
    int index = 0;
    for ( int i = 0; i < control_string.size(); i++ ) {
        const char ch = control_string[ i ];
        if ( ch != '%' ) {
            out << ch;
        } else if ( i + 1 < control_string.size() ) {
            const char next = control_string[ ++i ];
            switch ( next ) {
                case 's': 
                case 'p': {
                    if ( index < this->size() ) {
                        GSON at = this->at( index++ );
                        at.print( out );
                    } else {
                        throw Ginger::Mishap( "Not enough arguments for format string" ).culprit( "Format string", control_string );
                    }
                    break;
                }
                case '%' : {
                    out << ch;
                    break;
                }
                default: {
                    throw Ginger::Mishap( "Unexpected character after % in format string" ).culprit( "Format string", control_string );
                    break;
                }
            }
        } else {
            out << ch;
        }
    }    
}


} // namespace Ginger
