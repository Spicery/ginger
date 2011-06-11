#ifndef ITEM_HPP
#define ITEM_HPP

#include <string>

#include "toktype.hpp"

class ItemClass {
friend class ItemFactoryClass;
private:
    std::string 			name;
    TokType					tok_type;

public:
	ItemClass( const char *n, TokType f ) :
		name( n ),
		tok_type( f )
	{
	}
	
	ItemClass() :
		name( "" ),
		tok_type( tokty_eof )
	{
	}

public:
	//bool item_is_neg_num();
	//int item_int();
	bool isLiteralConstant();
	bool isName();
	bool isStringValue();
	bool isIntegerValue();
	bool isOpenList();
	bool isCloseList();
	bool isComma();
	bool isSign( char ch );
	char signChar();
	const std::string asValue();
	const std::string asType();
	
};

typedef ItemClass * Item;




#endif

