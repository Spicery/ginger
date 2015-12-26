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
#include <stdio.h>

#include "mnx.hpp"
#include "mnxsax.hpp"
#include "mishap.hpp"

namespace Ginger {
using namespace std;

bool MnxWalkPath::isLastIndex() const { return this->mnx.size() == this->index + 1; }

void mnxRenderChar( std::ostream & out, const char ch ) {
	if ( ch == '<' ) {
		out << "&lt;";
	} else if ( ch == '>' ) {
		out << "&gt;";
	} else if ( ch == '&' ) {
		out << "&amp;";
	} else if ( ch == '"' ) {
		out << "&quot;";
	} else if ( ch == '\'' ) {
		out << "&apos;";
	} else if ( 32 <= ch && ch < 127 ) {
		out << ch;
	} else {
		out << "&#" << (int)ch << ";";
	}
}

void mnxRenderText( std::ostream & out, const std::string & str ) {
	for ( std::string::const_iterator it = str.begin(); it != str.end(); ++it ) {
		const unsigned char ch = *it;
		mnxRenderChar( out, ch );
	}
}

void mnxFRenderChar( FILE * f, const char ch ) {
	if ( ch == '<' ) {
		fprintf( f, "&lt;" );
	} else if ( ch == '>' ) {
		fprintf( f, "&gt;" );
	} else if ( ch == '&' ) {
		fprintf( f,  "&amp;" );
	} else if ( ch == '"' ) {
		fprintf( f, "&quot;" );
	} else if ( ch == '\'' ) {
		fprintf( f, "&apos;" );
	} else if ( 32 <= ch && ch < 127 ) {
		fprintf( f, "%c", ch );
	} else {
		fprintf( f, "&#%d;", (int)ch );
	}
}


void mnxFRenderText( FILE * f, const std::string & str ) {
	for ( std::string::const_iterator it = str.begin(); it != str.end(); ++it ) {
		const unsigned char ch = *it;
		mnxFRenderChar( f, ch );
	}
}

void mnxRenderText( const std::string & str ) {
	mnxRenderText( cout, str );
}



void Mnx::putAttribute( const std::string & key, const std::string & value ) {
	this->attributes[ key ] = value;
}

void Mnx::putAttribute( const std::string & key, const long & value ) {
	stringstream s;
	s << value;
	this->attributes[ key ] = s.str();
}

void Mnx::putAttributeMap( std::map< std::string, std::string > & attrs ) {
	this->attributes.insert( attrs.begin(), attrs.end() );
}

bool Mnx::removeAttribute( const std::string & key ) {
	return this->attributes.erase( key ) == 1;
}

void Mnx::addChild( shared< Mnx > child ) {
	this->children.push_back( child );
}
	
shared< Mnx > Mnx::getChild( int n ) {
	return this->children.at( n );
}

void Mnx::setChild( int n, shared< Mnx > x ) {
	this->children.at( n ) = x;
}

shared< Mnx > Mnx::getFirstChild() {
	return this->children.front();
}

void Mnx::setFirstChild( shared< Mnx > x ) {
	this->children.front() = x;
}

shared< Mnx > Mnx::getLastChild() {
	return this->children.back();
}

void Mnx::setLastChild( shared< Mnx > x ) {
	this->children.back() = x;
}

const std::string & Mnx::attribute( const std::string & key ) const {
	std::map< std::string, std::string >::const_iterator it = this->attributes.find( key );
	if ( it != this->attributes.end() ) {
		return it->second;
	} else {
		throw Mishap( "No such key" ).culprit( "Key", key );
	}
}

int Mnx::attributeToBoolHelper( const std::string & key, const bool def, const bool use_def ) const {
	std::map< std::string, std::string >::const_iterator it = this->attributes.find( key );
	if ( it != this->attributes.end() ) {
		stringstream s( it->second );
		bool n;
		if ( s >> n ) {
			return n;
		} else {
			throw Mishap( "Integer attribute value needed" ).culprit( "Attribute", key ).culprit( "Value", it->second );
		}
	} else if ( use_def ) {
		return def;
	} else {
		throw Mishap( "No such key" ).culprit( "Key", key );
	}
}

int Mnx::attributeToBool( const std::string & key ) const {
	return this->attributeToBoolHelper( key, false, false );
}

int Mnx::attributeToBool( const std::string & key, const bool def ) const {
	return this->attributeToBoolHelper( key, def, true );
}

int Mnx::attributeToIntHelper( const std::string & key, const int def, const bool use_def ) const {
	std::map< std::string, std::string >::const_iterator it = this->attributes.find( key );
	if ( it != this->attributes.end() ) {
		stringstream s( it->second );
		int n;
		if ( s >> n ) {
			return n;
		} else {
			throw Mishap( "Integer attribute value needed" ).culprit( "Attribute", key ).culprit( "Value", it->second );
		}
	} else if ( use_def ) {
		return def;
	} else {
		throw Mishap( "No such key" ).culprit( "Key", key );
	}
}

int Mnx::attributeToInt( const std::string & key ) const {
	return this->attributeToIntHelper( key, 0, false );
}

int Mnx::attributeToInt( const std::string & key, const int def ) const {
	return this->attributeToIntHelper( key, def, true );
}

Maybe< long > Mnx::maybeAttributeToLongHelper( const std::string & key, const long def, const bool use_def ) const {
	std::map< std::string, std::string >::const_iterator it = this->attributes.find( key );
	if ( it != this->attributes.end() ) {
		stringstream s( it->second );
		long n;
		if ( s >> n ) {
			return Maybe< long >( n );
		} else {
			return Maybe< long >();
		}
	} else if ( use_def ) {
		return Maybe< long >( def );
	} else {
		throw Mishap( "No such key" ).culprit( "Key", key );
	}
}

long Mnx::attributeToLongHelper( const std::string & key, const long def, const bool use_def ) const {
	Maybe< long >m( maybeAttributeToLongHelper( key, def, use_def ) );
	if ( m.isValid() ) {
		return m.fastValue();
	} else {
		throw Mishap( "Cannot parse attibute as long value" ).culprit( "Attribute", key ).culprit( "Value", this->attribute( key ) );
	}
}

long Mnx::attributeToLong( const std::string & key ) const {
	return this->attributeToLongHelper( key, 0, false );
}

long Mnx::attributeToLong( const std::string & key, const long def  ) const {
	return this->attributeToLongHelper( key, def, true );
}

Maybe< long > Mnx::maybeAttributeToLong( const std::string & key, const long def ) const {
	return this->maybeAttributeToLongHelper( key, def, true );
}

Maybe< long > Mnx::maybeAttributeToLong( const std::string & key ) const {
	return this->maybeAttributeToLongHelper( key, 0, false );
}

const std::string & Mnx::attribute( const std::string & key, const std::string & def  ) const {
	std::map< std::string, std::string >::const_iterator it = this->attributes.find( key );
	return it != this->attributes.end() ? it->second : def;
}

bool Mnx::hasAttribute( const std::string & key ) const {
	return this->attributes.find( key ) != this->attributes.end();
}

bool Mnx::hasAttribute( const std::string & key, const std::string & eqval ) const {
	std::map< std::string, std::string >::const_iterator it = this->attributes.find( key );
	return it != this->attributes.end() && eqval == it->second;
}

void Mnx::frender( FILE * f ) {
	fprintf( f, "<%s", this->element_name.c_str() );
	for ( 
		std::map< std::string, std::string >::iterator it = this->attributes.begin();
		it != this->attributes.end();
		++it
	) {
		fprintf( f, " %s=\"", it->first.c_str() );
		mnxFRenderText( f, it->second.c_str() );
		fprintf( f, "\"" );
	}
	if ( this->children.empty() ) {
		fprintf( f, "/>" );
	} else {
		fprintf( f, ">" );
		for ( 
			std::vector< shared< Mnx > >::iterator it = this->children.begin();
			it != this->children.end();
			++it
		) {
			(*it)->frender( f );
		}
		fprintf( f, "</%s>", this->element_name.c_str() );
	}
}

void Mnx::render( std::ostream & out ) {
	out << "<" << this->element_name;
	for ( 
		std::map< std::string, std::string >::iterator it = this->attributes.begin();
		it != this->attributes.end();
		++it
	) {
		out << " " << it->first << "=\"";
		mnxRenderText( out, it->second );
		out << "\"";
	}
	if ( this->children.empty() ) {
		out << "/>";
	} else {
		out << ">";
		for ( 
			std::vector< shared< Mnx > >::iterator it = this->children.begin();
			it != this->children.end();
			++it
		) {
			(*it)->render( out );
		}
		out << "</" << this->element_name << ">";
	}
}

void Mnx::render() {
	this->render( cout );
}

std::string Mnx::toString() {
	ostringstream out;
	this->render( out );
	return out.str();
}


class MnxHandler : public MnxSaxHandler {
private:
	shared< Mnx > answer;
	vector< shared< Mnx > > context;
	
public:
	void startTagOpen( std::string & name ) {
		shared< Mnx > mnx( new Mnx( name ) );
		//mnx->putAttributeMap( attrs );
		this->context.push_back( mnx );
	}
	
	void put( std::string & key, std::string & value ) {
		this->context.back()->putAttribute( key, value );
	}
	
	void startTagClose( std::string & name ) {
	}
	
	void endTag( std::string & name ) {
		if ( not( this->context.empty() ) ) {
			this->answer = this->context.back();
			if ( this->answer->name() == name ) {
				this->context.pop_back();
				if ( not( this->context.empty() ) ) {
					this->context.back()->addChild( this->answer );
				}
			} else {
				throw Mishap( "Mismatched closing element name" ).culprit( "Open", this->answer->name() ).culprit( "Close", name );
			}
		} else {
			throw Mishap( "Unused closing element" ).culprit( "Name", name );
		}
	}
	
	shared< Mnx > result() {
		return answer;
	}
	
public:
	MnxHandler() {}
};

shared< Mnx > MnxReader::readMnx() {
	MnxHandler handler;
	Ginger::MnxSaxParser sax( this->in, handler );
	sax.readElement();
	return handler.result();
}

MnxVisitor::~MnxVisitor() {
}

MnxWalker::~MnxWalker() {
}

string & Mnx::name() {
	return this->element_name;
}

bool Mnx::hasName( const string & name ) {
	return this->element_name == name;
}

void Mnx::clearAllAttributes() {
	this->attributes.clear();
}

void Mnx::clearAttribute( const string & key ) {
	this->attributes.erase( key );
}

int Mnx::size() const {
	return this->children.size();
}

bool Mnx::isEmpty() const {
	return this->children.empty();
}

void Mnx::popFrontChild() {
	this->children.erase( this->children.begin() );
}

void Mnx::popLastChild() {
	this->children.erase( this->children.end() - 1 );
}

void Mnx::flattenChild( int n ) {
	vector< shared< Mnx > > kids = this->children[ n ]->children;
	vector< shared< Mnx > > ::iterator child = this->children.begin() + n;
	vector< shared< Mnx > > ::iterator after_child = this->children.erase( child );
	this->children.insert( after_child, kids.begin(), kids.end() );
}

void Mnx::visit( MnxVisitor & v ) {
	v.initialiseVisit( *this );
	this->subVisit( v );
	v.finaliseVisit( *this );
}

void Mnx::subVisit( MnxVisitor & v ) {
	v.startVisit( *this );
	
	for ( 
		vector< shared< Mnx > >::iterator it = this->children.begin();
		it != this->children.end();
		++it
	) {
		(*it)->subVisit( v );
	}

	v.endVisit( *this );
}

void Mnx::walk( MnxWalker & v, MnxWalkPath * path ) {
	v.startWalk( *this, path );
	int n = 0;
	for ( 
		vector< shared< Mnx > >::iterator it = this->children.begin();
		it != this->children.end();
		++it
	) {
		MnxWalkPath newpath( *this, n++, path );
		(*it)->walk( v, &newpath );
	}
	v.endWalk( *this, path );
}

bool Mnx::hasAnyFlags( int mask ) {
	return ( this->flags && mask ) != 0;
}

bool Mnx::hasAllFlags( int mask ) {
	return ( this->flags && mask ) == mask;
}

void Mnx::clearFlags( int mask ) {
	this->flags &= ~mask;
}

void Mnx::orFlags( int mask ) {
	this->flags |= mask;
}

void Mnx::andFlags( int mask ) {
	this->flags &= mask;
}

void Mnx::copyFrom( const Mnx & g ) {
	this->element_name = g.element_name;
	this->attributes = g.attributes;
	this->children = g.children;
	this->flags = g.flags;
}

Mnx::Mnx( const string & name ) : 
	element_name( name ), 
	flags( 0 ) 
{
}

MnxBuilder::MnxBuilder() 
{
	this->stack.push_back( shared< Mnx >( new Mnx( "dummy" ) ) );
}

MnxBuilder& MnxBuilder::start( const std::string & name ) {
	#ifdef DEBUG
		cerr << "*** START " << name << endl;
	#endif
	this->stack.push_back( shared< Mnx >( new Mnx( name ) ) );\
	return *this;
}

MnxBuilder& MnxBuilder::put( const std::string & key, const std::string & value ) {
	#ifdef DEBUG
		cerr << "*** PUT " << key << "=" << value << endl;
	#endif
	this->stack.back()->putAttribute( key, value );
	return *this;
}

MnxBuilder& MnxBuilder::put( const std::string & key, const long & value ) {
	this->stack.back()->putAttribute( key, value );
	return *this;
}

bool MnxBuilder::remove( const std::string & key ) {
	return this->stack.back()->removeAttribute( key );
}

MnxBuilder& MnxBuilder::end() {
	#ifdef DEBUG
		cerr << "*** END" << endl;
	#endif
	if ( this->stack.size() <= 1 ) {
		throw Mishap( "Too many 'ends'" );
	}
	shared< Mnx > prev( this->stack.back() );
	this->stack.pop_back();
	this->stack.back()->addChild( prev );
	return *this;
}

shared< Mnx > MnxBuilder::build() {
	#ifdef DEBUG
		cerr << "*** BUILD" << endl;
	#endif

	if ( this->stack.size() <= 0 ) {
		throw Mishap( "Too many 'builds'" );
	}
	shared< Mnx > in_progress = this->stack.back();
	
	if ( in_progress->isEmpty() ) {
		throw Mishap( "MinX construction failed (too many builds?)" );
	}
	
	shared< Mnx > answer = in_progress->getLastChild();
	in_progress->popLastChild();
	return answer;
}

MnxBuilder& MnxBuilder::add( shared< Mnx > child ) {
	#ifdef DEBUG
		cerr << "*** ADD" << endl;
	#endif
	this->stack.back()->addChild( child );
	return *this;
}

void MnxBuilder::save() {
	#ifdef DEBUG
		cerr << "*** SAVE" << endl;
	#endif
	this->put_aside.push_back( this->build() );
}

void MnxBuilder::restore() {
	#ifdef DEBUG
		cerr << "*** RESTORE" << endl;
	#endif
	if ( this->put_aside.empty() ) {
		throw Mishap( "Too many 'restores'" );
	}
	this->add( this->put_aside.back() );
	this->put_aside.pop_back();
}



/*class PrettyPrint {
private:
	std::ostream & out;
	std::string indentation;
	
public:
	PrettyPrint( std::ostream & out, const std::string & ind ) : 
		out( out ), 
		indentation( ind ) 
	{}

private:
	void indent( int level ) {
		for ( int n = 0; n < level; n++ ) {
			out << this->indentation;
		}
	}
	
public:
	void pretty( Mnx & mnx, int level ) {
		indent( level );
		out << "<" << mnx.name();
		
		MnxEntryIterator keys( mnx );
		while ( keys.hasNext() ) {
			MnxEntry & it = keys.next();
			out << " " << it.first << "=\"";
			mnxRenderText( out, it.second );
			out << "\"";
		}
		
		if ( mnx.isEmpty() ) {
			out << "/>" << endl;
		} else {
			out << ">" << endl;
			
			MnxChildIterator kids( mnx );
			while ( kids.hasNext() ) {
				SharedMnx & g = kids.next();
				this->pretty( *g, level + 1 );
			}

			indent( level );
			out << "</" << mnx.name() << ">" << endl;
		}	
	}
};*/

void Mnx::prettyPrint( std::ostream & out, const std::string & indentation ) {
	PrettyPrint pp( out, indentation.c_str() );
	pp.pretty( *this, 0 );
}

void Mnx::prettyPrint() {
	string indentation( "    " );
	this->prettyPrint( cout, indentation );
}

void Mnx::prettyPrint( const std::string & indentation ) {
	this->prettyPrint( cout, indentation );
}

void Mnx::prettyPrint( std::ostream & out ) {
	this->prettyPrint( out, "    " );
}

MnxChildIterator::MnxChildIterator( SharedMnx mnx ) {
	this->it = mnx->children.begin();
	this->end = mnx->children.end();
}

MnxChildIterator::MnxChildIterator( Mnx & mnx ) {
	this->it = mnx.children.begin();
	this->end = mnx.children.end();
}

bool MnxChildIterator::operator !() const {
	return this->it == this->end;
}

SharedMnx & MnxChildIterator::operator *() const {
	return *this->it;
}

MnxChildIterator & MnxChildIterator::operator ++() {
	++this->it;
	return *this;
}

bool MnxChildIterator::hasNext() {
	return this->it != this->end;
}

SharedMnx & MnxChildIterator::next() {
	return *this->it++;
}

MnxEntryIterator::MnxEntryIterator( SharedMnx mnx ) {
	this->it = mnx->attributes.begin();
	this->end = mnx->attributes.end();
}

MnxEntryIterator::MnxEntryIterator( Mnx & mnx ) {
	this->it = mnx.attributes.begin();
	this->end = mnx.attributes.end();
}

bool MnxEntryIterator::hasNext() {
	return this->it != this->end;
}

MnxEntry & MnxEntryIterator::next() {
	return *this->it++;
}

MnxRenderer::MnxRenderer( const char * ind ) : indentation( ind ) {
	//	Nothing.
}

MnxRenderer::~MnxRenderer() {
	//	Nothing.
}

void MnxRenderer::endl() {
	if ( this->indentation != NULL ) {
		this->out( '\n' );
	}
}

void MnxRenderer::indent( const int level ) {
	if ( this->indentation != NULL ) {
		for ( int n = 0; n < level; n++ ) {
			this->out( this->indentation );
		}
	}
}

void MnxRenderer::renderText( const std::string & str ) {
	for ( std::string::const_iterator it = str.begin(); it != str.end(); ++it ) {
		this->renderChar( *it );
	}
}

void MnxRenderer::renderChar( const char ch ) {
	if ( ch == '<' ) {
		this->out( "&lt;" );
	} else if ( ch == '>' ) {
		this->out( "&gt;" );
	} else if ( ch == '&' ) {
		this->out( "&amp;" );
	} else if ( ch == '"' ) {
		this->out( "&quot;" );
	} else if ( ch == '\'' ) {
		this->out( "&apos;" );
	} else if ( 32 <= ch && ch < 127 ) {
		this->out( ch );
	} else {
		stringstream s;
		s << (int)ch;
		this->out( "&#" );
		this->out( s.str().c_str() );
		this->out( ';' );
	}
}

//	Default that can be overridden.
void MnxRenderer::out( const char * text ) {
	char ch;
	while ( ( ch = *text++ ) != 0 ) {
		this->out( ch );
	}
}

void MnxRenderer::out( const std::string & text ) {
	this->out( text.c_str() );
}


void MnxRenderer::renderMnx( Mnx & mnx, const int level ) {
	this->indent( level );
	this->out( '<' );
	this->out( mnx.name() );
	
	MnxEntryIterator keys( mnx );
	while ( keys.hasNext() ) {
		MnxEntry & it = keys.next();
		this->out( ' ' );
		this->out( it.first );
		this->out( "=\"" );
		this->renderText( it.second );
		this->out( "\"" );
	}
	
	if ( mnx.isEmpty() ) {
		this->out( "/>" );
		this->endl();
	} else {
		this->out( '>' );
		this->endl();
		
		MnxChildIterator kids( mnx );
		while ( kids.hasNext() ) {
			SharedMnx & g = kids.next();
			this->pretty( *g, level + 1 );
		}

		this->indent( level );
		this->out( "</" );
		this->out( mnx.name() );
		this->out( '>' );
		this->endl();
	}		
}


} 	//	namespace
