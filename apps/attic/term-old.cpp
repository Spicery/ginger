#include <vector>
using namespace std;

#include "term.hpp"
#include "mishap.hpp"
#include "hash.hpp"
#include "cord.hpp"
#include "ident.hpp"
#include "item.hpp"
#include "key.hpp"

#include <stdarg.h>
#include <stdio.h>


/* 	Layout of term
		----------------------
        |   multi count-1    |
        |    .               |
        |    .               |
        |    .               |
        |   multi 1          |
        |   multi 0 / extra  |
        |--------------------|
        | functor            |  <----
        |--------------------|
		| role               |
        |--------------------|
        | arity              |
        |--------------------|
        | count              |
	    |--------------------|
        |   Term  0          |
        |   Term  1          |
        |     .              |
        |     .              |
        |     .              |
        |   Term  arity-1    |
        ----------------------
*/

class TermClass {
public:
    enum Functor    functor;
	Role			role;
    //int             arity;
	//int				count;
	std::vector< TermClass * > children;	
	std::vector< TermClass * > extras;
};

static TermClass skip_struct = { fnc_skip, SkipRole, std::vector< TermClass * >(), std::vector< TermClass * >() };
TermClass * term_skip = &skip_struct;

static TermClass anon_struct = { fnc_anon, AnonRole, std::vector< TermClass * >(), std::vector< TermClass * >() };
TermClass * term_anon = &anon_struct;

static Term new_empty_term( Functor fnc, int A, int C ) {
	Term t = new TermClass();
	t->functor = fnc;
	t->role = NoRole;
	t->children.resize( A );
	t->extras.resize( C );
#ifdef DBG_TERM
	printf( "new_empty_term returning %x (total = %u, expected %u)\n", (unsigned int)t, total, 4 + A + C );
#endif
	return t;
}


#define children( x ) \
	( (Term *)( (char *)(x) + sizeof( TermClass ) ) )

#define extras( x ) \
	( (multi *)(x) - 1 )

Term term_index( Term term, int n ) {
	Term *c = children( term );
	return c[ n ];
}

Term *term_index_ref( Term term, int n ) {
	Term *c = children( term );
	return c + n;
}

multi term_multi( Term term, int n ) {
	multi *m = extras( term );
	return m[ -n ];
}


//-- ref ----------------------------------------------------------------


Ref term_ref_cont( Term term ) {
	return term_multi( term, 0 ).the_ref;
}


multi *term_multi_ref( Term term, int n ) {
	multi *m = extras( term );
	return m - n;
}

//-- int ----------------------------------------------------------------

int term_int_cont( Term term ) {
	return SmallToInt( term_multi( term, 0 ).the_ref );
}

//-- char ---------------------------------------------------------------

Term term_new_char( char ch ) {
	Term term = new_empty_term( fnc_char, 0, 1 );
	term->role = CharRole;
	term_multi_ref( term, 0 )->the_ref = CharToCharacter( ch );
	return term;
}

char term_char_cont( Term term ) {
	return CharacterToChar( term_multi( term, 0 ).the_ref );
}

//-- string -------------------------------------------------------------
//	Well, I have clearly got this wrong.  There should be some kind of
//	unification possible here.  But I am not entirely sure about the right
//	way to achieve it.

Term term_new_string( const char *str ) {
    Term term = new_empty_term( fnc_string, 0, 1 );
	term->role = StringRole;
	//	To Do
	term_multi_ref( term, 0 )->the_string = strdup( str );
    return term;
}

const char *term_string_cont( Term term ) {
	return term_multi( term, 0 ).the_string;
}



//-- id -----------------------------------------------------------------
//	We need to record
//		1.	lexical name, hash, length
//		2.	declared attributes (i.e. flags)
//		3.	usage attributes
//		4.	valof (for globals)
//		5.	slot allocated (for locals)


Term term_new_id( const char *name ) {
    Term term = new_empty_term( fnc_id, 0, 2 );
	term_multi_ref( term, 0 )->the_cord = new CordClass( name );
	term_multi_ref( term, 1 )->the_ref = NULL;
    return term;
}

bool term_is_id( Term term ) {
	return term->functor == fnc_id;
}

Cord term_id_name( Term term ) {
	return term_multi( term, 0 ).the_cord;
}

Ident term_id_ident( Term term ) {
	return (Ident)term_multi( term, 1 ).the_ref;
}

Ident *term_id_ident_ref( Term term ) {
	return (Ident *)( &(term_multi_ref( term, 1 )->the_ref) );
}

//-- ref ----------------------------------------------------------------

Term term_new_ref( Functor fnc, Ref r ) {
	Term term = new_empty_term( fnc, 0, 1 );
	term_multi_ref( term, 0 )->the_ref = r;
	return term;
}


//-- fn -----------------------------------------------------------------

Term term_new_fn( Term fn, Term args, Term body ) {
	Term term = new_empty_term( fnc_fn, 3, 2 );
	term_fill( term, fn, args, body );
	return term;
}

int term_fn_nlocals( Term term ) {
	return term_multi( term, 0 ).the_int;
}

int term_fn_ninputs( Term term ) {
	return term_multi( term, 1 ).the_int;
}

int *term_fn_nlocals_ref( Term term ) {
	return ( int * )( & ( term_multi_ref( term, 0 )->the_int ) );
}

int *term_fn_ninputs_ref( Term term ) {
	return ( int * )( & ( term_multi_ref( term, 1 )->the_int ) );
}

//-- general terms ------------------------------------------------------


void term_fill( Term term, ... ) {
	int i;
    va_list args;
    va_start( args, term );
    const int n = static_cast< int >( term->children.size() );
    for ( i = 0; i < n; i++ ) {
		term->children[ i ] = va_arg( args, Term  );
    }
}

Term term_new( Functor tag, int size, ... ) {
    int i;
    Term term;
	Term *kids;
    va_list args;
    va_start( args, size );
    term = new_empty_term( tag, size, 0 );
	kids = children( term );
    for ( i = 0; i < size; i++ ) {
		kids[ i ] = va_arg( args, Term );
    }
    va_end( args );
    return term;
}

Term term_new_role( Functor fnc, Role role, int size, ... ) {
    int i;
    Term term;
	Term *kids;
    va_list args;
    va_start( args, size );
    term = new_empty_term( fnc, size, 0 );
	term->role = role;
	kids = children( term );
    for ( i = 0; i < size; i++ ) {
		kids[ i ] = va_arg( args, Term  );
    }
    va_end( args );
    return term;
}

Term term_new_simple_item( Item it ) {
	Term term = new_empty_term( it->functor, 0, 1 );
	multi *m = extras( term );
	term->role = it->role;
	m[0].the_ref = it->extra;
	return term;
}

Term term_new_from_chain( Functor fnc, Chain chain ) {
    int i;
    int a = chain_length( chain );
    Term term = new_empty_term( fnc, a, 0 );
	Term *kids = children( term );
    for ( i = 0; i < a; i++ ) {
		kids[ i ] = (Term)chain_index( chain, i );
    }
    return term;
}

Functor term_functor( Term term ) {
    return term->functor;
}

Role term_role( Term term ) {
	return term->role;
}

int term_arity( Term term ) {
    return static_cast< int >( term->children.size() );
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
    printf( "_%s_\n", functor_name( term->functor ) );

	switch ( term->functor ) {
        case fnc_int: {
    		splevel( level + 1 );
            printf( "%d\n", term_multi( term, 0 ).the_int );
            break;
        }
		case fnc_string: {
    		splevel( level + 1 );
            printf( "%s\n", term_multi( term, 0 ).the_string );
            break;
        }
		case fnc_id: {
			Cord c = term_id_name( term );
			Ident id = term_id_ident( term );
    		splevel( level + 1 );
            printf(
				"%s, length = %lu, hash = %u, ",
				c->text.c_str(), c->length(), c->hash()
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
