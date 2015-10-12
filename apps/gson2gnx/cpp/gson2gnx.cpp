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

#include <cstdlib>

#include <fstream>

#include <getopt.h>
#include <stddef.h>

#include "printGPL.hpp"
#include "mishap.hpp"
#include "gngversion.hpp"

#include "item_factory.hpp"
#include "mnx.hpp"
#include "toktype.hpp"

using namespace std;
using namespace Ginger;

/*
	EXPR ::= PRIMITIVE | COMPOUND
	PRIMITIVE ::= STRING | NUMBER | VARIABLE
	COMPOUND ::= APPLY | SEQ | LAMBDA | ARRAY | MAP  | ELEMENT
	APPLY ::= EXPR SEQ
	SEQ ::= ( EXPR, ... )
	ARRAY ::= [ EXPR, ... ]
	LAMBDA ::= NAME ATTRS SEQ? { EXPR, ... }
	MAP ::= { ((EXPR : EXPR)|(NAME=EXPR)), ... }
	ELEMENT ::= NAME ATTRS? ARRAY | NAME ATTRS
	ATTRS ::= < ( NAME = STRING ), ... >
*/

class GsonReader {
private: 
	MnxBuilder& builder;
	ItemFactory itemf;
	
public:

	void mustReadSign( char ch ) {
		Item it = itemf->read();
		if ( !it->isSign( ch ) ) {
			throw Mishap( "Unrecognised token" ).culprit( "Token", it->asValue() );
		}
	}
	
	bool tryReadSign( char ch ) {
		Item it = itemf->peek();
		bool answer = it->isSign( ch );
		if ( answer ) {
			this->itemf->drop();
		}
		return answer;
	}
	
	void readExprList() {
		do {
			this->readExpr();
		} while ( this->tryReadSign( ',' ) );
	}

	void readExprList( char ch ) {
		if ( !this->tryReadSign( ch ) ) {
			this->readExprList();
			this->mustReadSign( ch );
		}
	}
	
	void readMapletList( char signch ) {
		if ( !this->tryReadSign( signch ) ) {
			this->readMapletList();
			this->mustReadSign( signch );
		}		
	}
	
	void readMapletList() {
		do {
			this->readMaplet();
		} while ( this->tryReadSign( ',' ) );
	}
	
	void readMaplet() {
		//	This is a teensy bit tricksy as we have not got proper read-ahead.
		
		//	First we grab the next expression.
		this->readExpr();
		shared< Mnx > lhs = this->builder.build();
		
		//	Now we check out the operator.
		Item op = this->itemf->read();		
		if ( op->isSign( ':' ) ) {
			//	This is a general binary maplet.
			this->builder.start( "maplet" );
			this->builder.add( lhs );
			this->readExpr();
			this->builder.end();
		} else if ( op->isSign( '=' ) ) {
			//	This is the special case. We have to re-interpret the LHS as a name.
			//	We should have built <id name=THE_BIT_WE_WANT/>
			if ( lhs->name() == "id" && lhs->hasAttribute( "name" ) ) {
				string v( lhs->attribute( "name" ) );
				this->builder.start( "maplet" );
				this->builder.start( "constant" );
				this->builder.put( "type", "symbol" );
				this->builder.put( "value", v );
				this->builder.end();
				this->readExpr();
				this->builder.end();				
			} else {
				throw Mishap( "Invalid expression on the left of an '='" );
			}
		} else {
			throw Mishap( "Unexpected token" ).culprit( "token", op->asValue() );
		}
	}
	
	void readAttrList( char ch ) {
		if ( !this->tryReadSign( ch ) ) {
			this->readAttrList();
			this->mustReadSign( ch );
		}
	}
	
	void readAttrList() {
		do {
			this->readAttr();
		} while ( this->tryReadSign( ',' ) );
	}
	
	void readAttr() {
		Item item = this->itemf->read();
		string key = item->asValue();
		if ( item->isName() ) {
			this->mustReadSign( '=' );
			Item v = this->itemf->read();
			if ( v->isStringValue() ) {
				this->builder.put( key, v->asValue() );
			} else {
				throw Mishap( "Bad input" );
			}
		} else if ( item->isStringValue() ) {
			this->mustReadSign( ':' );
			Item v = this->itemf->read();
			if ( v->isStringValue() ) {
				this->builder.put( key, v->asValue() );
			} else {
				throw Mishap( "Bad input" );
			}
		} else {
			throw Mishap( "Invalid attribute" ).culprit( "key", item->asValue() );
		}
	}
	
	void readLambdaArgsAndBody() {
		if ( this->tryReadSign( '(' ) ) {
			builder.start( "seq" );
			this->readExprList( ')' );
			builder.end();

			int curry_count = 0;				
			while ( this->tryReadSign( '(' ) ) {
				curry_count += 1;
				builder.start( "fn" );
				builder.start( "seq" );
				this->readExprList( ')' );
				builder.end();
			}
			
			builder.start( "seq" );
			this->mustReadSign( '{' );
			this->readExprList( '}' );
			builder.end();
			
			for ( int i = 0; i < curry_count; i++ ) {
				builder.end();
			}
		} else if ( this->tryReadSign( '{' ) ) {
			builder.start( "seq" );
			builder.end();
			builder.start( "seq" );
			this->readExprList( '}' );
			builder.end();
		} else {
			throw Mishap( "Expecting lambda form starting ( or {" );
		}
	}

	void readElementOrName() {
		Item item = this->itemf->read();
		string name = item->asValue();
		if ( this->tryReadSign( '<' ) ) {
			builder.start( name );
			this->readAttrList( '>' );
			if ( this->tryReadSign( '[' ) ) {
				this->readExprList( ']' );
			}
			builder.end();
		} else if ( this->tryReadSign( '[' ) ) {
			builder.start( name );
			this->readExprList( ']' );
			builder.end();
		} else {
			builder.start( "id" );
			builder.put( "name", name );
			builder.end();
		}
	}
	
	void readPrimaryExpr() {
		Item item = itemf->peek();
		//cout << "Token type " << tok_type_name( item->tok_type ) << endl;
		if ( item->isLiteralConstant() ) {
			this->itemf->drop();
			if ( item->isCharSeqValue() ) {
				builder.start( "seq" );
				const string v( item->asValue() );
				for ( string::const_iterator it = v.begin(); it != v.end(); ++it ) {
					builder.start( "constant" );
					builder.put( "type", "char" );
					string s;
					const char ch = *it;
					s.push_back( ch );
					builder.put( "value", s );
					builder.end();
				}
				builder.end();
			} else {
				builder.start( "constant" );
				builder.put( "type", item->asType() );
				builder.put( "value", item->asValue() );
				builder.end();
			}
		} else if ( item->isName() ) {
			this->readElementOrName();
		} else if ( this->tryReadSign( '[' ) ) {
			builder.start( "list" );
			this->readExprList( ']' );
			builder.end();
		} else if ( this->tryReadSign( '{' ) ) {
			builder.start( "map" );
			this->readMapletList( '}' );
			builder.end();
		} else if ( this->tryReadSign( '(' ) ) {
			builder.start( "seq" );
			this->readExprList( ')' );	
			builder.end();
		} else if ( this->tryReadSign( '<' ) ) {
			builder.start( "fn" );
			this->readAttrList( '>' );
			this->readLambdaArgsAndBody();
			builder.end();			
		} else if ( this->tryReadSign( '?' ) ) {
			builder.start( "var" );
			builder.put( "name", this->itemf->read()->asValue() );
			builder.end();
		} else {
			throw Mishap( "Unimplemented" );
		}
	}

	void readExpr() {
		this->readPrimaryExpr();
		while ( this->tryReadSign( '(' ) ) {
			shared< Mnx > func = builder.build();
			builder.start( "app" );
			builder.add( func );
			builder.start( "seq" );
			this->readExprList( ')' );
			builder.end();
			builder.end();
		}
	}
	
public:
	GsonReader( MnxBuilder & b, ItemFactory x ) : builder( b ), itemf( x ) {}
};


#define GSON2GNX "gson2gnx"

extern char * optarg;
static struct option long_options[] =
    {
        { "help",			optional_argument,		0, 'H' },
        { "license",        optional_argument,      0, 'L' },
        { "version",        no_argument,            0, 'V' },
        { 0, 0, 0, 0 }
    };

class Main {
private:
	bool use_stdin;
	string input_file_name;

public:
	string version() {
		return "0.1";
	}

	void parseArgs( int argc, char **argv, char **envp ) {
		this->use_stdin = true;
		
		for(;;) {
			int option_index = 0;
			int c = getopt_long( argc, argv, "H::L::V", long_options, &option_index );
			if ( c == -1 ) break;
			switch ( c ) {
				case 'H': {
					//  Eventually we will have a "home" for our auxillary
					//  files and this will simply go there. Or run a web
					//  browser pointed there.
					if ( optarg == NULL ) {
						cout << "Usage:  " << GSON2GNX << " OPTIONS < GSON_IN > GNX_OUT\n"  << endl;
						cout << "OPTIONS" << endl;
						cout << "-H, --help[=TOPIC]    help info on optional topic (see --help=help)" << endl;
						cout << "-V, --version         print out version information and exit" << endl;
						cout << "-L, --license[=PART]  print out license information and exit (see --help=license)" << endl;
						cout << endl;
					} else if ( std::string( optarg ) == "help" ) {
						cout << "--help=help           this short help" << endl;
						cout << "--help=licence        help on displaying license information" << endl;
					} else if ( std::string( optarg ) == "license" ) {
						cout << "Displays key sections of the GNU Public License." << endl;
						cout << "--license=warranty    Shows warranty." << endl;
						cout << "--license=conditions  Shows terms and conditions." << endl;
					} else {
						cout << "Unknown help topic" << optarg << endl;
					}
					exit( EXIT_SUCCESS );   //  Is that right?
				}
				case 'L': {
					exit( Ginger::optionPrintGPL( optarg ) );              
				}
				case 'V': {
					cout << GSON2GNX << ": version " << this->version() << " (" << __DATE__ << " " << __TIME__ << ") part of " << PACKAGE_NAME << " version " << PACKAGE_VERSION << endl;
					exit( EXIT_SUCCESS );   //  Is that right?
				}
				case '?': {
					break;
				}
				default: {
					cout << "?? getopt returned character code 0x" << hex << static_cast< int >( c ) << dec << endl;
				}
			}
        }
        
		argc -= optind;
		argv += optind;
		
		if ( argc > 0 ) {
			use_stdin = false;
			input_file_name = argv[ argc - 1 ];
			argc -= 1;
			argv += 1;
		}
		
		if ( argc > 0 ) {
			throw Mishap( "Unused trailing argument" ).culprit( "Unused", argv[ argc - 1 ] );
		}

	}
	
public:
	void run() {
		FILE * in;
		if ( this->use_stdin ) {
			in = stdin;
		} else {
			in = fopen( input_file_name.c_str(), "r" ); 
			if ( in == NULL ) throw Mishap( "Cannot open file" ).culprit( "Filename", input_file_name );
		}
	
		ItemFactoryClass itemf( in );
		for (;;) {
			MnxBuilder builder;
			
			if ( itemf.peek()->isAtEnd() ) break;
			
			GsonReader reader( builder, &itemf );
			reader.readExpr();
	
			shared< Mnx > gnx( builder.build() );
			gnx->render();
	
			cout << endl;	
		}
	}
};

int main( int argc, char **argv, char **envp ) {
	try {
		Main main;
		main.parseArgs( argc, argv, envp );
		main.run();
	    return EXIT_SUCCESS;
	} catch ( Ginger::Mishap & p ) {
		p.report();
		return EXIT_FAILURE;
	}
}
