#include <iostream>

#include <cstdlib>
#include "mishap.hpp"


#include "item_factory.hpp"
#include "gnx.hpp"
#include "toktype.hpp"

using namespace std;
using namespace Ginger;

/*
	E ::= ATOMIC | COMPOUND
	ATOMIC ::= NAME | NUMBER | STRING | ?NAME
	COMPOUND ::= MAP | LIST | ELEMENT
	ELEMENT ::= NAME ATTRS? LIST | NAME ATTRS
	ATTRS ::= < ( NAME = STRING | STRING : STRING ),... >
	MAP ::= { ( NAME = E | E : E ),... }
	LIST ::= [ E,... ]
*/

class GsonReader {
private: 
	GnxBuilder& builder;
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
		shared< Gnx > lhs = this->builder.build();
		
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
				this->builder.put( "type", "string" );
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
			shared< Gnx > func = builder.build();
			builder.start( "app" );
			builder.add( func );
			builder.start( "seq" );
			this->readExprList( ')' );
			builder.end();
			builder.end();
		}
	}
	
public:
	GsonReader( GnxBuilder & b, ItemFactory x ) : builder( b ), itemf( x ) {}
};

int main( int argc, char **argv, char **envp ) {
	try {
		FILE * in;
		if ( argc == 2 ) {
			in = fopen( argv[1], "r" ); 
			if ( in == NULL ) throw Mishap( "Cannot open file" ).culprit( "Filename", argv[1] );
		} else {
			in = stdin;
		}
	
		GnxBuilder builder;
		{
			ItemFactoryClass itemf( in );
			GsonReader reader( builder, &itemf );
			reader.readExpr();
		}
		
		{
			shared< Gnx > gnx( builder.build() );
			gnx->render();
		}
		cout << endl;	
		//cout << "OK" << endl;
	} catch ( Mishap m ) {
		m.report();
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
