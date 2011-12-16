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

static void addCulprits( shared< Ginger::Mnx > mnx, Ginger::Problem & p ) {
	Ginger::MnxChildIterator kids( mnx );
	while ( kids.hasNext() ) {
		shared< Ginger::Mnx > kid( kids.next() );
		if ( kid->hasAttribute( "name" ) && kid->hasAttribute( "value" ) ) {
			p.culprit( kid->attribute( "name" ), kid->attribute( "value" ) );
		}
	}
}

Term mnxToTerm( shared< Ginger::Mnx > mnx ) {
	const string name( mnx->name() );
	const int nkids = mnx->size();
	if ( name == "fn" && nkids == 2 ) {
		return term_new_fn( mnx->attribute( "name", "" ), mnxChildToTerm( mnx, 0 ), mnxChildToTerm( mnx, 1 ) );
	} else if ( mnx->hasAttribute( "value" ) && mnx->hasAttribute( "type" ) ) {
	
		/*OK THIS IS WRONG - sysfn can never be reached as it will not have a 'type' attribute.
		There are a couple of ways out of this but regardless of what I choose I need to
		define some regression tests first.*/

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
	} else if ( name == "list" ) {
		if ( mnx->isEmpty() ) {
			return term_new_list_empty();
		} else {
			vector< Term > kids;
			fillKids( mnx, kids );
			return makeSysApp( "newList", kids );
		}
	} else if ( name == "vector" ) {
		vector< Term > kids;
		fillKids( mnx, kids );
		return makeSysApp( "newVector", kids );	
	} else if ( name == "throw" && nkids == 1 ) {
		return term_new_basic1( fnc_throw, mnxChildToTerm( mnx, 0 ) );
	} else if ( name == "assert" && nkids == 1 ) {
		//	Added in case-study: Adding New Element Type.
		if ( mnx->hasAttribute( "type", "bool" ) ) {
			return term_new_basic1( fnc_assert_bool, mnxChildToTerm( mnx, 0 ) );
		} else if ( mnx->hasAttribute( "n", "1" ) ) {
			return term_new_basic1( fnc_assert_single, mnxChildToTerm( mnx, 0 ) );
		}
		//	Fall through is treated as failure.
		throw Ginger::SystemError( "Unrecognised assert form" );
	} else if ( name == "import" ) {
		throw Ginger::SystemError( "No longer handles import directly" );
	} else if ( name == "problem" ) {
		Ginger::CompileTimeError mishap( mnx->attribute( "message" ) );
		addCulprits( mnx, mishap );
		throw mishap;
	} else {
		Ginger::CompileTimeError syserr( "Unrecognised term" );
		syserr.culprit( "Name", name );
		syserr.culprit( "#Kids", (long)nkids );
		throw syserr;
	}
}
