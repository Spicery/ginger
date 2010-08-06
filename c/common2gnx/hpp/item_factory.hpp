#ifndef ITEM_FACTORY_HPP
#define ITEM_FACTORY_HPP

#include <string>

#include <stdio.h>

#include "item.hpp"

class ItemFactoryClass {

public:

	FILE				*file;
    bool            	peeked;
    std::string			text;
	Item				item;
	Item				spare;
	
private:
	int trychar( int ch );

public:
	Item read();
	//std::string readURL();
	void unread();
	void drop();
	Item peek();
	int peekchar();
	void reset();
	
public:

	ItemFactoryClass( FILE * f ) :
		file( f ),
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

//Item item_factory_read( ItemFactory tok );
//void item_factory_unread( ItemFactory tok );
//Item item_factory_peek( ItemFactory tok );
//void item_factory_drop( ItemFactory tok );
//void item_factory_reset( ItemFactory tok );

#endif
