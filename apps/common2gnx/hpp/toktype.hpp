#ifndef TOKTYPE_HPP
#define TOKTYPE_HPP

typedef enum TokType {
     tokty___default,
     tokty_eof,
     tokty_absand,
     tokty_absent,
     tokty_absnot,
     tokty_absor,
     tokty_add,
     tokty_and,
     tokty_anon,
     tokty_app,
     tokty_append,
     tokty_args,
     tokty_arrow,
     tokty_assign,
     tokty_at,
     tokty_bar,
     tokty_bind,
     tokty_bindings,
     tokty_bool,
     tokty_cbrace,
     tokty_cbracket,
     tokty_fat_cbrace,
     tokty_fat_cbracket,
     tokty_char,
     tokty_charseq,
     tokty_comma,
     tokty_conditions,
     tokty_const,
     tokty_cparen,
     tokty_fat_cparen,
     tokty_define,
     tokty_div,
     tokty_do,
     tokty_dot,
     tokty_else,
     tokty_elseif,
     tokty_elseunless,
     tokty_enddefine,
     tokty_endfn,
     tokty_endfor,
     tokty_endif,
     tokty_endpackage,
     tokty_endunless,
     tokty_equal,
     tokty_explode,
     tokty_fn,
     tokty_for,
     tokty_from,
     tokty_gt,
     tokty_gte,
     tokty_id,
     tokty_identical,
     tokty_if,
     tokty_import,
     tokty_in,
     tokty_int,
     tokty_lt,
     tokty_lte,
     tokty_maplet,
     tokty_mul,
     tokty_not,
     tokty_obrace,
     tokty_obracket,
     tokty_oparen,
     tokty_fat_obrace,
     tokty_fat_obracket,
     tokty_fat_oparen,
     tokty_or,
     tokty_package,
     tokty_pow,
     tokty_present,
     tokty_semi,
     tokty_skip,
     tokty_string,
     tokty_sub,
     tokty_syscall,
     tokty_then,
     tokty_to,
     tokty_unless,
     tokty_until,
     tokty_val,
     tokty_var,
     tokty_while
} TokType;

const char *tok_type_name( TokType fnc );
const char *tok_type_as_tag( TokType fnc );
const char *tok_type_as_sysapp( TokType fnc );
const char *tok_type_as_type( TokType fnc );
//const char *functor_as_value( Functor fnc );

#endif
