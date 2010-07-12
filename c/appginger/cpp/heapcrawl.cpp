/******************************************************************************\
	Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
	of the GNU General Public License. This file is part of AppGinger.

    AppGinger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AppGinger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AppGinger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

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
