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

#ifndef GINGER_GNX_HPP
#define GINGER_GNX_HPP

#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <map>

#include "shared.hpp"


namespace Ginger {

extern void gnxRenderText( std::ostream & out, const std::string & string );
extern void gnxRenderText( const std::string & string );

class Gnx;
typedef shared< Gnx > SharedGnx;

class GnxVisitor {
public:
	virtual void startVisit( Gnx & element ) = 0;
	virtual void endVisit( Gnx & element ) = 0;
public:
	virtual ~GnxVisitor();
};

template <class T>
class Iterator {
public:
	bool hasNext();
	T next();
};

typedef class Iterator< SharedGnx > SharedGnxIterator;
typedef class Iterator< std::string > StringIterator;

class GnxChildIterator : public Iterator< SharedGnx & > {
private:
	std::vector< shared< Gnx > >::iterator it;
	std::vector< shared< Gnx > >::iterator end;
public:
	bool hasNext();
	SharedGnx & next();
public:
	GnxChildIterator( SharedGnx gnx );
	GnxChildIterator( Gnx & gnx );
};

typedef std::pair< const std::string, std::string > GnxEntry;

class GnxEntryIterator : public Iterator< GnxEntry & > {
private:
	std::map< std::string, std::string >::iterator it;
	std::map< std::string, std::string >::iterator end;
public:
	bool hasNext();
	GnxEntry & next();
public:
	GnxEntryIterator( SharedGnx gnx );
	GnxEntryIterator( Gnx & gnx );
};

class Gnx {
friend class GnxChildIterator;
friend class GnxEntryIterator;
private: 
	std::string element_name;
	std::vector< shared< Gnx > > children;
	std::map< std::string, std::string > attributes;
	int flags;
	
public:
	const std::string & attribute( const std::string & key ) const;
	const std::string & attribute( const std::string & key, const std::string & def ) const;
	bool hasAttribute( const std::string & key ) const;
	bool hasAttribute( const std::string & key, const std::string & eqval ) const;
	shared< Gnx > & child( int n );
	shared< Gnx > & firstChild();
	shared< Gnx > & lastChild();
	int size() const;
	bool isEmpty() const;
	std::string & name();
	bool hasName( const std::string & name );
	bool hasAnyFlags( int mask );
	bool hasAllFlags( int mask );
	void clearFlags( int mask );
	void orFlags( int mask );
	void andFlags( int mask );

public:
	void clearAttribute( const std::string & key );
	void clearAllAttributes();
	void putAttribute( const std::string & key, const std::string & value );
	void putAttribute( const std::string & key, const int & value );
	void putAttributeMap( std::map< std::string, std::string > & attrs );
	void addChild( shared< Gnx > child );
	void popFrontChild();
	void popLastChild();
	void flattenChild( int n );
	void copyFrom( const Gnx & g );
	
public:
	void render( std::ostream & out );
	void render();
	void prettyPrint( std::ostream & out );
	void prettyPrint( std::ostream & out, const std::string & indentation );
	void prettyPrint( const std::string & indentation );
	void prettyPrint();
	void visit( GnxVisitor & v );
	
public:
	Gnx( const std::string & name );
};

class GnxBuilder {
private:
	std::vector< shared< Gnx > > stack;
	std::vector< shared< Gnx > > put_aside;
public:
	GnxBuilder();
	void start( const std::string & name );
	void put( const std::string & key, const std::string & value );
	void add( shared< Gnx > & child );
	void end();
	void save();
	void restore();
	shared< Gnx > build();
};

class GnxReader {
private:
	std::istream & in;
	
public:
	shared< Gnx > readGnx();
	
public:
	GnxReader( std::istream & in ) : in( in ) {}
	GnxReader() : in( std::cin ) {}
};

}	//	namespace

#endif
