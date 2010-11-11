#include <iostream>
#include <vector>
#include <cstdarg>
#include <typeinfo>
using namespace std;

#include "term.hpp"
#include "mishap.hpp"
#include "ident.hpp"
#include "item.hpp"
#include "key.hpp"

#include <stdio.h>

class TermClass {
public:
	virtual enum Functor functor() = 0;
	virtual Term & child_ref( const int n ) = 0;
	virtual int arity() = 0;
	virtual const char * type_name() = 0;
	
public:

	virtual Role role() { 
		return NoRole; 
	}
	
	virtual Term child( const int n ) {
		return this->child_ref( n );
	}
	
	
public:

	virtual void add( Term t ) {
		cerr << this->type_name() << endl;
		throw "No add for this type";
	}
	
	virtual Ref & ref() {
		cerr << this->type_name() << endl;
		throw "No ref for this type";
	}
	
	virtual Ident & ident() {
		cerr << this->type_name() << endl;
		throw "No ident for this type";
	}
	
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
};

class RoleTermMixin : public virtual TermClass {
private:
	Role role_data;

public:
	Role role() {
		return this->role_data;
	}

public:
	RoleTermMixin( Role r ) : role_data( r ) {}
};

class BasicTermClass : 
	public FunctorTermMixin, 
	public RoleTermMixin 
{
private:
	std::vector< Term > children;
	
public:

	Term & child_ref( const int n ) {
		return children[ n ];
	}
	
	int arity() {
		return static_cast< int >( this->children.size() );
	}
	
	void add( Term kid ) {
		this->children.push_back( kid );
	}
	
	const char * type_name() { return "BasicTermClass"; }
		
public:

	BasicTermClass(
		enum Functor f,
		Role r
	) :	
		FunctorTermMixin( f ),
		RoleTermMixin( r )
	{
	}
};

class NoChildrenTermMixin : public virtual TermClass {
public:
	Term & child_ref( const int n ) {
		fprintf( stderr, "Trying to find the child of %s\n", this->type_name() );
		throw "Invalid index";
	}
	
	int arity() {
		return 0;
	}
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
};


class EmptyTermClass : 
	public Functor0TermMixin, 
	public RoleTermMixin 
{
public:

	const char * type_name() { return "EmptyTermClass"; }

public:
	EmptyTermClass(
		enum Functor f,
		Role r
	) :
		Functor0TermMixin( f ),
		RoleTermMixin( r )
	{
	}

};


//-- general ------------------------------------------------------------


Term term_index( Term term, int n ) {
	return term->child( n );
}

Term *term_index_ref( Term term, int n ) {
	return &term->child_ref( n );
}


//-- ref ----------------------------------------------------------------

class RefTermMixin : public virtual TermClass {
private:
	Ref ref_data;
	
public:
	virtual Ref & ref() {
		return this->ref_data;
	}
	
public:
	RefTermMixin( Ref r ) : ref_data( r ) {}
};

class RefTermClass : 
	public Functor0TermMixin, 
	public RefTermMixin 
{
public:
	virtual Role role() {
		return NoRole;
	}
	
	const char * type_name() { return "RefTermClass"; }
	
public:
	RefTermClass(
		enum Functor f,
		Ref r
	) :
		Functor0TermMixin( f ),
		RefTermMixin( r )
	{
	}
	
};

Term term_new_ref( Functor fnc, Ref r ) {
	return boost::shared_ptr< TermClass >( new RefTermClass( fnc, r ) );
}


Ref term_ref_cont( Term term ) {
	return term->ref();
}


//-- int ----------------------------------------------------------------

int term_int_cont( Term term ) {
	Ref r = term->ref();
	return SmallToInt( r );
}

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

	const char * type_name() { return "CharTermClass"; }

	
public:
	enum Functor functor() {
		return fnc_char;
	}
	
	Role role() {
		return CharRole;
	}
};

Term term_new_char( const char ch ) {
	return boost::shared_ptr< TermClass >( new CharTermClass( ch ) );
}

char term_char_cont( Term term ) {
	return dynamic_cast< CharTermClass * >( term.get() )->character();
}

//-- string -------------------------------------------------------------
//	Well, I have clearly got this wrong.  There should be some kind of
//	unification possible here.  But I am not entirely sure about the right
//	way to achieve it.

class StringTermClass : 
	public NoChildrenTermMixin 
{
private:
	std::string 		string_data;

public:
	StringTermClass( const char * ch ) :
		string_data( ch )
	{
	}

public:
	const char * charArray() {
		return this->string_data.c_str();
	}
	
	const char * type_name() { return "StringTermClass"; }

	
public:
	enum Functor functor() {
		return fnc_string;
	}
	
	Role role() {
		return StringRole;
	}
};



Term term_new_string( const char *str ) {
	return boost::shared_ptr< TermClass >( new StringTermClass( str ) );
}

const char *term_string_cont( Term term ) {
	return dynamic_cast< StringTermClass * >( term.get() )->charArray();
}



//-- id -----------------------------------------------------------------
//	We need to record
//		1.	lexical name, hash, length
//		2.	declared attributes (i.e. flags)
//		3.	usage attributes
//		4.	valof (for globals)
//		5.	slot allocated (for locals)

class IdTermClass : 
	public NoChildrenTermMixin 
{
private:
	const std::string 	name_data;
	Ident				ident_data;

public:
	IdTermClass( const char * nm ) :
		name_data( nm )
	{
	}
	
	IdTermClass( const std::string & nm ) :
		name_data( nm )
	{	
	}
	
	virtual ~IdTermClass() {
		//delete this->name_data;
	}

public:
	/*Cord & name() {
		return this->name_data;
	}*/
	
	Ident & ident() {
		return this->ident_data;
	}
	
	const char * type_name() { return "IdTermClass"; }

	const std::string & nameString() {
		return this->name_data;
	}
public:
	enum Functor functor() {
		return fnc_id;
	}
	
	Role role() {
		return NoRole;
	}
};



Term term_new_id( const std::string & name ) {
	return boost::shared_ptr< TermClass >( new IdTermClass( name ) );
}

const std::string & term_id_string( Term term ) {
	return dynamic_cast< IdTermClass * >( term.get() )->nameString();
}

Ident term_id_ident( Term term ) {
	return term->ident();
}

Ident *term_id_ident_ref( Term term ) {
	return &dynamic_cast< IdTermClass * >( term.get() )->ident();
}

bool term_is_id( Term term ) {
	return term->functor() == fnc_id;
}


//-- fn -----------------------------------------------------------------

class FnTermClass : public TermClass {
private:
	Term	fn_data;
	Term 	arg_data;
	Term	body_data;
	int 	nlocals_data;
	int 	ninputs_data;

public:
	FnTermClass( Term fn, Term arg, Term body ) :
		fn_data( fn ),
		arg_data( arg ),
		body_data( body )
	{
	}
	
	virtual ~FnTermClass() {
	}

public:

	int arity() {
		return 3;
	}
	
	Term & child_ref( int n ) {
		switch ( n ) {
		case 0: return this->fn_data; 
		case 1: return this->arg_data;
		case 2: return this->body_data;
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
	
	Role role() {
		return NoRole;
	}
};


Term term_new_fn( Term fn, Term args, Term body ) {
	return boost::shared_ptr< TermClass >( new FnTermClass( fn, args, body ) );
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

//-- item ---------------------------------------------------------------

class ItemTermClass : 
	public Functor0TermMixin, 
	public RoleTermMixin,
	public RefTermMixin
{
public:
	const char * type_name() { return "ItemTermClass"; }
	
public:
	ItemTermClass(
		enum Functor f,
		Role role,
		Ref r
	) :
		Functor0TermMixin( f ),
		RoleTermMixin( role ),
		RefTermMixin( r )
	{
	}
};

//-- general terms ------------------------------------------------------



Term term_new_basic0( Functor fnc ) {
	return boost::shared_ptr< TermClass >( new BasicTermClass( fnc, NoRole ) );
}

Term term_new_basic0( Functor fnc, Role role ) {
	return boost::shared_ptr< TermClass >( new BasicTermClass( fnc, role ) );
}

Term term_new_basic1( Functor fnc, Role role, Term x ) {
	Term t( new BasicTermClass( fnc, role ) );
	t->add( x );
	return t;
}

Term term_new_basic1( Functor fnc, Term x ) {
	return term_new_basic1( fnc, NoRole, x );
}

Term term_new_basic2( Functor fnc, Role r, Term x, Term y ) {
	Term t( new BasicTermClass( fnc, NoRole ) );
	t->add( x );
	t->add( y );
	return t;
}

Term term_new_basic2( Functor fnc, Term x, Term y ) {
	return term_new_basic2( fnc, NoRole, x, y );
}

Term term_new_basic3( Functor fnc, Role r, Term x, Term y, Term z ) {
	Term t( new BasicTermClass( fnc, r ) );
	t->add( x );
	t->add( y );
	t->add( z );
	return t;
}

Term term_new_basic3( Functor fnc, Term x, Term y, Term z ) {
	return term_new_basic3( fnc, NoRole, x, y, z );
}

Term term_add( Term t, Term x ) {
	t->add( x );
	return t;
}

Term term_new_simple_item( Item it ) {
	return boost::shared_ptr< TermClass >( new ItemTermClass( it->functor, it->role, it->extra ) );
}

Functor term_functor( Term term ) {
    return term->functor();
}

Role term_role( Term term ) {
	return term->role();
}

int term_arity( Term term ) {
    return term->arity();
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
    printf( "[%d,%s]", term->arity(), term->type_name() );	//	DEBUG ONLY
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
			const std::string & c = term_id_string( term );
			Ident id = term_id_ident( term );
    		splevel( level + 1 );
            printf(
				"%s, length = %lu, ",
				c.c_str(), c.size()
			);
			if ( id == NULL ) {
				printf( "slot = <null>" );
			} else if ( id->is_local ) {
				printf( "slot = %d", id->slot );
			} else {
				printf( "global" );
			}
			printf( "\n" );
            break;
        }
		default: {
            //  printf( "(%d children to print)\n", term->arity );
            {
            	const int n = term_arity( term );
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

Term term_skip( new EmptyTermClass( fnc_skip, SkipRole ) );
Term term_anon( new EmptyTermClass( fnc_anon, AnonRole ) );

