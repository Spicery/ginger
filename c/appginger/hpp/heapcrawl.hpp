#ifndef HEAP_CRAWL_HPP
#define HEAP_CRAWL_HPP

#include <iostream>

#include "heap.hpp"
#include "cage.hpp"

class HeapCrawl {
private:
	std::vector< CageClass * >			 	zoo;
	std::vector< CageClass * >::iterator	cages;
	
public:
	CageClass * next();

public:
	HeapCrawl( HeapClass & h ) {
		this->zoo.push_back( h.current );
		this->zoo.insert( this->zoo.begin(), h.zoo.begin(), h.zoo.end() );
		this->cages = this->zoo.begin();
		std::cout << "#Cages = " << this->zoo.size() << std::endl;
	}

};

#endif