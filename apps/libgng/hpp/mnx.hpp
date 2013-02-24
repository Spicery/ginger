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

#ifndef MNX_LIBGNG_HPP
#define MNX_LIBGNG_HPP

#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <map>

#include "shared.hpp"


namespace Ginger {

extern void mnxFRenderText( FILE * f, const std::string & string );
extern void mnxRenderText( std::ostream & out, const std::string & string );
extern void mnxRenderText( const std::string & string );

class Mnx;
typedef shared< Mnx > SharedMnx;

class MnxVisitor {
public:
	virtual void startVisit( Mnx & element ) = 0;
	virtual void endVisit( Mnx & element ) = 0;
public:
	virtual ~MnxVisitor();
};

class MnxWalkPath {
private:
	Mnx &			mnx;
	int				index;
	MnxWalkPath *	prev;

public:
	Mnx & 			getMnx() const { return this->mnx; }
	int				getIndex() const { return this->index; }
	MnxWalkPath *	getPrevious() const { return this->prev; }
	bool			isLastIndex() const;

public:
	MnxWalkPath( 
		Mnx &			mnx,
		int				index,
		MnxWalkPath * 	prev
	) : 
		mnx( mnx ),
		index( index ),
		prev( prev )
	{}
};

class MnxWalker {
public:
	virtual void startWalk( Mnx & element, MnxWalkPath * p ) = 0;
	virtual void endWalk( Mnx & element, MnxWalkPath * p ) = 0;

public:
	virtual ~MnxWalker();
};

template <class T>
class Iterator {
public:
	bool hasNext();
	T next();
};

typedef class Iterator< SharedMnx > SharedMnxIterator;
typedef class Iterator< std::string > StringIterator;

class MnxChildIterator : public Iterator< SharedMnx & > {
private:
	std::vector< shared< Mnx > >::iterator it;
	std::vector< shared< Mnx > >::iterator end;
public:
	bool hasNext();
	SharedMnx & next();
public:
	MnxChildIterator( SharedMnx mnx );
	MnxChildIterator( Mnx & mnx );
};

typedef std::pair< const std::string, std::string > MnxEntry;

class MnxEntryIterator : public Iterator< MnxEntry & > {
private:
	std::map< std::string, std::string >::iterator it;
	std::map< std::string, std::string >::iterator end;
public:
	bool hasNext();
	MnxEntry & next();
public:
	MnxEntryIterator( SharedMnx mnx );
	MnxEntryIterator( Mnx & mnx );
};

class Mnx {
friend class MnxChildIterator;
friend class MnxEntryIterator;
private: 
	std::string element_name;
	std::vector< shared< Mnx > > children;
	std::map< std::string, std::string > attributes;
	int flags;

private:
	long attributeToLongHelper( const std::string & key, const long def, const bool use_def ) const;
	int attributeToIntHelper( const std::string & key, const int def, const bool use_def ) const;

public:
	const std::string & attribute( const std::string & key ) const;
	const std::string & attribute( const std::string & key, const std::string & def ) const;
	int attributeToInt( const std::string & key ) const;
	int attributeToInt( const std::string & key, const int def ) const;
	long attributeToLong( const std::string & key ) const;
	long attributeToLong( const std::string & key, const long def ) const;
	bool hasAttribute( const std::string & key ) const;
	bool hasAttribute( const std::string & key, const std::string & eqval ) const;
	shared< Mnx > & child( int n );
	shared< Mnx > & firstChild();
	shared< Mnx > & lastChild();
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
	void putAttribute( const std::string & key, const long & value );
	void putAttributeMap( std::map< std::string, std::string > & attrs );
	bool removeAttribute( const std::string & key );
	void addChild( shared< Mnx > child );
	void popFrontChild();
	void popLastChild();
	void flattenChild( int n );
	void copyFrom( const Mnx & g );
	
public:
	std::string toString();
	void frender( FILE * f );
	void render( std::ostream & out );
	void render();
	void prettyPrint( std::ostream & out );
	void prettyPrint( std::ostream & out, const std::string & indentation );
	void prettyPrint( const std::string & indentation );
	void prettyPrint();
	void visit( MnxVisitor & v );
	void walk( MnxWalker & w, MnxWalkPath * p = 0 );
	
public:
	Mnx( const std::string & name );
};

class MnxBuilder {
private:
	std::vector< shared< Mnx > > stack;
	std::vector< shared< Mnx > > put_aside;
public:
	MnxBuilder();
	void start( const std::string & name );
	void put( const std::string & key, const std::string & value );
	void put( const std::string & key, const long & value );
	bool remove( const std::string & key );
	void add( shared< Mnx > & child );
	void end();
	void save();
	void restore();
	shared< Mnx > build();
};

class MnxReader {
private:
	std::istream & in;
	
public:
	shared< Mnx > readMnx();
	
public:
	MnxReader( std::istream & in ) : in( in ) {}
	MnxReader() : in( std::cin ) {}
};

}	//	namespace

#endif
