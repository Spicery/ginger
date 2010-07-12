#ifndef HEAP_CRAWL_HPP
#define HEAP_CRAWL_HPP

#include <iostream>

#include "heap.hpp"
#include "cage.hpp"

class HeapCrawl {
private:
	std::vector< CageClass * >::iterator	cages;
	std::vector< CageClass * >::iterator	end;
	
public:
	CageClass * next();

public:
	HeapCrawl( HeapClass & h );
};

#endif