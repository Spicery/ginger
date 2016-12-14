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

#ifndef MNXSAX_GINGER_HPP
#define MNXSAX_GINGER_HPP

#include <istream>
#include <vector>
#include <map>
#include <memory>

#include "mnxsrc.hpp"
#include "rdlmnxsrc.hpp"

namespace Ginger {

class MnxSaxHandler {
public:
	virtual void startTagOpen( std::string & name ) = 0;
	virtual void put( std::string & key, std::string & value ) = 0;
	virtual void startTagClose( std::string & name ) = 0;
	virtual void endTag( std::string & name ) = 0;
	
public:
	virtual ~MnxSaxHandler() {}
};

class MnxSaxParser {
private:
	std::shared_ptr< MnxSource > input;
	MnxSaxHandler & parent;
	std::string tag_name;
	bool pending_end_tag;
	int level;
	
private:
	void readName( std::string & name );
	void eatWhiteSpace();
	void processAttributes(); // std::map< std::string, std::string > & attrs );
	void readAttributeValue( std::string & attr );
	void mustReadChar( const char ch );
	char nextChar();
	char peekChar();
	
public:
	/**
		Each read will consume input and invoke the callbacks until it
		comes to the end of a start tag or stop tag. In the case of an
		empty element, it will treat it a separate start and end tag.
	*/
	void read();

public:
	/**
		Returns true if an element was read off the input. Returns false
		if the end of input was encountered before non-whitespace characters.
		Otherwise mishaps.
	*/
	bool readElement();
	
public:
	MnxSaxParser( std::istream & in, MnxSaxHandler & p ) :
		input( new InputStreamMnxSource( in ) ),
		parent( p ),
		pending_end_tag( false ),
		level( 0 )
	{
	}

	MnxSaxParser( std::shared_ptr< MnxSource > in, MnxSaxHandler & p ) :
		input( in ),
		parent( p ),
		pending_end_tag( false ),
		level( 0 )
	{
	}

	MnxSaxParser( MnxSource & in, MnxSaxHandler & p ) :
		input( new ProxyMnxSource( in ) ),
		parent( p ),
		pending_end_tag( false ),
		level( 0 )
	{
	}

	MnxSaxParser( MnxSaxHandler & p ) :
		input( new ReadlineMnxSource() ),
		parent( p ),
		pending_end_tag( false ),
		level( 0 )
	{
	}


};


}

#endif
