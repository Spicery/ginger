#include "heapcrawl.hpp"

CageClass * HeapCrawl::next() {
	if ( this->cages == this->zoo.end() ) {
		return (CageClass *)0;
	} else {
		return *this->cages++;
	}
}
