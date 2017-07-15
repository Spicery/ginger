/******************************************************************************\
	Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
	of the GNU General Public License. This file is part of Ginger.

    Ginger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Ginger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Ginger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

#ifndef MAPPING_SRC2LNX_HPP
#define MAPPING_SRC2LNX_HPP

//	STL
#include <string>
#include <map>


namespace SRC2LNX_NS {

class State;

class Mapping {
public:
	
private:
	const int src;
	const int dst;
	std::map< std::string, std::string > mapping;
	
public:
	void put( const std::string & key, const std::string & value );
	void update( State * state );
	
public:
	Mapping(
		int src,
		int dst
	) :
		src( src ),
		dst( dst )
	{}
};

	
} // namespace

#endif
