#include "node.hpp"

//----- Node Factory -----------------------------------------------------------

NodeFactory::NodeFactory( const std::string & name ) :
	data( new ElementData() )
{
	this->data->element_name = name;
}

void NodeFactory::setName( const std::string & name ) {
	this->data->element_name = name;
}

void NodeFactory::putAttr( const std::string & name, const std::string & value ) {
	this->data->attributes[ name ] = value;
}

void NodeFactory::addNode( const Node & node ) {
	this->data->children.push_back( node );
}

void NodeFactory::addText( const std::string & text ) {
	this->data->children.push_back( shared< NodeClass >( new TextNodeClass( text ) ) );
}

void NodeFactory::start( const std::string & name ) {
	this->dump.push_back( this->data );
	this->data = shared< ElementData >( new ElementData() );
	this->data->element_name = name;
}

void NodeFactory::end() {
	Node c = this->node();
	this->data = this->dump.back();
	this->dump.pop_back();
	this->addNode( c );
}

void NodeFactory::end( const std::string & name ) {
	if ( name != this->data->element_name ) {
		throw "names do not match";
	}
	this->end();
}

Node NodeFactory::node() {
	return shared< NodeClass >( new ElementNodeClass( *this->data ) );
}

//------ NodeClass -------------------------------------------------------------



//----- ElementNodeClass -------------------------------------------------------

const std::string ElementNodeClass::get( const std::string & key ) const { 
	std::map< std::string, std::string > a = this->data.attributes;
	std::map< std::string, std::string >::iterator it = a.find( key );
	if ( it == a.end() ) throw;
	return it->second;
}

void ElementNodeClass::updateAsPattern() {
	if ( this->data.element_name == "id" ) {
		this->data.element_name = "var";
		this->data.attributes[ "protected" ] = "true";
	}
	for ( 
		std::vector< shared< NodeClass > >::iterator it = this->data.children.begin();
		it != this->data.children.end();
		++it
	) {
		//(*it)->updateAsPattern();
	}
}

static void renderText( std::ostream & out, const std::string & str ) {
	for ( std::string::const_iterator it = str.begin(); it != str.end(); ++it ) {
		const unsigned char ch = *it;
		if ( ch == '<' ) {
			out << "&lt;";
		} else if ( ch == '>' ) {
			out << "&gt;";
		} else if ( ch == '&' ) {
			out << "&amp;";
		} else if ( 32 <= ch && ch < 127 ) {
			out << ch;
		} else {
			out << "&#" << (int)ch << ";";
		}
	}
}

void ElementNodeClass::render( std::ostream & out ) {
	out << "<" << this->data.element_name;
	for ( 
		std::map< std::string, std::string >::iterator it = this->data.attributes.begin();
		it != this->data.attributes.end();
		++it
	) {
		//	INCORRECT - second must be escaped
		out << " " << it->first << "=\"";
		renderText( out, it->second );
		out << "\"";
	}
	if ( this->data.children.empty() ) {
		out << "/>";
	} else {
		out << ">";
		for ( 
			std::vector< shared< NodeClass > >::iterator it = this->data.children.begin();
			it != this->data.children.end();
			++it
		) {
			(*it)->render( out );
		}
		out << "</" << this->data.element_name << ">";		
	}
}


//----- TextNode ---------------------------------------------------------------


const std::string TextNodeClass::get( const std::string & key ) const {	
	if ( key == "text" ) {
		return this->text_string;
	} else {
		return std::string();
	}
}

void TextNodeClass::render( std::ostream & out ) {
	//	INCORRECT - now text is escape but the formatting is lost
	renderText( out, this->text_string );
}

void TextNodeClass::updateAsPattern() {
}
