#ifndef READ_XML_HPP
#define READ_XML_HPP

#include <istream>
#include <vector>
#include <map>

#include "term.hpp"

class ReadXmlClass;

class SaxClass {
private:
	std::istream & input;
	ReadXmlClass & parent;
	
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
	SaxClass( std::istream & in, ReadXmlClass & p ) :
		input( in ),
		parent( p )
	{
	}
};

class TermData {
public:
	std::string name;
	std::map< std::string, std::string > attrs;
	std::vector< Term > kids;
	
public:
	Term makeTerm();

public:
	TermData() 
	{
	}
};

class ReadXmlClass {
private:
	SaxClass sax;

	std::vector< Term > term_stack;
	std::vector< TermData > tag_stack;
	
	
	
public:
	void startTag( std::string & name, std::map< std::string, std::string > & attrs );
	void endTag( std::string & name );
	Term readElement();
	
public:
	ReadXmlClass( std::istream & in ) : 
		sax( SaxClass( in, *this ) )
	{
	}
};


#endif