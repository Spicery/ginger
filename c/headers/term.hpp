#ifndef TERM_HPP
#define TERM_HPP

#include "shared.hpp"

#include "functor.hpp"
#include "ident.hpp"

typedef shared< class TermClass > Term;


Term term_new_basic0( Functor fnc );
Term term_new_basic1( Functor fnc, Term x );
Term term_new_basic2( Functor fnc, Term x, Term y );
Term term_new_basic3( Functor fnc, Term x, Term y, Term z );
Term term_add( Term t, Term x );

//Term term_new_simple_item( Item kw );
Ref term_item_extra_ref( Term term );
Instruction term_item_extra_instruction( Term term );

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

Term term_new_absent();

Term term_new_named( Functor fnc, const std::string & name );
const std::string & term_named_string( Term );
Ident & term_named_ident( Term term );
bool term_is_id( Term term );


Functor term_functor( Term term );
int term_arity( Term term );
int term_count( Term term );

void term_print( Term term );

#endif
