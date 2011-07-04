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

#ifndef NODE_SRC2LNX_HPP
#define NODE_SRC2LNX_HPP

//	STL
#include <vector>
#include <string>

//	Local libs
#include "arc.hpp"
#include "match.hpp"

namespace SRC2LNX_NS {

class Node {
private:
	const std::string name;
	Arc * eof_arc;
	std::vector< Arc * > arc_vector;

public:
	const std::string getName() const { return this->name; }
	void addArc( Arc * arc );
	std::vector< Arc * > & arcs() { return this->arc_vector; }
	Arc * & eofArc() { return this->eof_arc; }
	
public:
	Node( const std::string & name ) : name( name ), eof_arc( NULL ) {}
};


} // namespace

#endif
