#ifndef ITEM_MAP_H
#define ITEM_MAP_H

#include <string>
#include <map>

#include "item.hpp"

class ItemMap {
private:
	std::map< std::string, ItemClass * > table;
	
private:
	void add( const char *n, TokType f, Role r, int p );
	
public:
	ItemMap();
	
public:
	ItemClass * lookup( const char * name );
	ItemClass * lookup( const std::string & name );
};

extern ItemMap itemMap;

#endif
