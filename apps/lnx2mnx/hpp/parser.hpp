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

#ifndef XSONPARSER_HPP
#define XSONPARSER_HPP

//	Ginger library.
#include "mnx.hpp"
#include "lnxreader.hpp"

//	This project.
//#include "item_factory.hpp"

namespace LNX2MNX_NS {


class Parser {
public:
	Ginger::SharedMnx mishapDesc;
	
private:
	Ginger::MnxBuilder builder;
	Ginger::LnxReader & itemf;
	std::string start;
	std::map< std::string, std::vector< Ginger::SharedMnx > > rules;
	std::string token_print_property;
	
	void init( Ginger::SharedMnx grammar );
	void loadRules( Ginger::SharedMnx g );
	void load1Rule( Ginger::SharedMnx g );

public:
	std::vector< Ginger::SharedMnx > & getRule( const std::string & key );
	Ginger::MnxBuilder & getBuilder() { return builder; }
	Ginger::LnxItem * peek();
	void drop();

public:
	Ginger::SharedMnx parse();
	Parser( Ginger::LnxReader & itemf, Ginger::SharedMnx grammar );
};

class RuleParser {
private:
	Parser * parent;
	Ginger::MnxBuilder & builder;
	int depth;
	const std::string init_state;
	float precedence;
	Ginger::LnxItem * item;
	
public:
	Ginger::LnxItem * peek();
	void drop();
	void parse();
	void parseFromState( const std::string & state, float precedence );
	void parseFromState( const std::string & state );
	void processChildren( Ginger::SharedMnx & g );
	Ginger::SharedMnx findMatchingRule( const std::string & state, Ginger::LnxItem * item, const bool throwVsReturnNull );
	bool evaluateCondition( Ginger::LnxItem * item, Ginger::SharedMnx & g, const bool readVsPeek );

public:	
	//	Error message (copy & paste to be refactored)
	void unexpectedToken( Ginger::LnxItem * item );
	void parserFailed( Ginger::LnxItem * item );


public:
	void processAction( Ginger::SharedMnx & g );
	void parseAction( Ginger::SharedMnx & g );
	void parseListAction( Ginger::SharedMnx & action, const bool readVsPeek );
	void putAction( Ginger::SharedMnx & action );
	void elementAction( Ginger::SharedMnx & action );
	void saveAction();
	void restoreAction();
	void mustReadAction( Ginger::SharedMnx & action );
	void ifUnlessAction( Ginger::SharedMnx & action, const bool ifVsUnless, const bool readVsPeek );
	void whileUntilAction( Ginger::SharedMnx & action, const bool whileVsUntil, const bool readVsPeek );
public:
	RuleParser( Parser * parent, const std::string & state, float precedence, Ginger::LnxItem * item );
};

} // namespace XSON2GNX {

#endif
