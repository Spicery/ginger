#ifndef ITEM_FACTORY_HPP
#define ITEM_FACTORY_HPP

//	C++ STL
#include <iostream>
#include <string>

//	C std lib
#include <stdio.h>

//	AppGinger lib
#include "shared.hpp"

//	Local lib
#include "item.hpp"


namespace LNX2MNX_NS {

class ItemFactoryClass {
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
		
};

typedef shared< ItemFactoryClass > ItemFactory;

} // namespace

#endif

