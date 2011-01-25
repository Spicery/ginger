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

#include "debug.hpp"

#include <iostream>
#include <vector>
#include <cstdarg>
#include <typeinfo>
using namespace std;

#include "term.hpp"
#include "mishap.hpp"
#include "ident.hpp"
#include "key.hpp"

#include <stdio.h>

//#define DBG_LIFTING


void TermClass::add( Term t ) {
	std::cerr << this->type_name() << std::endl;
	throw "No add for this type";
}

Ref TermClass::ref() {
	std::cerr << this->type_name() << std::endl;
	throw "No ref for this type";
}

Ident & TermClass::ident() {
	std::cerr << this->type_name() << std::endl;
	throw "No ident for this type";
}

enum NamedRefType TermClass::refType() {
	std::cerr << this->type_name() << std::endl;
	throw "No refType for this type";
}

const std::string & TermClass::encPkg() {
	std::cerr << this->type_name() << std::endl;
	throw "No package name for this type";
}

const std::string & TermClass::defPkg() {
	std::cerr << this->type_name() << std::endl;
	throw "No package name for this type";
}

const std::string & TermClass::name() {
	cerr << this->type_name() << endl;
	throw "No name for this type";
}

const std::string TermClass::url() {
	std::cerr << this->type_name() << std::endl;
	throw "No package name for this type";
}

const std::string PackageTermClass::url() { 
	return this->url_data; 
}



Term TermClass::child( int n ) {
	return this->child_ref( n );
}



Term term_index( Term term, int n ) {
	return term->child( n );
}

Term *term_index_ref( Term term, int n ) {
	return &term->child_ref( n );
}




Term term_new_ref( Functor fnc, Ref r ) {
	return shared< TermClass >( new RefTermClass( fnc, r ) );
}


Ref term_ref_cont( Term term ) {
	return term->ref();
}

//- int

int term_int_cont( Term term ) {
	Ref r = term->ref();
	return SmallToLong( r );
}

//-	char

Ref CharTermClass::ref() {
	return IntToChar( this->char_data );
}

Term term_new_char( const char ch ) {
	return shared< TermClass >( new CharTermClass( ch ) );
}

char term_char_cont( Term term ) {
	return dynamic_cast< CharTermClass * >( term.get() )->character();
}




Term term_new_string( const char *str ) {
	return shared< TermClass >( new StringTermClass( fnc_string, str ) );
}

Term term_new_string( const std::string & str ) {
	return shared< TermClass >( new StringTermClass( fnc_string, str.c_str() ) );
}

const char *term_string_cont( Term term ) {
	return dynamic_cast< StringTermClass * >( term.get() )->charArray();
}

Term term_new_symbol( const char *str ) {
	return shared< TermClass >( new StringTermClass( fnc_symbol, str ) );
}

Term term_new_symbol( const std::string & str ) {
	return shared< TermClass >( new StringTermClass( fnc_symbol, str.c_str() ) );
}

const char *term_symbol_cont( Term term ) {
	return dynamic_cast< StringTermClass * >( term.get() )->charArray();
}

Term term_new_sysfn( const char *str ) {
	return shared< TermClass >( new StringTermClass( fnc_sysfn, str ) );
}

Term term_new_sysfn( const std::string & str ) {
	return shared< TermClass >( new StringTermClass( fnc_sysfn, str.c_str() ) );
}

const char *term_sysfn_cont( Term term ) {
	return dynamic_cast< StringTermClass * >( term.get() )->charArray();
}


Term term_new_fn( const std::string name, Term args, Term body ) {
	return shared< TermClass >( new FnTermClass( name, args, body ) );
}

int term_fn_nlocals( Term term ) {
	return dynamic_cast< FnTermClass * >( term.get() )->nlocals();
}

int term_fn_ninputs( Term term ) {
	return dynamic_cast< FnTermClass * >( term.get() )->ninputs();
}

int *term_fn_nlocals_ref( Term term ) {
	return &dynamic_cast< FnTermClass * >( term.get() )->nlocals();
}

int *term_fn_ninputs_ref( Term term ) {
	return &dynamic_cast< FnTermClass * >( term.get() )->ninputs();
}



//- general terms --------------------------------------------------------------



Term term_new_basic0( Functor fnc ) {
	return shared< TermClass >( new BasicTermClass( fnc ) );
}

Term term_new_basic1( Functor fnc, Term x ) {
	Term t( new BasicTermClass( fnc ) );
	t->add( x );
	return t;
}

Term term_new_basic2( Functor fnc, Term x, Term y ) {
	Term t( new BasicTermClass( fnc ) );
	t->add( x );
	t->add( y );
	return t;
}

Term term_new_basic3( Functor fnc, Term x, Term y, Term z ) {
	Term t( new BasicTermClass( fnc ) );
	t->add( x );
	t->add( y );
	t->add( z );
	return t;
}

Term term_add( Term t, Term x ) {
	t->add( x );
	return t;
}

Term term_new_package( const std::string url ) {
	return 
		shared< TermClass >(
			new PackageTermClass( url )
		)
	;
}

const std::string term_package_url( Term term ) {
	return term->url();
}

Term term_new_int( const int n ) {
	return 
		shared< TermClass >(
			new ItemTermClass( fnc_int, LongToRef( n ) )
		)
	;
}

Term term_new_list_empty() {
	return shared< TermClass >( new ItemTermClass( fnc_list, sys_nil ) );
}

Term term_new_bool( const bool flag ) {
	return 
		shared< TermClass >(
			new ItemTermClass( fnc_bool, flag ? sys_true : sys_false )
		)
	;	
}

Term term_new_absent() {
	return 
		shared< TermClass >(
			new ItemTermClass( fnc_absent, sys_absent )
		)
	;	
}

Term term_new_from( Term id, Term start_expr, Term end_expr ) {
	Term x( new FromTermClass() );
	x->add( id );
	x->add( start_expr );
	x->add( end_expr );
	return x;
}

Term term_new_in( Term id, Term expr ) {
	Term x( new InTermClass() );
	x->add( id );
	x->add( expr );
	return x;
}

Functor term_functor( Term term ) {
    return term->functor();
}

int term_count( Term term ) {
    return term->count();
}


// -- fn

void FnTermClass::addOuter( Ident id ) {
	this->outers.push_back( id );
}

void FnTermClass::addInner( Ident id ) {
	this->inners.push_back( id );
}

void FnTermClass::setAsFinalInput( Ident new_input ) {
	int N = this->ninputs_data++;

	#ifdef DBG_LIFTING
		cerr << "Bumping #inputs for " << this->name() << " from " << this->ninputs_data-1 << " to " << this->ninputs_data << endl;
	#endif
	
	//	Now we need to swap nin and slot. Check there is a need to make
	//	changes!
	int slot = new_input->getFinalSlot();
	if ( N != slot ) {
		for ( std::vector< Ident >::iterator it = this->inners.begin(); it != this->inners.end(); ++it ) {
			it->get()->swapSlot( N, slot );
		}
	
	}
}



//-- printing -----------------------------------------------------------

static void splevel( int level ) {
    int i;
    level *= 4;
    for ( i = 0; i < level; i++ ) {
        putchar( ' ' );
    }
}

static void tprint( Term term, int level ) {
    splevel( level );
    printf( "[%d,%s]", term->count(), term->type_name() );	//	DEBUG ONLY
    printf( "_%s_\n", functor_name( term->functor() ) );

	switch ( term->functor() ) {
        case fnc_int: {
    		splevel( level + 1 );
            printf( "%d\n", term_int_cont( term ) );
            break;
        }
		case fnc_string: {
    		splevel( level + 1 );
            printf( "%s\n", term_string_cont( term ) );
            break;
        }
		case fnc_id: {
			const std::string & c = term->name();
			Ident & id = term->ident();
    		splevel( level + 1 );
            printf(
				"%s, length = %Zu, ",
				c.c_str(), c.size()
			);
			if ( id == NULL ) {
				printf( "slot = <null>" );
			} else if ( id->isLocal() ) {
				printf( "slot = %d", id->getFinalSlot() );
			} else {
				printf( "global" );
			}
			printf( "\n" );
            break;
        }
		default: {
            //  printf( "(%d children to print)\n", term->count );
            {
            	const int n = term_count( term );
				for ( int i = 0; i < n; i++ ) {
					tprint( term_index( term, i ), level + 1 );
				}
            }
		}
    }
}

void term_print( Term term ) {
	if ( term == NULL ) {
		printf( "<none>\n" );
	} else {
    	tprint( term, 0 );
	}
}

//-- globals ------------------------------------------------------------

Term term_skip( new EmptyTermClass( fnc_seq ) );
Term term_anon( new EmptyTermClass( fnc_anon ) );


