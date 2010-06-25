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

class TermClass {
public:
	virtual enum Functor functor() = 0;
	virtual Term & child_ref( const int n ) = 0;
	virtual int arity() = 0;
	virtual const char * type_name() = 0;
	
public:

	virtual Term child( const int n ) {
		return this->child_ref( n );
	}
	
	
public:

	virtual void add( Term t ) {
		cerr << this->type_name() << endl;
		throw "No add for this type";
	}
	
	virtual Ref ref() {
		cerr << this->type_name() << endl;
		throw "No ref for this type";
	}
	
	virtual Ident & ident() {
		cerr << this->type_name() << endl;
		throw "No ident for this type";
	}
	
	virtual const std::string & name() {
		cerr << this->type_name() << endl;
		throw "No name for this type";
	}
	
};

class KidsTermMixin : public virtual TermClass {
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

class NoChildrenTermMixin : public virtual TermClass {
public:
	Term & child_ref( const int n ) {
		fprintf( stderr, "Trying to find the child of %s\n", this->type_name() );
		throw "Invalid index";
	}
	
	int arity() {
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

Term term_new_ref( Functor fnc, Ref r ) {
	return shared< TermClass >( new RefTermClass( fnc, r ) );
}


Ref term_ref_cont( Term term ) {
	return term->ref();
}


//-- int ----------------------------------------------------------------

int term_int_cont( Term term ) {
	Ref r = term->ref();
	return SmallToLong( r );
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
	
	Ref ref() {
		return IntToChar( this->char_data );
	}

	const char * type_name() { return "CharTermClass"; }

	
public:
	enum Functor functor() {
		return fnc_char;
	}
	
	virtual ~CharTermClass() {}
};

Term term_new_char( const char ch ) {
	return shared< TermClass >( new CharTermClass( ch ) );
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
	
	virtual ~StringTermClass() {}

public:
	const char * charArray() {
		return this->string_data.c_str();
	}
	
	const char * type_name() { return "StringTermClass"; }

	
public:
	enum Functor functor() {
		return fnc_string;
	}
		
};



Term term_new_string( const char *str ) {
	return shared< TermClass >( new StringTermClass( str ) );
}

Term term_new_string( const std::string & str ) {
	return shared< TermClass >( new StringTermClass( str.c_str() ) );
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

class NamedTermClass : 
	public FunctorTermMixin,
	public NoChildrenTermMixin 
{
private:
	const std::string 	name_data;
	Ident				ident_data;

public:
	NamedTermClass( Functor fnc, const char * nm ) :
		FunctorTermMixin( fnc ),
		name_data( nm )
	{
	}
	
	NamedTermClass( Functor fnc, const std::string & nm ) :
		FunctorTermMixin( fnc ),
		name_data( nm )
	{	
	}
	
	virtual ~NamedTermClass() {
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
	
};



Term term_new_named( Functor fnc, const std::string & name ) {
	return shared< TermClass >( new NamedTermClass( fnc, name ) );
}

const std::string & term_named_string( Term term ) {
	return term->name();
}

Ident & term_named_ident( Term term ) {
	return term->ident();
}

bool term_is_id( Term term ) {
	return term->functor() == fnc_id;
}


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
		body_data( body )
	{
	}
	
	virtual ~FnTermClass() {
	}

public:

	int arity() {
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


Term term_new_fn( Term args, Term body ) {
	return shared< TermClass >( new FnTermClass( args, body ) );
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

//-- general terms ------------------------------------------------------



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

Term term_new_int( const int n ) {
	return 
		shared< TermClass >(
			new ItemTermClass( fnc_int, IntToRef( n ) )
		)
	;
}

Term term_new_bool( const bool flag ) {
	return 
		shared< TermClass >(
			new ItemTermClass( fnc_bool, sys_true )
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



Functor term_functor( Term term ) {
    return term->functor();
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
			const std::string & c = term->name();
			Ident & id = term->ident();
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

Term term_skip( new EmptyTermClass( fnc_seq ) );
Term term_anon( new EmptyTermClass( fnc_anon ) );

