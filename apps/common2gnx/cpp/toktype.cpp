#include "keywords.hpp"
#include "toktype.hpp"
#include "mishap.hpp"

#define X(a,b) case a : return b;
const char *tok_type_name( TokType fnc ) {
    switch ( fnc ) {
		#include "toktype.xdef"
    }
    return "?";
}
#undef X


const bool tok_type_as_direction( TokType fnc ) {
	switch ( fnc ) {
		case tokty_equal:
		case tokty_assignrev: 
			return false;
		default: 
			return true;
	}
}

const char * tok_type_as_tag( TokType fnc ) {
    switch ( fnc ) {
		case tokty_absor : return "absor";
		case tokty_absand : return "absand";
		case tokty_dbang : return "absnot";
    	case tokty_also: return "also";
		case tokty_and : return "and";
		case tokty_equal:
		case tokty_assign : 
		case tokty_assignrev : 
			//	The swapping of arguments us handled by tok_type_as_direction.
			return "set";
		case tokty_comma : return "seq";
		case tokty_or : return "or";
		case tokty_semi : return "seq";
		default: throw CompileTimeError( "Internal error (tok_type_as_tag): unexpected tag" ).culprit( "Token Type", tok_type_name( fnc ) );
	}
}

const char * tok_type_as_sysapp( TokType fnc ) {
    switch ( fnc ) {
	case tokty_add : return "+";
	case tokty_append: return "append";
	case tokty_bang: return "absNot";
	case tokty_dbang: return "boolAbs";
	case tokty_div : return "/";
	case tokty_equal : return "=";
	case tokty_explode: return "explode";
	case tokty_gt : return ">";
	case tokty_gte : return ">=";
	case tokty_identical: return "==";
	case tokty_isntabsent: return "isntAbsent";
	case tokty_lt : return "<";
	case tokty_lte : return "<=";
	case tokty_ltegt: return "<=>";
	case tokty_ltgt: return "<>";
	case tokty_maplet : return "newMaplet";
	case tokty_mod : return "mod";
	case tokty_mul : return "*";
	case tokty_not : return "not";
	case tokty_not_equal: return "!=";
	case tokty_not_gt: return "!>";
	case tokty_not_gte: return "!>=";
	case tokty_not_lt: return "!<";
	case tokty_not_lte: return "!<=";
	case tokty_not_ltegt: return "!<=>";
	case tokty_not_ltgt: return "!<>";
	case tokty_pow : return "**";
	case tokty_quo : return "quo";
	case tokty_sub : return "-";
	default: throw CompileTimeError( "Internal error (tok_type_as_sysapp): missing sysapp" ).culprit( "Token type", tok_type_name( fnc ) );
	}
}

const char *tok_type_as_type( TokType fnc ) {
    switch ( fnc ) {
	case tokty_absent : return "absent";
	case tokty_termin : return "termin";
	case tokty_bool : return "bool";
	case tokty_int : return "int";
	case tokty_double : return "double";
	case tokty_char : return "char";
	case tokty_string : return "string";
	case tokty_symbol: return "symbol";
	case tokty_indeterminate : return "indeterminate";
	case tokty_undefined : return "undefined";
	default: throw CompileTimeError( "Internal error (tok_type_as_type): Unreachable" );
	}
}
