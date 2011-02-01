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

#include <ostream>
#include <string>
#include <vector>
#include <map>

#include "shared.hpp"


namespace Ginger {

extern void gnxRenderText( std::ostream & out, const std::string & string );
extern void gnxRenderText( const std::string & string );

class Gnx;

class GnxVisitor {
public:
	virtual void startVisit( Gnx & element ) = 0;
	virtual void endVisit( Gnx & element ) = 0;
public:
	virtual ~GnxVisitor();
};

class Gnx {
private: 
	std::string element_name;
	std::vector< shared< Gnx > > children;
	std::map< std::string, std::string > attributes;
	
public:
	const std::string & attribute( const std::string & key ) const;
	bool hasAttribute( const std::string & key ) const;
	shared< Gnx > child( int n ) const;
	int size() const;
	std::string & name();

public:
	void clearAttributes();
	void putAttribute( const std::string & key, const std::string & value );
	void putAttributeMap( std::map< std::string, std::string > & attrs );
	void addChild( shared< Gnx > child );
	void popFrontChild();
	void flattenChild( int n );
	
public:
	void render( std::ostream & out );
	void render();
	void visit( GnxVisitor & v );
	
public:
	Gnx( const std::string & name );
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
