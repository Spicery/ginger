#ifndef FUNCTOR_HPP
#define FUNCTOR_HPP

#include "instruction.hpp"
#include "common.hpp"

typedef enum Functor {
	fnc___fnc_default,
	fnc__print,
	fnc__putchar,
	fnc_eof,
	fnc_absand,
	fnc_absent,
	fnc_absnot,
	fnc_absor,
	fnc_add,
	fnc_and,
	fnc_anon,
	fnc_app,
	fnc_append,
	fnc_appspc,
	fnc_args,
	fnc_arrow,
	fnc_assign,
	fnc_bindings,
	fnc_bool,
	fnc_char,
	fnc_charseq,
	fnc_conditions,
	fnc_const,
	fnc_dec,
	fnc_decr_by,
	fnc_define,
	fnc_div,
	fnc_eq,
	fnc_fn,
	fnc_for,
	fnc_gt,
	fnc_gte,
	fnc_id,
	fnc_if,
	fnc_incr_by,
	fnc_int,
	fnc_lt,
	fnc_lte,
	fnc_mul,
	fnc_not,
	fnc_or,
	fnc_pow,
	fnc_present,
	fnc_seq,
	fnc_string,
	fnc_sub,
	fnc_syscall,
	fnc_until,
	fnc_var,
	fnc_while
} Functor;

const char *functor_name( Functor fnc );
Instruction functor_inline( enum Functor tag );
Ref functor_value( enum Functor tag );


#endif
