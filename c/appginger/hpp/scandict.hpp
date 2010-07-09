#ifndef SCAN_DICT_HPP
#define SCAN_DICT_HPP

#include "dict.hpp"

class ScanDict {
private:
	DictClass * dict;
	std::map< std::string, Ident >::iterator it;

public:
	Ref * next();
	
public:
	ScanDict( DictClass * d );
};

#endif