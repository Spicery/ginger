#include "heapcrawl.hpp"

CageClass * HeapCrawl::next() {
	if ( this->cages == this->end ) {
		return static_cast< CageClass * >( 0 );
	} else {
		return *this->cages++;
	}
}

 HeapCrawl::HeapCrawl( HeapClass & h ) {
	//	Change to semantics of current - now points to an element of the zoo.
	//	this->zoo.push_back( h.current );
	//	this->zoo.insert( this->zoo.begin(), h.zoo.begin(), h.zoo.end() );
	//std::cout << "There are " << h.zoo.size() << " cages" << std::endl;
	//flush( std::cout );
	this->cages = h.zoo.begin();
	this->end = h.zoo.end();
}
