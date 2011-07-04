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

#ifndef SRC2LNX_ARC_HPP
#define SRC2LNX_ARC_HPP

//	STL
#include <vector>

//	Local libs
#include "action.hpp"
#include "match.hpp"

namespace SRC2LNX_NS {

class Arc {
private:
	Match * match_ptr;
	std::vector< Action * > action_vector;
	
public:	
	void addAction( Action * action );
	std::vector< Action * > & actions() { return this->action_vector; }
	Match * & match() { return this->match_ptr; }
	void runActions( char ch );

public:
	Arc() : match_ptr( NULL ) {}
};

} // namespace

#endif
