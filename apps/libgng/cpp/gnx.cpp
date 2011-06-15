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

#include <iostream>
#include <sstream>

#include "gnx.hpp"
#include "sax.hpp"
#include "mishap.hpp"


namespace Ginger {
using namespace std;

void gnxRenderText( std::ostream & out, const std::string & str ) {
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

void gnxRenderText( const std::string & str ) {
	gnxRenderText( cout, str );
}

void Gnx::putAttribute( const std::string & key, const std::string & value ) {
	this->attributes[ key ] = value;
}

void Gnx::putAttribute( const std::string & key, const int & value ) {
	stringstream s;
	s << value;
	this->attributes[ key ] = s.str();
}

void Gnx::putAttributeMap( std::map< std::string, std::string > & attrs ) {
	this->attributes.insert( attrs.begin(), attrs.end() );
}

void Gnx::addChild( shared< Gnx > child ) {
	this->children.push_back( child );
}
	
shared< Gnx > & Gnx::child( int n ) {
	return this->children.at( n );
}

shared< Gnx > & Gnx::lastChild() {
	return this->children.back();
}

shared< Gnx > & Gnx::firstChild() {
	return this->children.front();
}

const std::string & Gnx::attribute( const std::string & key ) const {
	std::map< std::string, std::string >::const_iterator it = this->attributes.find( key );
	if ( it != this->attributes.end() ) {
		return it->second;
	} else {
		throw Mishap( "No such key" ).culprit( "Key", key );
	}
}

const std::string & Gnx::attribute( const std::string & key, const std::string & def  ) const {
	std::map< std::string, std::string >::const_iterator it = this->attributes.find( key );
	return it != this->attributes.end() ? it->second : def;
}

bool Gnx::hasAttribute( const std::string & key ) const {
	return this->attributes.find( key ) != this->attributes.end();
}

bool Gnx::hasAttribute( const std::string & key, const std::string & eqval ) const {
	std::map< std::string, std::string >::const_iterator it = this->attributes.find( key );
	return it != this->attributes.end() && eqval == it->second;
}

class PrettyPrint {
private:
	std::ostream & out;
	
public:
	PrettyPrint( std::ostream & out ) : out( out ) {}

private:
	void indent( int level ) {
		for ( int n = 0; n < level; n++ ) {
			out << ' ';
		}
	}
	
public:
	void pretty( Gnx & gnx, int level ) {
		indent( level );
		out << "<" << gnx.name();	
		for ( 
			std::map< std::string, std::string >::iterator it = gnx.attributes.begin();
			it != gnx.attributes.end();
			++it
		) {
			out << " " << it->first << "=\"";
			gnxRenderText( out, it->second );
			out << "\"";
		}
		if ( gnx.children.empty() ) {
			out << "/>" << endl;
		} else {
			out << ">" << endl;
			for ( 
				std::vector< shared< Gnx > >::iterator it = gnx.children.begin();
				it != gnx.children.end();
				++it
			) {
				Gnx & g = **it;
				this->pretty( g, level + 1 );
			}
			indent( level );
			out << "</" << gnx.name() << ">" << endl;
		}	
	}
};

void Gnx::prettyPrint( std::ostream & out ) {
	PrettyPrint pp( out );
	pp.pretty( *this, 0 );
}

void Gnx::prettyPrint() {
	this->prettyPrint( cout );
}



void Gnx::render( std::ostream & out ) {
	out << "<" << this->element_name;
	for ( 
		std::map< std::string, std::string >::iterator it = this->attributes.begin();
		it != this->attributes.end();
		++it
	) {
		out << " " << it->first << "=\"";
		gnxRenderText( out, it->second );
		out << "\"";
	}
	if ( this->children.empty() ) {
		out << "/>";
	} else {
		out << ">";
		for ( 
			std::vector< shared< Gnx > >::iterator it = this->children.begin();
			it != this->children.end();
			++it
		) {
			(*it)->render( out );
		}
		out << "</" << this->element_name << ">";
	}
}

void Gnx::render() {
	this->render( cout );
}

class GnxHandler : public SaxHandler {
private:
	shared< Gnx > answer;
	vector< shared< Gnx > > context;
	
public:
	void startTag( std::string & name, std::map< std::string, std::string > & attrs ) {
		shared< Gnx > gnx( new Gnx( name ) );
		gnx->putAttributeMap( attrs );
		this->context.push_back( gnx );
	}
	
	void endTag( std::string & name ) {
		if ( not( this->context.empty() ) ) {
			this->answer = this->context.back();
			this->context.pop_back();
			if ( not( this->context.empty() ) ) {
				this->context.back()->addChild( this->answer );
			}
		}
	}
	
	shared< Gnx > result() {
		return answer;
	}
	
public:
	GnxHandler() {
	}
};

shared< Gnx > GnxReader::readGnx() {
	GnxHandler handler;
	Ginger::SaxParser sax( this->in, handler );
	sax.readElement();
	return handler.result();
}

GnxVisitor::~GnxVisitor() {
}

string & Gnx::name() {
	return this->element_name;
}

void Gnx::clearAllAttributes() {
	this->attributes.clear();
}

void Gnx::clearAttribute( const string & key ) {
	this->attributes.erase( key );
}

int Gnx::size() const {
	return this->children.size();
}

void Gnx::popFrontChild() {
	this->children.erase( this->children.begin() );
}

void Gnx::popLastChild() {
	this->children.erase( this->children.end() - 1 );
}

void Gnx::flattenChild( int n ) {
	vector< shared< Gnx > > kids = this->children[ n ]->children;
	vector< shared< Gnx > > ::iterator child = this->children.begin() + n;
	vector< shared< Gnx > > ::iterator after_child = this->children.erase( child );
	this->children.insert( after_child, kids.begin(), kids.end() );
}


void Gnx::visit( GnxVisitor & v ) {
	v.startVisit( *this );
	for ( 
		vector< shared< Gnx > >::iterator it = this->children.begin();
		it != this->children.end();
		++it
	) {
		(*it)->visit( v );
	}
	v.endVisit( *this );
}

bool Gnx::hasAnyFlags( int mask ) {
	return this->flags && mask != 0;
}

bool Gnx::hasAllFlags( int mask ) {
	return this->flags && mask == mask;
}

void Gnx::clearFlags( int mask ) {
	this->flags &= ~mask;
}

void Gnx::orFlags( int mask ) {
	this->flags |= mask;
}

void Gnx::andFlags( int mask ) {
	this->flags &= mask;
}

void Gnx::copyFrom( const Gnx & g ) {
	this->element_name = g.element_name;
	this->attributes = g.attributes;
	this->children = g.children;
	this->flags = g.flags;
}

Gnx::Gnx( const string & name ) : 
	element_name( name ), 
	flags( 0 ) 
{
}

GnxBuilder::GnxBuilder() :
	answer( shared< Gnx >() )
{
}

void GnxBuilder::start( const std::string & name ) {
	this->stack.push_back( shared< Gnx >( new Gnx( name ) ) );
}

void GnxBuilder::put( const std::string & key, const std::string & value ) {
	this->stack.back()->putAttribute( key, value );
}

void GnxBuilder::end() {
	shared< Gnx > prev( this->stack.back() );
	this->stack.pop_back();
	if ( this->stack.empty() ) {
		this->answer = prev;
	} else {
		this->stack.back()->addChild( prev );
	}
}

shared< Gnx > & GnxBuilder::build() {
	if ( !this->stack.empty() ) {
		shared< Gnx > & in_progress = this->stack.back();
		this->answer = in_progress->lastChild();
		in_progress->popLastChild();
	}
	return this->answer;
}

void GnxBuilder::add( shared< Gnx > & child ) {
	this->stack.back()->addChild( child );
}



} 	//	namespace
