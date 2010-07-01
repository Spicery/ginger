#include "functor.hpp"
#include "mishap.hpp"

const char *functor_name( Functor fnc ) {
    switch ( fnc ) {
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
	case fnc_at : return "at";
	case fnc_bind: return "bind";
	case fnc_bindings : return "bindings";
	case fnc_bool: return "bool";
	case fnc_cbrace : return "cbrace";
	case fnc_cbracket : return "cbracket";
	case fnc_char : return "char";
	case fnc_charseq : return "charseq";
	case fnc_comma : return "comma";
	case fnc_conditions : return "conditions";
	case fnc_const : return "const";
	case fnc_cparen : return "cparen";
	case fnc_define : return "define";
	case fnc_div : return "div";
	case fnc_do : return "do";
	case fnc_dot : return "dot";
	case fnc_else : return "else";
	case fnc_elseif : return "elseif";
	case fnc_elseunless : return "elseunless";
	case fnc_enddefine : return "enddefine";
	case fnc_endfn : return "endfn";
	case fnc_endfor : return "endfor";
	case fnc_endif : return "endif";
	case fnc_endunless : return "endunless";
	case fnc_equal : return "equal";
	case fnc_fn : return "fn";
	case fnc_for : return "for";
	case fnc_from : return "from";
	case fnc_gt : return "gt";
	case fnc_gte : return "gte";
	case fnc_id : return "id";
	case fnc_identical: return "identical";
	case fnc_if : return "if";
	case fnc_int : return "int";
	case fnc_lt : return "lt";
	case fnc_lte : return "lte";
	case fnc_mul : return "mul";
	case fnc_not : return "not";
	case fnc_obrace : return "obrace";
	case fnc_obracket : return "obracket";
	case fnc_oparen : return "oparen";
	case fnc_or : return "or";
	case fnc_pow : return "pow";
	case fnc_present : return "present";
	case fnc_semi : return "semi";
	case fnc_skip : return "skip";
	case fnc_string : return "string";
	case fnc_sub : return "sub";
	case fnc_syscall : return "syscall";
	case fnc_then : return "then";
	case fnc_to : return "to";
	case fnc_unless : return "unless";
	case fnc_until : return "until";
	case fnc_val: return "val";
	case fnc_var : return "var";
	case fnc_while : return "while";
    }
    return "?";
}

const char * functor_as_tag( Functor fnc ) {
    switch ( fnc ) {
	case fnc_and : return "and";
	case fnc_assign : return "set";
	case fnc_comma : return "seq";
	case fnc_or : return "or";
	case fnc_semi : return "seq";
	default: throw Mishap( "Internal error - unexpected tag" ).culprit( "Functor", functor_name( fnc ) );
	}
}

const char * functor_as_sysapp( Functor fnc ) {
    switch ( fnc ) {
	case fnc_add : return "+";
	case fnc_arrow : return "newMaplet";
	case fnc_div : return "/";
	case fnc_equal : return "=";
	case fnc_identical: return "==";
	case fnc_gt : return ">";
	case fnc_gte : return ">=";
	case fnc_lt : return "<";
	case fnc_lte : return "<=";
	case fnc_mul : return "*";
	case fnc_not : return "not";
	case fnc_pow : return "**";
	case fnc_sub : return "-";
	default: return NULL;
	}
}

const char *functor_as_type( Functor fnc ) {
    switch ( fnc ) {
	case fnc_absent : return "absent";
	case fnc_present : return "present";
	case fnc_bool : return "bool";
	case fnc_int : return "int";
	case fnc_char : return "char";
	case fnc_string : return "string";
	default: throw Mishap( "Unreachable" );
	}
}