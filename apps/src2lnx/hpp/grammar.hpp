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

#ifndef GRAMMAR_SRC2LNX_HPP
#define GRAMMAR_SRC2LNX_HPP

//	STL
#include <map>
#include <string>

//	Ginger libs
#include "mnx.hpp"

//	Local libs
#include "state.hpp"
#include "node.hpp"
#include "mapping.hpp"

namespace SRC2LNX_NS {

class Grammar {
private:
	State * state;
	int node_count;
	std::map< std::string, int > node_index;
	std::vector< Node * > nodes;

private:	
	void initEmptyNodes(
		State * state, 
		Ginger::SharedMnx description,
		std::vector< Ginger::SharedMnx > & node_desc,
		std::vector< Ginger::SharedMnx > & mapping_desc,
		std::map< std::string, std::string > & named_categories
	);

	void initStartNode( Ginger::SharedMnx description );

	void initArcs( std::vector< Ginger::SharedMnx > & node_desc, std::map< std::string, std::string > & named_categories );
	void initMappings( std::vector< Ginger::SharedMnx > & mapping_desc );
	
	void initMatch( Node * node, Ginger::SharedMnx arcd, Arc * arc, std::map< std::string, std::string > & named_categories );
	void initAction( Ginger::SharedMnx actiond, Arc * arc );
	
public:
	void start();
	bool processChar( char ch );
	void processEof();
	void finish();
	
public:
	Grammar( State * state, Ginger::SharedMnx description );
};

} // namespace

#endif
