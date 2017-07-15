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

#ifndef STATE_SRC2LNX_HPP
#define STATE_SRC2LNX_HPP

//	STLs
#include <string>
#include <map>
#include <vector>


namespace SRC2LNX_NS {

//	Forward declaration.
class Mapping;

class State {
public:
	bool ok;

public:
	int lineno;
	bool lineno_needed;
	std::string lineno_property;
	
public:
	std::vector< Mapping * > mappings;
	
private:
	int node_index;

private:
	int prop_count;
	std::map< std::string, int > property_index;
	std::vector< bool > property_output;
	std::vector< std::string > property_name;
	std::vector< std::string > property_default;
	std::vector< std::string > property_value;

public:
	int & nodeIndex() { return this->node_index; }

private:
	void runMappings();
	void emitContents();

public:
	int count() { return this->prop_count; }
	std::string & name( int n );
	std::string & def( int n );
	void addProperty( const std::string & name, const std::string & def, bool output );
	int propIndex( const std::string & name );
	void pushToIndex( int index, char ch );
	void pushToIndex( int index, const std::string & value );
	void pushToIndexFromIndex( int to_index, int from_index );
	void emit();
	void emitHead();
	void emitTail();
	void reset( int index );
	std::string & propValue( int n ) { return this->property_value[ n ]; }

public:
	State();
};

} // SRC2LNX_NS

#endif
