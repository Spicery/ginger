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

#ifndef XSONPARSER_HPP
#define XSONPARSER_HPP

//	AppGinger library.
#include "gnx.hpp"

//	This project.
#include "item_factory.hpp"

namespace XSON2GNX {


class XSONParser {
private:
	Ginger::GnxBuilder builder;
	ItemFactory itemf;
	std::string start;
	std::map< std::string, std::vector< Ginger::SharedGnx > > rules;
	
	void init( Ginger::SharedGnx grammar );
	void loadRules( Ginger::SharedGnx g );
	void load1Rule( Ginger::SharedGnx g );

public:
	std::vector< Ginger::SharedGnx > & getRule( const std::string & key );
	Ginger::GnxBuilder & getBuilder() { return builder; }
	Item peek();
	void drop();

public:
	Ginger::SharedGnx parse();
	XSONParser( ItemFactory itemf, Ginger::SharedGnx grammar );
};

class RuleParser {
private:
	XSONParser * parent;
	Ginger::GnxBuilder & builder;
	const std::string state;
	float precedence;
	Item item;
	
public:
	Item peek();
	void drop();
	void parse();
	void parseFromState( const std::string & state, float precedence );
	void parseFromState( const std::string & state );
	void processChildren( Ginger::SharedGnx & g );
	Ginger::SharedGnx findMatchingRule( const std::string & state, const Item item, const bool throwVsReturnNull );
	bool evaluateCondition( Item item, Ginger::SharedGnx & g, const bool readVsPeek );

public:
	void processAction( Ginger::SharedGnx & g );
	void parseAction( Ginger::SharedGnx & g );
	void parseListAction( Ginger::SharedGnx & action, const bool readVsPeek );
	void putAction( Ginger::SharedGnx & action );
	void elementAction( Ginger::SharedGnx & action );
	void saveAction();
	void restoreAction();
	void mustReadAction( Ginger::SharedGnx & action );
	void ifUnlessAction( Ginger::SharedGnx & action, const bool ifVsUnless, const bool readVsPeek );
	void whileUntilAction( Ginger::SharedGnx & action, const bool whileVsUntil, const bool readVsPeek );
public:
	RuleParser( XSONParser * parent, const std::string & state, float precedence, Item item );
};

} // namespace XSON2GNX {

#endif
