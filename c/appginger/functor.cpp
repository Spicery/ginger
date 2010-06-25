#include "functor.hpp"
#include "key.hpp"

const char * functor_name( Functor tag ) {
    switch ( tag ) {
	case fnc___fnc_default : return "__fnc_default";
	case fnc__print : return "_print";
	case fnc__putchar : return "_putchar";
	case fnc_eof : return "eof";
	case fnc_absand : return "absand";
	case fnc_absent : return "absent";
	case fnc_absnot : return "absnot";
	case fnc_absor : return "absor";
	case fnc_add : return "add";
	case fnc_and : return "and";
	case fnc_anon : return "anon";
	case fnc_app : return "app";
	case fnc_append : return "append";
	case fnc_appspc : return "appspc";
	case fnc_args : return "args";
	case fnc_arrow : return "arrow";
	case fnc_assign : return "assign";
	case fnc_bindings : return "bindings";
	case fnc_bool: return "bool";
	case fnc_char : return "char";
	case fnc_charseq : return "charseq";
	case fnc_conditions : return "conditions";
	case fnc_const : return "const";
	case fnc_dec: return "dec";
	case fnc_decr_by : return "decr_by";
	case fnc_define : return "define";
	case fnc_div : return "div";
	case fnc_eq : return "eq";
	case fnc_fn : return "fn";
	case fnc_for : return "for";
	case fnc_gt : return "gt";
	case fnc_gte : return "gte";
	case fnc_id : return "id";
	case fnc_if : return "if";
	case fnc_incr_by : return "incr_by";
	case fnc_int : return "int";
	case fnc_lt : return "lt";
	case fnc_lte : return "lte";
	case fnc_mul : return "mul";
	case fnc_not : return "not";
	case fnc_or : return "or";
	case fnc_pow : return "pow";
	case fnc_present : return "present";
	case fnc_seq : return "seq";
	case fnc_string : return "string";
	case fnc_sub : return "sub";
	case fnc_syscall : return "syscall";
	case fnc_until : return "until";
	case fnc_var : return "var";
	case fnc_while : return "while";
    }
    return "?";
}

Instruction functor_inline( enum Functor tag ) {
	switch ( tag ) {
	case fnc__print: 	return vmc__print;
	case fnc__putchar:	return vmc__putchar;
	default: 			throw "No inline value";
	}
}

Ref functor_value( enum Functor tag ) {
	switch ( tag ) {
	case fnc_absent:	return sys_absent;
	//case fnc_false:		return sys_false;
	//case fnc_true:		return sys_true;
	default:			throw "No value";
	}
	
}
