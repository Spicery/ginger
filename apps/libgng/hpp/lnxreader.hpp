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

#ifndef LNX_READER_GINGER_HPP
#define LNX_READER_GINGER_HPP

//	STL
#include <map>
#include <vector>
#include <string>
#include <iostream>

//	Local libs
#include "lnxitem.hpp"
#include "mnxsax.hpp"

namespace Ginger {

class LnxHandler;

class LnxReader {
private:
	std::istream & input;

private:
	int property_count;
	std::map< std::string, int > property_index;
	std::vector< std::string > property_keys;
	std::vector< std::string > property_defaults;
	LnxItem * current_item;
	LnxHandler * handler;
	Ginger::MnxSaxParser * parser;

public:
	void put( const std::string & key, const std::string & value );
	std::vector< std::string > & propertyDefaults() { return this->property_defaults; }
	std::string & propertyKey( int n ) { return this->property_keys[ n ]; }
	
	//	This needs to check that the value is in range - otherwise throw an exception.
	int propertyIndex( const std::string & key );
	int propertyCount();
	
	void fillMap( std::map< std::string, std::string > & map );
	
public:
	LnxItem * read();
	void unread();
	void drop();
	LnxItem * peek();
	
public:
	LnxReader( std::istream & input );
	~LnxReader();
};

} // namespave

#endif
