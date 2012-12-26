#ifndef ITEM_FACTORY_HPP
#define ITEM_FACTORY_HPP

#include <string>
#include <list>

#include <stdio.h>

#include "item.hpp"
#include "item_map.hpp"

class Source {
private:
	std::list< int >	buffer;
	FILE *				file;
	int					lineno;
	
public:
	int peekchar();
	int nextchar();
	void pushchar( const int ch );
	void resetSource();
	int lineNumber() { return this->lineno; }
	
public:
	Source( FILE * file ) : file( file ), lineno( 1 ) {}
};



class ItemFactoryClass : public Source {

public:
	bool				cstyle_mode;
	ItemMap				itemMap;
    bool            	peeked;
    std::string			text;
	Item				item;
	Item				spare;
	
private:
	int trychar( int ch );
	int eatsWhiteSpaceAndComments();
	void readAtEndOfFile();
	void readAtDigitOrMinus( int ch );
	void readAtAlphaOrUnderbar( int ch );
	void readAtQuoteCharType( int ch );
	void readAtSeparatorCharType( int ch );
	void readAtBracketCharType( int ch );
	void readAtBracketDecorationCharType( int ch );
	void readAtSelfGlueCharType( int ch );
	void readAtSignCharType( int ch );


public:
	Item read();
	void unread();
	void drop();
	Item peek();
	void reset();
	
public:

	ItemFactoryClass( FILE * f, const bool cstyle ) :
		Source( f ),
		cstyle_mode( cstyle ),
		itemMap( cstyle ),
		peeked( false ),
		item( NULL ),
		spare( new ItemClass() )
	{
	}
	
	~ItemFactoryClass() {
		delete this->spare;
	}
	
};

typedef class ItemFactoryClass *ItemFactory;

#endif

