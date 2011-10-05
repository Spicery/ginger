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

#include "mnx.hpp"


#include "read_xml.hpp"
#include "sys.hpp"
#include "mishap.hpp"
#include "facet.hpp"

using namespace std;


//- Read XML -------------------------------------------------------------------

void ReadXmlClass::startTag( std::string & name, std::map< std::string, std::string > & attrs ) {
	this->tag_stack.push_back( TermData() );
	TermData & t = this->tag_stack.back();
	t.name = name;
	t.attrs = attrs;
}

void ReadXmlClass::endTag( std::string & name ) {
	if ( this->tag_stack.size() < 2 ) throw Ginger::Mishap( "Too many end tags" );
	
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

static Term makeSysApp( const string & name, vector< Term > & kids ) {
	SysMap::iterator smit = sysMap.find( name );
	if ( smit == sysMap.end() ) {
		throw Ginger::Mishap( "No such system call" ).culprit( "Name", name );
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

static void packageContext( TermData * t, string & enc_pkg_name, string & def_pkg_name, string & alias_name, enum NamedRefType & r  ) {
	if ( has_attr( t, "def.pkg" ) ) {
		r = ABSOLUTE_REF_TYPE;
	} else if ( has_attr( t, "alias" ) ){
		r = ALIAS_REF_TYPE;
	} else {
		r = UNQUALIFIED_REF_TYPE;
	}
	
	//	Note how these assignments come last as they are side
	//	effecting on the attrs map.
	def_pkg_name = t->attrs[ "def.pkg" ];
	enc_pkg_name = t->attrs[ "enc.pkg" ];
	alias_name = t->attrs[ "alias" ];
}

bool TermData::hasConstantType( const char * type ) {
	return (
		this->name == "constant" && has_attr( this, "type" ) && this->attrs[ "type" ] == type ||
		this->name == type 
	);
}


Term TermData::makeTerm() {
	if ( name == "fn" && kids.size() == 2 ) {
		return term_new_fn( this->attrs[ "name" ], kids[ 0 ], kids[ 1 ] );
	} else if ( has_attr( this, "value" ) ) {
		if ( this->hasConstantType( "int" ) ) {
			int i;
			istringstream s( this->attrs[ "value" ] );
			if ( not ( s >> i ) ) throw;
			return term_new_int( i );
		} else if ( this->hasConstantType( "bool" ) ) {
			if ( this->attrs[ "value" ] == "true" ) {
				return term_new_bool( true );
			} else if ( this->attrs[ "value" ] == "false" ) {
				return term_new_bool( false );
			} else {
				throw;
			}
		} else if ( this->hasConstantType( "char" ) ) {
			std::string t = this->attrs[ "value" ];
			if ( t.size() <= 0 ) throw;
			return term_new_char( t[ 0 ] );
		} else if ( this->hasConstantType( "string" ) ) {
			return term_new_string( this->attrs[ "value" ] );
		} else if ( this->hasConstantType( "symbol" ) ) {
			return term_new_symbol( this->attrs[ "value" ] );
		} else if ( this->hasConstantType( "absent" ) ) {
			return term_new_absent();
		} else if ( this->hasConstantType( "list" ) ) {
			if ( this->attrs[ "value" ] == "empty" ) {
				return term_new_list_empty();
			} else {
				throw Ginger::Mishap( "Invalid Ginger XML" );
			}
		} else if ( this->hasConstantType( "sysfn" ) ) {
			return term_new_sysfn( this->attrs[ "value" ] );
		} else {
			throw Ginger::Unreachable( __FILE__, __LINE__ );
		}
	} else if ( has_attr( this, "name" ) ) {
		if ( name == "id" ) {
			enum NamedRefType r;
			std::string enc_pkg_name, def_pkg_name, alias_name;
			packageContext( this, enc_pkg_name, def_pkg_name, alias_name, r );
			return shared< TermClass >( new IdTermClass( r, enc_pkg_name, def_pkg_name, alias_name, attrs[ "name" ] ) );
		} else if ( name == "var" ) {
			enum NamedRefType r;
			std::string enc_pkg_name, def_pkg_name, alias_name;
			packageContext( this, enc_pkg_name, def_pkg_name, alias_name, r );
			//const FacetSet * facets = makeFacetSet( this, "tag" );
			return shared< TermClass >( new VarTermClass( r, enc_pkg_name, def_pkg_name, alias_name, /*facets,*/ attrs[ "name" ] ) );
		} else if ( name == "sysapp" ) {
			return makeSysApp( attrs[ "name" ], kids );
		} else {
			throw Ginger::Mishap( "Unknown functor" ).culprit( "Functor", name );
		}
	} else if ( name == "bind" ) {
		if ( kids.size() == 2 ) {
			return term_new_basic2( fnc_bind, kids[ 0 ], kids[ 1 ] );
		} else {
			throw Ginger::Mishap( "Malformed bind" );
		}
	} else if ( name == "set" ) {
		if ( kids.size() == 2 ) {
			return term_new_basic2( fnc_assign, kids[ 0 ], kids[ 1 ] );
		} else {
			throw Ginger::Mishap( "Malformed assignment" );
		}
	} else if ( name == "seq" ) {
		Term seq = term_new_basic0( fnc_seq );
		for ( std::vector< Term >::iterator it = kids.begin(); it != kids.end(); ++it ) {
			term_add( seq, *it );
		}
		return seq;
	} else if ( name == "block" ) {
		Term seq = term_new_basic0( fnc_block );
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
		throw Ginger::SystemError( "No longer handles import directly" );
	} else {
		cerr << "name = " << name << endl;
		cerr << "#kids = " << kids.size() << endl;
		throw Ginger::SystemError( "Unrecognised term" );
	}
}


// ---------------

static void decodePackageContext( shared< Ginger::Mnx > t, string & enc_pkg_name, string & def_pkg_name, string & alias_name, enum NamedRefType & r  ) {
	if ( t->hasAttribute( "def.pkg" ) ) {
		r = ABSOLUTE_REF_TYPE;
	} else if ( t->hasAttribute( "alias" ) ){
		r = ALIAS_REF_TYPE;
	} else {
		r = UNQUALIFIED_REF_TYPE;
	}
	
	//	Note how these assignments come last as they are side
	//	effecting on the attrs map.
	def_pkg_name = t->attribute( "def.pkg", "" );
	enc_pkg_name = t->attribute( "enc.pkg", "" );
	alias_name = t->attribute( "alias", "" );
}



static void fillKids( shared< Ginger::Mnx > mnx, vector< Term > & kids) {
	Ginger::MnxChildIterator it( mnx );
	while ( it.hasNext() ) {
		kids.push_back( mnxToTerm( it.next() ) );
	}
}

static Term mnxChildToTerm( shared< Ginger::Mnx > mnx, int n  ) {
	return mnxToTerm( mnx->child( n ) );
}

Term mnxToTerm( shared< Ginger::Mnx > mnx ) {
	const string name( mnx->name() );
	const int nkids = mnx->size();
	if ( name == "fn" && nkids == 2 ) {
		return term_new_fn( mnx->attribute( "name", "" ), mnxChildToTerm( mnx, 0 ), mnxChildToTerm( mnx, 1 ) );
	} else if ( mnx->hasAttribute( "value" ) && mnx->hasAttribute( "type" ) ) {
		const string & type = mnx->attribute( "type" );
		const string & value = mnx->attribute( "value" );
		if ( type == "int" ) {
			int i;
			istringstream s( value );
			if ( not ( s >> i ) ) throw Ginger::Mishap( "Not an integer value" ).culprit( "Value", value );
			return term_new_int( i );
		} else if ( type == "bool" ) {
			if ( value == "true" ) {
				return term_new_bool( true );
			} else if ( value == "false" ) {
				return term_new_bool( false );
			} else {
				throw Ginger::Mishap( "Invalid boolean value" ).culprit( "Value", value );
			}
		} else if ( type == "char" ) {
			if ( value.size() <= 0 ) throw Ginger::Mishap( "Zero characters" );
			return term_new_char( value[ 0 ] );
		} else if ( type == "string" ) {
			return term_new_string( value );
		} else if ( type == "symbol" ) {
			return term_new_symbol( value );
		} else if ( type == "absent" ) {
			return term_new_absent();
		} else if ( type == "list" ) {
			if ( value == "empty" ) {
				return term_new_list_empty();
			} else {
				throw Ginger::Mishap( "Invalid Ginger XML" );
			}
		} else if ( type == "sysfn" ) {
			return term_new_sysfn( value );
		} else {
			throw Ginger::Unreachable( __FILE__, __LINE__ );
		}
	} else if ( mnx->hasAttribute( "name" ) ) {
		const string & atname = mnx->attribute( "name" );
		if ( name == "id" ) {
			enum NamedRefType r;
			std::string enc_pkg_name, def_pkg_name, alias_name;
			decodePackageContext( mnx, enc_pkg_name, def_pkg_name, alias_name, r );
			return shared< TermClass >( new IdTermClass( r, enc_pkg_name, def_pkg_name, alias_name, atname ) );
		} else if ( name == "var" ) {
			enum NamedRefType r;
			std::string enc_pkg_name, def_pkg_name, alias_name;
			decodePackageContext( mnx, enc_pkg_name, def_pkg_name, alias_name, r );
			//const FacetSet * facets = makeFacetSet( mnx, "tag" );
			return shared< TermClass >( new VarTermClass( r, enc_pkg_name, def_pkg_name, alias_name, /*facets,*/ atname ) );
		} else if ( name == "sysapp" ) {
			vector< Term > kids;
			fillKids( mnx, kids );
			return makeSysApp( atname, kids );
		} else {
			throw Ginger::Mishap( "Unknown functor" ).culprit( "Functor", name );
		}
	} else if ( name == "bind" ) {
		if ( nkids == 2 ) {
			return term_new_basic2( fnc_bind, mnxChildToTerm( mnx, 0 ), mnxChildToTerm( mnx, 1 ) );
		} else {
			throw Ginger::Mishap( "Malformed bind" );
		}
	} else if ( name == "set" ) {
		if ( nkids == 2 ) {
			return term_new_basic2( fnc_assign, mnxChildToTerm( mnx, 0 ), mnxChildToTerm( mnx, 1 ) );
		} else {
			throw Ginger::Mishap( "Malformed assignment" );
		}
	} else if ( name == "seq" ) {
		Term seq = term_new_basic0( fnc_seq );
		Ginger::MnxChildIterator it( mnx );
		while ( it.hasNext() ) {
			term_add( seq, mnxToTerm( it.next() ) );
		}
		return seq;
	} else if ( name == "block" ) {
		Term seq = term_new_basic0( fnc_block );
		Ginger::MnxChildIterator it( mnx );
		while ( it.hasNext() ) {
			term_add( seq, mnxToTerm( it.next() ) );
		}
		return seq;
	} else if ( name == "if" ) {
		if ( nkids == 0 ) { 					// 	unusual but defined.
			return term_new_basic0( fnc_seq );
		} else if ( nkids == 1 ) {				//	unusual but defined.
			return mnxChildToTerm( mnx, 0 );
		} else {
			vector< Term > kids;
			fillKids( mnx, kids );			
			return makeIf( 0, nkids, kids );
		}
	} else if ( name == "for" && nkids == 2 ) {
		return term_new_basic2( fnc_for, mnxToTerm( mnx->child( 0 ) ), mnxToTerm( mnx->child( 1 ) ) );
	} else if ( name == "from" && nkids == 3 ) {
		return term_new_from( mnxChildToTerm( mnx, 0 ), mnxChildToTerm( mnx, 1 ), mnxChildToTerm( mnx, 2 )  );
	} else if ( name == "in" && nkids == 2 ) {
		return term_new_in( mnxToTerm( mnx->child( 0 ) ), mnxToTerm( mnx->child( 1 ) ) );
	} else if ( name == "app" && nkids == 2 ) {
		return term_new_basic2( fnc_app, mnxChildToTerm( mnx, 0 ), mnxChildToTerm( mnx, 1 ) );
	} else if ( name == "package" && mnx->hasAttribute( "url" )) {
		Term t = term_new_package( mnx->attribute( "url" ) );
		Ginger::MnxChildIterator it( mnx );
		while ( it.hasNext() ) {
			term_add( t, mnxToTerm( it.next() ) );
		}
		return t;
	} else if ( name == "import" ) {
		throw Ginger::SystemError( "No longer handles import directly" );
	} else {
		cerr << "name = " << name << endl;
		cerr << "#kids = " << nkids << endl;
		throw Ginger::SystemError( "Unrecognised term" );
	}
}
