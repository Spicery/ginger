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
#include <cctype>
#include <string>
#include <map>
#include <sstream>

using namespace std;

#include "read_xml.hpp"

#include "sys.hpp"
#include "mishap.hpp"
#include "facet.hpp"

//- SAX Parser -----------------------------------------------------------------


char SaxClass::nextChar() {
	char c;
	if ( not input.get( c ) ) throw;
	return c;
}

char SaxClass::peekChar() {
	char c = input.peek();
	if ( not input ) throw;
	return c;
}

void SaxClass::mustReadChar( const char ch_want ) {
	char c_got;
	if ( not ( input.get( c_got ) ) ) throw;
	if ( c_got != ch_want ) throw;
}

void SaxClass::eatWhiteSpace() {
	char ch;
	while ( input.get( ch ) && isspace( ch ) ) {
		//	Skip.
	}
	//cout << "Ungetting '" << ch << "'" << endl;
	input.putback( ch );
}

static bool is_name_char( const char ch ) {
	return isalnum( ch ) || ch == '-';
}

void SaxClass::readName( std::string & name ) {
	char ch;
	while ( (ch = nextChar()) && is_name_char( ch ) ) {
		//std::cout << "pushing '" << ch << "'"<< endl;
		name.push_back( ch );
		//std::cout << "name so far '" << name << "'" << endl;
	}
	input.putback( ch );
}

void SaxClass::readAttributeValue( std::string & attr ) {
	this->mustReadChar( '\"' );
	for (;;) {
		char ch = this->nextChar();
		if ( ch == '\"' ) break;
		if ( ch == '&' ) {
			std::string esc;
			while ( ch = this->nextChar(), ch != ';' ) {
				//std::cout << "char " << ch << endl;
				esc.push_back( ch );
				if ( esc.size() > 3 ) {
					throw;
				}
			}
			if ( esc == "lt" ) {
				attr.push_back( '<' );
			} else if ( esc == "gt" ) {
				attr.push_back( '>' );
			} else if ( esc == "amp" ) {
				attr.push_back( '&' );
			} else {
				throw;
			}
		} else {
			attr.push_back( ch );
		}
	}
}	


void SaxClass::processAttributes(
	std::map< std::string, std::string > & attrs
) {
	//	Process attributes
	for (;;) {
		this->eatWhiteSpace();
		char c = peekChar();
		if ( c == '/' || c == '>' ) {
			break;
		}
		std::string key;
		this->readName( key );
		this->eatWhiteSpace();
		this->mustReadChar( '=' );
		this->eatWhiteSpace();
		std::string value;
		this->readAttributeValue( value );
		attrs[ key ] = value;
	}
}


void SaxClass::read() {
	this->input >> noskipws;
	
	this->eatWhiteSpace();
	if ( this->input.eof() ) return;
	this->mustReadChar( '<' );
		
	char ch = nextChar();
	if ( ch == '/' ) {
		std::string end_tag;
		this->readName( end_tag );
		this->eatWhiteSpace();
		this->mustReadChar( '>' );
		this->parent.endTag( end_tag );
		return;
	} else if ( ch == '!' || ch == '?' ) {
		for (;;) {
			ch = nextChar();
			if ( ch == '>' ) break;
		}
		this->read();
		return;
	} else {
		//cout << "Ungetting '" << ch << "'" << endl;
		input.putback( ch );
	}
	
	std::string name;
	this->readName( name );
	
	std::map< std::string, std::string > attributes;
	this->processAttributes( attributes );
	
	this->eatWhiteSpace();
	
	
	ch = nextChar();
	if ( ch == '/' ) {
		this->mustReadChar( '>' );
		this->parent.startTag( name, attributes );
		this->parent.endTag( name );
		return;
	} else if ( ch == '>' ) {
		this->parent.startTag( name, attributes );
		return;
	} else {
		throw;
	}
			
}


//- Read XML -------------------------------------------------------------------

void ReadXmlClass::startTag( std::string & name, std::map< std::string, std::string > & attrs ) {
	this->tag_stack.push_back( TermData() );
	TermData & t = this->tag_stack.back();
	t.name = name;
	t.attrs = attrs;
}

void ReadXmlClass::endTag( std::string & name ) {
	if ( this->tag_stack.size() < 2 ) throw;
	
	TermData & t = this->tag_stack.back();
	Term term = t.makeTerm();
	this->tag_stack.pop_back();
	
	TermData & u = this->tag_stack.back();
	u.kids.push_back( term );
}

Term ReadXmlClass::readElement() {
	this->tag_stack.push_back( TermData() );		//	Push dummy.
	for (;;) {
		this->sax.read();
		if ( this->tag_stack.size() == 1 ) break;
	}
	
	std::vector< Term > & v = this->tag_stack.front().kids;
	if ( v.empty() ) {
		return Term();
	} else {
		return v[ 0 ];
	}
}


//- TermData -------------------------------------------------------------------

static bool has_attr( TermData * t, const char * v ) {
	return t->attrs.find( v ) != t->attrs.end();
}

static bool startsWith( const std::string & a, const std::string & b ) {
	return !a.compare( 0, b.length(), b );
}

static const FacetSet * makeFacetSet( TermData * t, const std::string & prefix ) {
	set< string > tags;
	for ( 
		map<string,string>::iterator it = t->attrs.begin();
		it != t->attrs.end();
		++it 
	) {
		if ( startsWith( it->first, prefix ) ) {
			tags.insert( it->second );
		}
	}
	return fetchFacetSet( tags );
}

static Term makeIf( const int i, const int n, const std::vector< Term > & kids ) {
	int d = n - i;
	if ( d == 1 ) {
		return kids[ i ];
	} else if ( d == 2 ) {
		return term_new_basic3( fnc_if, kids[ i ], kids[ i + 1 ], term_skip );
	} else {
		return term_new_basic3( fnc_if, kids[ i ], kids[ i + 1 ], makeIf( i + 2, n, kids ) );
	}
}

static Arity kids_analysis( vector< Term > & kids ) {
	Arity sofar( 0 );
	for ( vector< Term >::iterator it = kids.begin(); it != kids.end(); it++ ) {
		Arity ta( *it );
		sofar = sofar.join( ta );
	}
	return sofar;
}

static Term makeSysApp( string & name, vector< Term > & kids ) {
	SysMap::iterator smit = sysMap.find( name );
	if ( smit == sysMap.end() ) {
		throw Mishap( "No such system call" ).culprit( "Name", name );
	}

	SysInfo & info = smit->second;
	Arity ka = kids_analysis( kids );
	info.in_arity.check( ka ); 

	Term t;
	if ( info.functor == fnc_syscall ) {
		t = term_new_ref( info.functor, ToRef( info.syscall ) );
	} else {
		t = term_new_basic0( info.functor );
	}	
	
	for ( vector< Term >::iterator it = kids.begin(); it != kids.end(); it++ ) {
		term_add( t, *it );
	}
	
	return t;
}

static void packageContext( TermData * t, string & pkg_name, string & alias_name, enum NamedRefType & r  ) {
	if ( has_attr( t, "pkg" ) ) {
		r = ABSOLUTE_REF_TYPE;
	} else if ( has_attr( t, "alias" ) ){
		r = ALIAS_REF_TYPE;
	} else {
		r = LOCAL_REF_TYPE;
	}
	
	//	Note how these assignments come last as they are side
	//	effecting on the attrs map.
	pkg_name = t->attrs[ "pkg" ];
	alias_name = t->attrs[ "alias" ];
}


Term TermData::makeTerm() {
	if ( name == "fn" && kids.size() == 2 ) {
		//	NAME is discarded for the moment, until we know how to use it.
		return term_new_fn( kids[ 0 ], kids[ 1 ] );
	} else if ( has_attr( this, "value" ) ) {
		if ( name == "int" ) {
			int i;
			istringstream s( this->attrs[ "value" ] );
			if ( not ( s >> i ) ) throw;
			return term_new_int( i );
		} else if ( name == "bool" ) {
			if ( this->attrs[ "value" ] == "true" ) {
				return term_new_bool( true );
			} else if ( this->attrs[ "value" ] == "false" ) {
				return term_new_bool( false );
			} else {
				throw;
			}
		} else if ( name == "char" ) {
			std::string t = this->attrs[ "value" ];
			if ( t.size() <= 0 ) throw;
			return term_new_char( t[ 0 ] );
		} else if ( name == "string" ) {
			return term_new_string( this->attrs[ "value" ] );
		} else if ( name == "absent" ) {
			return term_new_absent();
		} else if ( name == "list" ) {
			if ( this->attrs[ "value" ] == "empty" ) {
				return term_new_list_empty();
			} else {
				throw Mishap( "Invalid Ginger XML" );
			}
		} else if ( name == "sysfn" ) {
			return term_new_sysfn( this->attrs[ "value" ] );
		} else {
			throw Unreachable();
		}
	} else if ( has_attr( this, "name" ) ) {
		if ( name == "id" ) {
			enum NamedRefType r;
			std::string pkg_name, alias_name;
			packageContext( this, pkg_name, alias_name, r );
			return shared< TermClass >( new IdTermClass( r, pkg_name, alias_name, attrs[ "name" ] ) );
		} else if ( name == "var" ) {
			enum NamedRefType r;
			std::string pkg_name, alias_name;
			packageContext( this, pkg_name, alias_name, r );
			const FacetSet * facets = makeFacetSet( this, "tag" );
			return shared< TermClass >( new VarTermClass( r, pkg_name, alias_name, facets, attrs[ "name" ] ) );
		} else if ( name == "sysapp" ) {
			return makeSysApp( attrs[ "name" ], kids );
		} else {
			throw;
		}
	} else if ( name == "bind" && kids.size() == 2 ) {
		return term_new_basic2( fnc_dec, kids[ 0 ], kids[ 1 ] );
	} else if ( name == "seq" ) {
		Term seq = term_new_basic0( fnc_seq );
		for ( std::vector< Term >::iterator it = kids.begin(); it != kids.end(); ++it ) {
			term_add( seq, *it );
		}
		return seq;
	} else if ( name == "if" ) {
		int n = kids.size();
		
		if ( n == 0 ) { 					// 	unusual but defined.
			return term_new_basic0( fnc_seq );
		} else if ( n == 1 ) {				//	unusual but defined.
			return kids[ 0 ];
		} else {
			return makeIf( 0, n, kids );
		}
	} else if ( name == "for" && kids.size() == 2 ) {
		return term_new_basic2( fnc_for, kids[0], kids[1] );
	} else if ( name == "from" && kids.size() == 3 ) {
		return term_new_from( kids[0], kids[1], kids[2] );
	} else if ( name == "in" && kids.size() == 2 ) {
		return term_new_in( kids[ 0 ], kids[ 1 ] );
	} else if ( name == "app" && kids.size() == 2 ) {
		return term_new_basic2( fnc_app, kids[ 0 ], kids[ 1 ] );
	} else if ( name == "package" ) {
		Term t = term_new_package( attrs[ "url" ] );
		const int n = kids.size();
		for ( int i = 0; i < n; i++ ) {
			term_add( t, kids[ i ] );
		}
		return t;
	} else if ( name == "import" ) {
		if ( !has_attr( this, "from" ) ) throw Mishap( "Malformed import" );
		const FacetSet * match_tags = makeFacetSet( this, "match" );
		string from = attrs[ "from" ];
		string alias = has_attr( this, "alias" ) ? attrs[ "alias" ] : from;
		bool prot = has_attr( this, "protected" ) && ( attrs[ "protected" ] == string( "true" ) );
		//const Facet * into = has_attr( this, "into" ) ? fetchFacet( attrs[ "into" ] ) : NULL;
		const FacetSet * intos = makeFacetSet( this, "into" );
		//intos->debug();
		return shared< TermClass >( new ImportTermClass( match_tags, from, alias, prot, /*into,*/ intos ) ); 
	} else {
		cerr << "name = " << name << endl;
		cerr << "#kids = " << kids.size() << endl;
		throw;
	}
}


