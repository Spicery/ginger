#ifndef ITEM_FACTORY_HPP
#define ITEM_FACTORY_HPP

#include <string>

#include <stdio.h>

#include "item.hpp"

class ItemFactoryClass {
public:
	Item				spare;

public:
	FILE				*file;
    bool            	peeked;
    std::string			text;
	Item				item;
	
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
		item( new ItemClass() )
	{
	}
	
	~ItemFactoryClass() {
		delete this->spare;
	}
	
};

typedef class ItemFactoryClass *ItemFactory;


#endif

