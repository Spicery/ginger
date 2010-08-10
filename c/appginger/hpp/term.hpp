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

#ifndef TERM_HPP
#define TERM_HPP

#include <vector>
#include <iostream>
#include "shared.hpp"

#include "functor.hpp"
#include "ident.hpp"
#include "package.hpp"

typedef shared< class TermClass > Term;

//-- general ------------------------------------------------------------

enum NamedRefType {
	LOCAL_REF_TYPE,
	QUALIFIED_REF_TYPE,
	ABSOLUTE_REF_TYPE
};



class TermClass {
public:
	virtual enum Functor functor() = 0;
	virtual Term & child_ref( const int n ) = 0;
	virtual int count() = 0;
	virtual const char * type_name() = 0;
	
	
public:

	virtual Term child( const int n );
	
	
public:

	virtual void add( Term t );	
	virtual Ref ref();	
	virtual Ident & ident();
	virtual enum NamedRefType refType();
	virtual const std::string & pkg();
	virtual const std::string & name();
	virtual const std::string url();
	
};

class KidsTermMixin : public virtual TermClass {
private:
	std::vector< Term > children;
	
public:

	Term & child_ref( const int n ) {
		return children[ n ];
	}
	
	int count() {
		return static_cast< int >( this->children.size() );
	}
	
	void add( Term kid ) {
		this->children.push_back( kid );
	}
			
public:

	KidsTermMixin() 
	{
	}
	
	virtual ~KidsTermMixin() {}
};

class FunctorTermMixin : public virtual TermClass {
private:
	enum Functor functor_data;
	
public:
	virtual enum Functor functor() {
		return this->functor_data;
	}
	
public:
	FunctorTermMixin( enum Functor f ) :
		functor_data( f )
	{
	}
	
	virtual ~FunctorTermMixin() {}
};

class BasicTermClass : 
	public FunctorTermMixin,
	public KidsTermMixin
{
public:
	const char * type_name() { return "BasicTermClass"; }
		
public:

	BasicTermClass(
		enum Functor f
	) :	
		FunctorTermMixin( f ),
		KidsTermMixin()
	{
	}
	
	virtual ~BasicTermClass() {}
};

class PackageTermClass : 
	public BasicTermClass
{
private:
	const std::string url_data;
	
public:
	const std::string url();

public:
	PackageTermClass( const std::string & url ) :
		BasicTermClass( fnc_package ),
		url_data( url )
	{
	}
	
	virtual ~PackageTermClass() {}
};

class NoChildrenTermMixin : public virtual TermClass {
public:
	Term & child_ref( const int n ) {
		std::cerr << "Trying to find the child of " << this->type_name() << std::cerr;
		throw "Invalid index";
	}
	
	int count() {
		return 0;
	}
	
	virtual ~NoChildrenTermMixin() {}
};

class Functor0TermMixin : 
	public FunctorTermMixin, 
	public NoChildrenTermMixin 
{
public:
	Functor0TermMixin(
		enum Functor f
	) :
		FunctorTermMixin( f )
	{
	}
	
	virtual ~Functor0TermMixin() {}
};


class EmptyTermClass : 
	public Functor0TermMixin
{
public:

	const char * type_name() { return "EmptyTermClass"; }

public:
	EmptyTermClass(
		enum Functor f
	) :
		Functor0TermMixin( f )
	{
	}
	
	virtual ~EmptyTermClass() {}

};

//-- ref ----------------------------------------------------------------

class RefTermMixin : public virtual TermClass {
private:
	Ref ref_data;
	
public:
	virtual Ref ref() {
		return this->ref_data;
	}
	
public:
	RefTermMixin( Ref r ) : ref_data( r ) {}
};

class RefTermClass : 
	public FunctorTermMixin, 
	public KidsTermMixin,
	public RefTermMixin 
{
public:
	const char * type_name() { return "RefTermClass"; }
	
public:
	RefTermClass(
		enum Functor f,
		Ref r
	) :
		FunctorTermMixin( f ),
		KidsTermMixin(),
		RefTermMixin( r )
	{
	}
	
	virtual ~RefTermClass() {}
};

//-- char ---------------------------------------------------------------

class CharTermClass : 
	public NoChildrenTermMixin 
{
private:
	char 		char_data;

public:
	CharTermClass( const char ch ) {
		this->char_data = ch;
	}

public:
	char character() {
		return this->char_data;
	}
	
	Ref ref();

	const char * type_name() { return "CharTermClass"; }

	
public:
	enum Functor functor() {
		return fnc_char;
	}
	
	virtual ~CharTermClass() {}
};

//-- string -------------------------------------------------------------
//	Well, I have clearly got this wrong.  There should be some kind of
//	unification possible here.  But I am not entirely sure about the right
//	way to achieve it.

class StringTermClass : 
	public Functor0TermMixin 
{
private:
	std::string 		string_data;

public:
	StringTermClass( Functor fnc, const char * ch ) :
		Functor0TermMixin( fnc ),
		string_data( ch )
	{
	}
	
	virtual ~StringTermClass() {}

public:
	const char * charArray() {
		return this->string_data.c_str();
	}
	
	const char * type_name() { return "StringTermClass"; }

	
};

//-- id -----------------------------------------------------------------
//	We need to record
//		1.	lexical name, hash, length
//		2.	declared attributes (i.e. flags)
//		3.	usage attributes
//		4.	valof (for globals)
//		5.	slot allocated (for locals)

class NamedTermMixin : 	public NoChildrenTermMixin {
private:
	enum NamedRefType	ref_type;
	const std::string	pkg_data;
	const std::string 	name_data;
	Ident				ident_data;

public:
	NamedTermMixin( enum NamedRefType r, const char * p, const char * nm ) :
		ref_type( r ),
		pkg_data( p ),
		name_data( nm )
	{
	}
	
	NamedTermMixin( enum NamedRefType r, const std::string & p, const std::string & nm ) :
		ref_type( r ),
		pkg_data( p ),
		name_data( nm )
	{	
	}
	
	virtual ~NamedTermMixin() {
	}

public:
	Ident & ident() {
		return this->ident_data;
	}
	
	const char * type_name() { return "NamedTermClass"; }

	const std::string & nameString() {
		return this->name_data;
	}
	
	const std::string & name() {
		return this->name_data;
	}
	
	const std::string & pkg() {
		return this->pkg_data;
	}
	
	enum NamedRefType refType() {
		return this->ref_type;
	}
	
};


class IdTermClass : public NamedTermMixin {
public:
	enum Functor functor() {
		return fnc_id;
	}
	
public:
	IdTermClass( enum NamedRefType r, const char * p, const char * nm ) :
		NamedTermMixin( r, p, nm )
	{
	}
	
	IdTermClass( enum NamedRefType r, const std::string & p, const std::string & nm ) :
		NamedTermMixin( r, p, nm )
	{	
	}

	virtual ~IdTermClass() {
	}
};

class VarTermClass : public NamedTermMixin {
private:
	const Facet * facet_data;
	
public:
	const Facet * facet() {
		return this->facet_data;
	}


public:
	enum Functor functor() {
		return fnc_var;
	}
	
public:
	VarTermClass( enum NamedRefType r, const char * p, const char * nm ) :
		NamedTermMixin( r, p, nm )
	{
	}
	
	VarTermClass( enum NamedRefType r, const std::string & p, const Facet * f, const std::string & nm ) :
		NamedTermMixin( r, p, nm ),
		facet_data( f )
	{	
	}
	
	virtual ~VarTermClass() {
	}
};

//-- fn -----------------------------------------------------------------

class FnTermClass : public TermClass {
private:
	Term 	arg_data;
	Term	body_data;
	int 	nlocals_data;
	int 	ninputs_data;

public:
	FnTermClass( Term arg, Term body ) :
		arg_data( arg ),
		body_data( body ),
		nlocals_data( 0 ),
		ninputs_data( 0 )
	{
	}
	
	virtual ~FnTermClass() {
	}

public:

	int count() {
		return 2;
	}
	
	Term & child_ref( int n ) {
		switch ( n ) {
		case 0: return this->arg_data;
		case 1: return this->body_data;
		default: throw "no such index";
		}
	}

	int & nlocals() {
		return this->nlocals_data;
	}
	
	int & ninputs() {
		return this->ninputs_data;
	}
	
	const char * type_name() { return "FnTermClass"; }

	
public:
	enum Functor functor() {
		return fnc_fn;
	}
	
};

//-- item ---------------------------------------------------------------

class ItemTermClass : 
	public Functor0TermMixin, 
	public RefTermMixin
{
public:
	const char * type_name() { return "ItemTermClass"; }
	
public:
	ItemTermClass(
		enum Functor f,
		//Role role,
		Ref r
	) :
		Functor0TermMixin( f ),
		//RoleTermMixin( role ),
		RefTermMixin( r )
	{
	}
	
	virtual ~ItemTermClass() {}
};

//-- from ---------------------------------------------------------------


class FromTermClass : 
	public KidsTermMixin
{
public:
	const char * type_name() { return "FromTermClass"; }
	virtual enum Functor functor() { return fnc_from; }
	
public:
	Ident 	end_expr_ident;
	
public:

	FromTermClass() :
		KidsTermMixin()
	{
	}
	
	virtual ~FromTermClass() {}
};

//-- from ---------------------------------------------------------------


class InTermClass : 
	public KidsTermMixin
{
public:
	const char * type_name() { return "InTermClass"; }
	virtual enum Functor functor() { return fnc_in; }
	
public:
	Ident 	state;
	Ident	context;
	Ident 	next_fn;
	
public:

	InTermClass() :
		KidsTermMixin()
	{
	}
	
	virtual ~InTermClass() {}
};

//-- from ---------------------------------------------------------------

class ImportTermClass : public NoChildrenTermMixin {
public:
	const Facet * 		facet;
	const std::string	from;
	const std::string	alias;
	bool				prot;
	const Facet *		into;
	
public:
	const char * type_name() { return "ImportTermClass"; }
	enum Functor functor() { return fnc_import; }
	
public:
	ImportTermClass( 
		const Facet * 		facet,
		const std::string	from,
		const std::string	alias,
		bool				prot,
		const Facet *		into
	) :
		facet( facet ),
		from( from ),
		alias( alias ),
		prot( prot ),
		into( into )
	{
	}
	
	virtual ~ImportTermClass() {}
};

//------------------------------------------------------------------------------







Term term_new_basic0( Functor fnc );
Term term_new_basic1( Functor fnc, Term x );
Term term_new_basic2( Functor fnc, Term x, Term y );
Term term_new_basic3( Functor fnc, Term x, Term y, Term z );
Term term_add( Term t, Term x );

Term term_new_package( const std::string s );
const std::string term_package_url( Term term );

//Term term_new_simple_item( Item kw );
Ref term_item_extra_ref( Term term );
Instruction term_item_extra_instruction( Term term );

Term term_new_list_empty();

Term term_new_int( const int n );
Term term_new_bool( const bool flag );

Term term_index( Term term, int index );
Term *term_index_ref( Term term, int index );

extern Term term_true;
extern Term term_false;
extern Term term_absent;
extern Term term_skip;
extern Term term_anon;

Term term_new_fn( Term args, Term body );
int term_fn_nlocals( Term term );
int term_fn_ninputs( Term term );
int * term_fn_ninputs_ref( Term term );
int * term_fn_nlocals_ref( Term term );

Term term_new_ref( Functor fnc, Ref r );
Ref term_ref_cont( Term term );

int term_int_cont( Term term );

Term term_new_char( char n );
char term_char_cont( Term term );

Term term_new_string( const char *name );
Term term_new_string( const std::string & s );
const char *term_string_cont( Term term );

Term term_new_sysfn( const char *name );
Term term_new_sysfn( const std::string & s );
const char *term_sysfn_cont( Term term );

Term term_new_absent();

Term term_new_from( Term id, Term start_expr, Term end_expr );
Term term_new_in( Term id, Term expr );

Functor term_functor( Term term );
int term_count( Term term );

void term_print( Term term );

#endif
