#ifndef NODE_HPP
#define NODE_HPP

#include <string>
#include <vector>
#include <map>
#include <iostream>

#include "shared.hpp"

class NodeClass;

struct ElementData {
	std::string element_name;
	std::vector< shared< NodeClass > > children;
	std::map< std::string, std::string > attributes;
};

class NodeFactory {
private:
	shared< ElementData > data;	
	std::vector< shared< ElementData > > dump;

public:
	void setName( const std::string & name );
	void putAttr( const std::string & name, const std::string & value );
	void addNode( const shared< NodeClass > & node );
	void addText( const std::string & text );
	
	void start( const std::string & name );
	void end();
	void end( const std::string & name );
	
	shared< NodeClass > node();

public:
	NodeFactory( const std::string & name );
	NodeFactory();
	
	virtual ~NodeFactory() {}
};


/*
	Presents a uniform interface for both elements and text.
	
	Node are element-like, they have a tag, attributes and
	a sequence of children. The mapping of elements to nodes
	is trivial.
	
	Text is modelled as a node with tag "text", a single
	attribute "text" and no children.
*/
class NodeClass {
public:
	virtual bool isElement() = 0;
	virtual void render( std::ostream & out ) = 0;
	virtual const std::string elementName() = 0;
	virtual int size() const = 0;
	virtual shared< NodeClass > child( const int n ) const = 0;
	virtual const std::string get( const std::string & key ) const = 0;
	virtual void updateAsPattern() = 0;
	
public:
	virtual ~NodeClass() {
	}
};

class ElementNodeClass : public NodeClass {
public:
	ElementData data;
	
public:
	bool isElement() { return true; }
	const std::string elementName() { return this->data.element_name; }
	const std::string get( const std::string & key ) const;
	int size() const {
		return this->data.children.size();
	}
	shared< NodeClass > child( const int n ) const {
		return this->data.children[ n ];
	}	
	void render( std::ostream & out );
	
public:
	virtual void updateAsPattern();


public:
	ElementNodeClass( const ElementData & f ) :
		data( f )
	{
	}
	
	virtual ~ElementNodeClass() {}
};

class TextNodeClass : public NodeClass {
private:
	const std::string text_string;

public:
	const std::string elementName() { return "text"; }
	int size() const { return 0; }
	shared<NodeClass> child( int n ) const { throw; }
	const std::string get( const std::string& key ) const;
	
	virtual void updateAsPattern();

	
public:
	const std::string & text() const {
		return this->text_string;
	}

public:
	bool isElement() { return false; }

	
public:
	void render( std::ostream & out ) ;


public:
	TextNodeClass( const std::string & s ) :
		text_string( s )
	{
	}
	
	virtual ~TextNodeClass() {}
};

typedef shared< NodeClass > Node;


#endif
