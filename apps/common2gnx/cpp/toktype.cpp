#include "toktype.hpp"
#include "mishap.hpp"

const char *tok_type_name( TokType fnc ) {
    switch ( fnc ) {
	case tokty___default : return "__tokty_default";
	case tokty_eof : return "eof";
	case tokty_absand : return "absand";
	case tokty_absent : return "absent";
	case tokty_absnot : return "absnot";
	case tokty_absor : return "absor";
	case tokty_add : return "add";
	case tokty_and : return "and";
	case tokty_anon : return "anon";
	case tokty_app : return "app";
	case tokty_append : return "append";
	case tokty_args : return "args";
	case tokty_arrow : return "arrow";
	case tokty_assign : return "assign";
	case tokty_at : return "at";
	case tokty_bar: return "bar";
	case tokty_bind: return "bind";
	case tokty_bindings : return "bindings";
	case tokty_bool: return "bool";
	case tokty_cbrace : return "cbrace";
	case tokty_cbracket : return "cbracket";
	case tokty_fat_cbrace : return "fat_cbrace";
	case tokty_fat_cbracket : return "fat_cbracket";
	case tokty_char : return "char";
	case tokty_charseq : return "charseq";
	case tokty_comma : return "comma";
	case tokty_conditions : return "conditions";
	case tokty_const : return "const";
	case tokty_cparen : return "cparen";
	case tokty_fat_cparen : return "fat_cparen";
	case tokty_define : return "define";
	case tokty_div : return "div";
	case tokty_do : return "do";
	case tokty_dot : return "dot";
	case tokty_else : return "else";
	case tokty_elseif : return "elseif";
	case tokty_elseunless : return "elseunless";
	case tokty_enddefine : return "enddefine";
	case tokty_endfn : return "endfn";
	case tokty_endfor : return "endfor";
	case tokty_endif : return "endif";
	case tokty_endpackage: return "endpackage";
	case tokty_endunless : return "endunless";
	case tokty_envvar: return "$";
	case tokty_equal : return "equal";
	case tokty_explode: return "explode";
	case tokty_fn : return "fn";
	case tokty_for : return "for";
	case tokty_from : return "from";
	case tokty_gt : return "gt";
	case tokty_gte : return "gte";
	case tokty_id : return "id";
	case tokty_identical: return "identical";
	case tokty_if : return "if";
	case tokty_import: return "import";
	case tokty_in : return "in";
	case tokty_int : return "int";
	case tokty_lt : return "lt";
	case tokty_lte : return "lte";
	case tokty_maplet: return "maplet";
	case tokty_mul : return "mul";
	case tokty_not : return "not";
	case tokty_obrace : return "obrace";
	case tokty_obracket : return "obracket";
	case tokty_oparen : return "oparen";
	case tokty_fat_obrace : return "fat_obrace";
	case tokty_fat_obracket : return "fat_obracket";
	case tokty_fat_oparen : return "fat_oparen";
	case tokty_or : return "or";
	case tokty_package: return "package";
	case tokty_pow : return "pow";
	case tokty_present : return "present";
	case tokty_semi : return "semi";
	case tokty_skip : return "skip";
	case tokty_string : return "string";
	case tokty_sub : return "sub";
	case tokty_syscall : return "syscall";
	case tokty_then : return "then";
	case tokty_to : return "to";
	case tokty_unless : return "unless";
	case tokty_until : return "until";
	case tokty_val: return "val";
	case tokty_var : return "var";
	case tokty_while : return "while";
    }
    return "?";
}

const char * tok_type_as_tag( TokType fnc ) {
    switch ( fnc ) {
	case tokty_and : return "and";
	case tokty_assign : return "set";
	case tokty_comma : return "seq";
	case tokty_or : return "or";
	case tokty_semi : return "seq";
	default: throw Mishap( "Internal error - unexpected tag" ).culprit( "Token Type", tok_type_name( fnc ) );
	}
}

const char * tok_type_as_sysapp( TokType fnc ) {
    switch ( fnc ) {
	case tokty_add : return "+";
	case tokty_append: return "append";
	case tokty_arrow : return "newMaplet";
	case tokty_div : return "/";
	case tokty_equal : return "=";
	case tokty_identical: return "==";
	case tokty_gt : return ">";
	case tokty_gte : return ">=";
	case tokty_lt : return "<";
	case tokty_lte : return "<=";
	case tokty_mul : return "*";
	case tokty_not : return "not";
	case tokty_pow : return "**";
	case tokty_sub : return "-";
	case tokty_maplet : return "newMaplet";
	default: throw Mishap( "Internal error - missing sysapp" ).culprit( "Token type", tok_type_name( fnc ) );
	}
}

const char *tok_type_as_type( TokType fnc ) {
    switch ( fnc ) {
	case tokty_absent : return "absent";
	case tokty_present : return "present";
	case tokty_bool : return "bool";
	case tokty_int : return "int";
	case tokty_char : return "char";
	case tokty_string : return "string";
	default: throw Mishap( "Unreachable" );
	}
}
