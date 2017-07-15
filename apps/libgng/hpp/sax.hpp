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

#ifndef GINGER_SAX_HPP
#define GINGER_SAX_HPP

#include <istream>
#include <vector>
#include <map>
#include <memory>

namespace Ginger {

class SaxHandler {
public:
	virtual void startTag( std::string & name, std::map< std::string, std::string > & attrs ) = 0;
	virtual void endTag( std::string & name ) = 0;
	
public:
	virtual ~SaxHandler() {}
};

class SaxInput {
public:
	virtual void mustReadChar( const char ch ) = 0;
	virtual bool tryReadChar( const char ch ) = 0;
	virtual char nextChar() = 0;
	virtual char peekChar() = 0;
	virtual void pushChar( const char ch ) = 0;
	virtual bool isEof() = 0;
public:
	virtual ~SaxInput() {}
};

class IStreamSaxInput : public SaxInput {
private: 
	std::istream & input;
	
public:
	virtual void mustReadChar( const char ch );
	virtual bool tryReadChar( const char ch );
	virtual char nextChar();
	virtual char peekChar();
	virtual void pushChar( const char ch );
	virtual bool isEof();
	
public:
	IStreamSaxInput( std::istream & input );
	virtual ~IStreamSaxInput() {}
};

class FileStreamSaxInput : public SaxInput {
private: 
	FILE * file;
	
public:
	virtual void mustReadChar( const char ch );
	virtual bool tryReadChar( const char ch );
	virtual char nextChar();
	virtual char peekChar();
	virtual void pushChar( const char ch );
	virtual bool isEof();
	
public:
	FileStreamSaxInput( FILE * file );
	virtual ~FileStreamSaxInput() {}
};

class SaxParser {
private:
	std::unique_ptr< SaxInput > input;
	//std::istream & input;
	SaxHandler & parent;
	int level;
	bool finished;
	
private:
	void readName( std::string & name );
	void eatWhiteSpace();
	void processAttributes( std::map< std::string, std::string > & attrs );
	void readAttributeValue( std::string & attr );
	//void mustReadChar( const char ch );
	//char nextChar();
	//char peekChar();
	
public:
	void read();
	void readElement();
	SaxParser( std::istream & in, SaxHandler & p ) :
		input( new IStreamSaxInput( in ) ),
		//input( in ),
		parent( p ),
		level( 0 ),
		finished( false )
	{
	}
	SaxParser( FILE * in, SaxHandler & p ) :
		input( new FileStreamSaxInput( in ) ),
		//input( in ),
		parent( p ),
		level( 0 ),
		finished( false )
	{
	}
};


}

#endif
