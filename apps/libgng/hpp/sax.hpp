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

#ifndef LIB_SAX_HPP
#define LIB_SAX_HPP

#include <istream>
#include <vector>
#include <map>

namespace Ginger {

class SaxHandler {
public:
	virtual void startTag( std::string & name, std::map< std::string, std::string > & attrs ) = 0;
	virtual void endTag( std::string & name ) = 0;
	
public:
	virtual ~SaxHandler() {}
};

class SaxParser {
private:
	std::istream & input;
	SaxHandler & parent;
	int level;
	
private:
	void readName( std::string & name );
	void eatWhiteSpace();
	void processAttributes( std::map< std::string, std::string > & attrs );
	void readAttributeValue( std::string & attr );
	void mustReadChar( const char ch );
	char nextChar();
	char peekChar();
	
public:
	void read();
	void readElement();
	SaxParser( std::istream & in, SaxHandler & p ) :
		input( in ),
		parent( p ),
		level( 0 )
	{
	}
};


}

#endif
