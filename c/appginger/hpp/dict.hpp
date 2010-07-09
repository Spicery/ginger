#ifndef DICT_HPP
#define DICT_HPP

#include <vector>
#include <map>

#include "ident.hpp"


class DictClass {
friend class ScanDict;
private:
	std::map< std::string, Ident > table;
    
public:	
	Ident lookup( const std::string & c );
	Ident add( const std::string & c );
	Ident lookup_or_add( const std::string & c );
};

typedef DictClass *Dict;


#endif
