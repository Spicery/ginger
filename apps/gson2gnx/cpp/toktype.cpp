#include "toktype.hpp"
#include "mishap.hpp"

const char *tok_type_name( TokType fnc ) {
    switch ( fnc ) {
		case tokty_eof: return "eof";
		case tokty_name: return "name";
		case tokty_sign: return "sign";
		/*case tokty_cbrace : return "cbrace";
		case tokty_cbracket : return "cbracket";
		case tokty_cparen : return "cparen";
		case tokty_obrace : return "obrace";
		case tokty_obracket : return "obracket";
		case tokty_oparen : return "oparen";
		case tokty_comma : return "comma";
		case tokty_equals : return "equals";*/
		case tokty_number : return "number";
		case tokty_string : return "string";
		default: return "?";
    }
    throw "Unreachable";
}
