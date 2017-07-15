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

#ifndef LIBGNG_GSON_HPP
#define LIBGNG_GSON_HPP

#include <string>
#include <map>
#include <vector>
#include <istream>

#include <stdlib.h>

#include "shared.hpp"
#include "mishap.hpp"

namespace Ginger {

/**
 * 	In time I would like to replace UserSettings with something like
 *	GSON. I don't think that GSON is quite right as yet, so
 *	I will settle for something fairly simple.
 */

class GSONData;
class BoolGSONData;
class LongGSONData;
class StringGSONData;
class ListGSONData;
class MapGSONData;
class GSONDataVisitor;
class GSONDataWalker;


typedef shared< GSONData > SharedBabyGSON;
class GSON : public SharedBabyGSON {
public:
	GSON() : SharedBabyGSON() {}
	GSON( GSONData * p ) : SharedBabyGSON( p ) {}

private:
	GSONData * self() { return this->SharedBabyGSON::get(); }
	const GSONData * self() const { return this->SharedBabyGSON::get(); }

public:
	bool isAbsent() const;
	bool isBool() const;
	bool isLong() const;
	bool isString() const;
	bool isList() const;
	bool isMap() const;
	const std::string typeOf() const;

public:
	/**
		Read a GSON term from a file
		@param fname The name of the file with GSON content.
	*/
	static GSON read( const std::string fname );
	
	/**
		Read a GSON term from an input stream.
		@param input The input stream yielding GSON content.
	*/
	static GSON read( std::istream & input );
	
	/**
		Read a GSON term from a FILE *
		@param input The FILE * pointer that yields GSON content.
	*/
	static GSON read( FILE * input, bool close_on_exit = false );

	/**
		Read a setting files in GSON format. This is a sequence of
		map bindings that are not required to be comma-separated.
		@param fname The name of the settings file.
	*/
	static GSON readSettingsFile( const std::string fname  );
	
	/**
		Read a setting files in GSON format. This is a sequence of
		map bindings that are not required to be comma-separated.
		@param input The input stream yielding GSON settings.
	*/
	static GSON readSettingsFile( std::istream & input  );

	/**
		Read a setting files in GSON format. This is a sequence of
		map bindings that are not required to be comma-separated.
		@param input The FILE * pointer yielding GSON settings.
	*/
	static GSON readSettingsFile( FILE * input, bool close_on_exit = false );



public:
	bool lessThan( const GSON other ) const;
	void visit( GSONDataVisitor & visitor );
	void walk( GSONDataWalker & walker );
	void formatUsing( std::ostream & out, const std::string & control_string );
	void render();
	void print( std::ostream & out );

public: //	Bool operations.
	bool getBool() const;

public: //	Long operations.
	long getLong() const;

public:	//	String operations.
	std::string getString() const;

public:	//	List operations.
	GSON at( const int n ) const;
	size_t size() const;
	bool isEmpty() const;
	bool isntEmpty() const;
	GSON first() const;
	GSON last() const;

public: //	Map operations.
	GSON index( const std::string & s ) const;
	GSON index( const char * s ) const { return this->index( std::string( s ) ); }

};

class GSONData {
public:
	//virtual bool isLong() const { return false; }
	//virtual bool isString() const { return false; }
	//virtual bool isList() const { return false; }
	//virtual bool isMap() const { return false; }
	virtual void render();
	virtual void visit( GSONDataVisitor & visitor ) = 0;
	virtual void walk( GSONDataWalker & walker ) = 0;
	//virtual const std::string typeOf() const = 0;
	virtual bool lessThan( const GSONData & other ) const = 0;
	GSON at( const int n ) const;
	size_t size() const;

public:
	GSONData() {}
	virtual ~GSONData() {}

	static GSON read( const std::string fname );
};

class AbsentGSONData : public GSONData {
public:
	virtual ~AbsentGSONData() {}
public:
	virtual void visit( GSONDataVisitor & visitor );
	virtual void walk( GSONDataWalker & walker );
	virtual bool lessThan( const GSONData & other ) const;
};

class BoolGSONData : public GSONData {
private:
	bool val;
public:
	BoolGSONData( const bool b ) : val( b ) {}
	virtual ~BoolGSONData() {}
public:
	bool getBool() const { return this->val; }
	virtual void visit( GSONDataVisitor & visitor );
	virtual void walk( GSONDataWalker & walker );
	virtual bool lessThan( const GSONData & other ) const;
};

class LongGSONData : public GSONData {
private:
	long val;
public:
	LongGSONData( const long _n ) : val( _n ) {}
	virtual ~LongGSONData() {}
public:
	long getLong() const { return this->val; }
	//bool isLong() const { return true; }
	//const std::string typeOf() const { return "long"; }
	virtual void visit( GSONDataVisitor & visitor );
	virtual void walk( GSONDataWalker & walker );
	virtual bool lessThan( const GSONData & other ) const;
};

class StringGSONData : public GSONData {
private:
	std::string val;
public:
	StringGSONData( const char * _t ) : val( _t ) {}
	StringGSONData( const std::string & _t ) : val( _t ) {}
	virtual ~StringGSONData() {}
public:
	const std::string getString() const { return this->val; }
	//bool isString() const { return true; }
	//const std::string typeOf() const { return "string"; }
	virtual void visit( GSONDataVisitor & visitor );
	virtual void walk( GSONDataWalker & walker );
	virtual bool lessThan( const GSONData & other ) const;
};

class ListGSONData : public GSONData {
public:
	typedef std::vector< GSON > BabyList;
	typedef BabyList::iterator iterator;
private:
	BabyList val;
public:
	ListGSONData() {}
	virtual ~ListGSONData() {}
public:
	iterator begin() { return val.begin(); }
	iterator end() { return val.end(); }
	void add( GSON b ) { this->val.push_back( b ); }
	bool isEmpty() const { return this->val.empty(); }
	bool isntEmpty() const { return not this->val.empty(); }
	size_t size() const { return this->val.size(); }
	//bool isList() const { return true; }
	//const std::string typeOf() const { return "list"; }
	virtual void visit( GSONDataVisitor & visitor );
	virtual void walk( GSONDataWalker & walk );
	virtual bool lessThan( const GSONData & other ) const;
	GSON at( const int n ) const { return this->val[ n ]; }
	GSON first() const { return this->val.front(); }
	GSON last() const { return this->val.back(); }
};


class MapGSONData : public GSONData {
	struct classcomp {
	  	bool operator() ( const GSON & lhs, const GSON & rhs) const {
	  		return lhs->lessThan( *rhs );
	  	}
	};
	typedef std::map< GSON, GSON, classcomp > Map;
private:
	Map val;
public:
	MapGSONData() {}
	virtual ~MapGSONData() {}
public:
	//bool isMap() const { return true; }
	//const std::string typeOf() const { return "map"; }
	virtual void visit( GSONDataVisitor & visitor );
	virtual void walk( GSONDataWalker & walker );
	virtual bool lessThan( const GSONData & other ) const;
	GSON index( GSON p ) const;
	void put( GSON p, GSON q ) { this->val[ p ] = q; }
};

class GSONDataVisitor {
public:
	virtual void visitAbsent( AbsentGSONData & s ) = 0;
	virtual void visitBool( BoolGSONData & s ) = 0;
	virtual void visitLong( LongGSONData & s ) = 0;
	virtual void visitString( StringGSONData & s ) = 0;
	virtual void visitList( ListGSONData & s ) = 0;
	virtual void visitMap( MapGSONData & s ) = 0;
	virtual ~GSONDataVisitor() {}
};

class GSONDataWalker {
public:
	virtual ~GSONDataWalker() {}
public:
	virtual void absentValue() = 0;
	virtual void boolValue( const bool n ) = 0;
	virtual void longValue( const long n ) = 0;
	virtual void stringValue( const std::string & s ) = 0;
public:
	virtual void beginList( ListGSONData & s ) = 0;
	virtual void beforeMemberList( ListGSONData & s, const int index ) = 0;
	virtual void afterMemberList( ListGSONData & s, const int index ) = 0;
	virtual void endList( ListGSONData & s ) = 0;
public:
	virtual void beginMap( MapGSONData & s ) = 0;
	virtual void beginKeyMap( MapGSONData & s, const int index ) = 0;
	virtual void endKeyMap( MapGSONData & s, const int index ) = 0;
	virtual void beginValueMap( MapGSONData & s, const int index ) = 0;
	virtual void endValueMap( MapGSONData & s, const int index ) = 0;
	virtual void endMap( MapGSONData & s ) = 0;
};

class GSONDriver {
public:
	virtual ~GSONDriver() {}
public:
	virtual void absentValue() = 0;
	virtual void boolValue( const bool n ) = 0;
	virtual void longValue( const long n ) = 0;
	virtual void stringValue( const std::string & s ) = 0;
public:
	virtual void beginList() = 0;
	virtual void beginMemberList() = 0;
	virtual void endMemberList() = 0;
	virtual void endList() = 0;
public:
	virtual void beginMap() = 0;
	virtual void beginMaplet() = 0;
	virtual void beginKeyMaplet() = 0;
	virtual void endKeyMaplet() = 0;
	virtual void beginValueMaplet() = 0;
	virtual void endValueMaplet() = 0;
	virtual void endMaplet() = 0;
	virtual void endMap() = 0;

};

class GSONBuilder : public GSONDriver {
private:
    std::vector< GSON > stack;
    std::vector< size_t > dump;
public:
    virtual ~GSONBuilder() {}
public:
    void push( GSONData * p ) {
        this->stack.push_back( GSON( p ) );
    }
public:
    virtual void absentValue() {
        this->push( new AbsentGSONData() );
    }
    virtual void boolValue( const bool n ) {
        //  cerr << "push bool: " << n << endl;
        this->push( new BoolGSONData( n ) );
    }
    virtual void longValue( const long n ) {
        //  cerr << "push long: " << n << endl;
        this->push( new LongGSONData( n ) );
    }
    virtual void stringValue( const std::string & s ) {
        //  cerr << "push string: " << s << endl;
        this->push( new StringGSONData( s ) );
    }
public:
    virtual void beginList() {
        //  cerr << "start list: " << endl;
        this->dump.push_back( this->stack.size() );
    }
    virtual void beginMemberList() {
        //  Skip.
    }
    virtual void endMemberList() {
        //  Skip.
    }
    virtual void endList() {
        //  cerr << "end list: " << endl;
        ListGSONData * p = new ListGSONData();
        GSON answer( p );
        for ( 
            std::vector< GSON >::iterator it = this->stack.begin() + this->dump.back();
            it != this->stack.end();
            ++it
        ) {
            p->add( *it );
        }
        this->stack.resize( this->dump.back() );
        this->dump.pop_back();
        this->stack.push_back( answer );            
    }
public:
    virtual void beginMap() {
        //  cerr << "start map: " << this->stack.size() << endl;
        this->dump.push_back( this->stack.size() );
    }
    virtual void beginMaplet() {
        //  Skip.
    }
    virtual void beginKeyMaplet() {
        //  Skip.
    }
    virtual void endKeyMaplet() {
        //  Skip.
    }
    virtual void beginValueMaplet() {
        //  Skip.
    }
    virtual void endValueMaplet() {
        //  Skip.
    }
    virtual void endMaplet() {
        //  Skip.
    }
    virtual void endMap() {
        //cerr << "end map: " << this->stack.size() << endl;
        MapGSONData * p = new MapGSONData();
        GSON answer( p );
        for ( 
            std::vector< GSON >::iterator it = this->stack.begin() + this->dump.back();
            it != this->stack.end();
        ) {
            //cerr << "popping" << endl;
            GSON lhs = *it++;
            GSON rhs = *it++;
            p->put( lhs, rhs );
        }
        this->stack.resize( this->dump.back() );
        this->dump.pop_back();
        this->stack.push_back( answer );            
    }

public:
    GSON newGSON() {
        if ( this->stack.size() == 1 ) {
            return this->stack[ 0 ];
        } else {
            throw Ginger::Mishap( "Error while building GSON value" );
        }
    }
};

} // namespace

#endif
