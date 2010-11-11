#include <map>
#include <string>

#include "item_map.hpp"

#include "functor.hpp"
#include "item.hpp"
#include "key.hpp"
//#include "special.hpp"
#include "role.hpp"
//  Special constants to help with regular text layout.
#define Clo CloserRole
#define PrF PrefixRole
#define PoF PostfixRole
#define BoF BothfixRole
#define Imm ImmediateRole
#define POp PrefixOpRole
#define BOp BinaryOpRole
#define SOp SuffixOpRole
#define InL InLineRole

static Item the_item_default = 
	new ItemClass(        
		"<default>",       //  name
        fnc___fnc_default,   //  functor
        PrefixRole,
        0,
        0
 	)
 ;

void ItemMap::add( const char *n, Functor f, Role r, int p  ) {
	this->table[ n ] = new ItemClass( n, f, r, p, 0 );
}
	
	
ItemMap::ItemMap() {
	add( "!!", 			fnc_absnot, 	POp, 	prec_not	);
	add( "&&", 			fnc_absand, 	BOp, 	1700		);
	add( "(", 			fnc_oparen, 	BoF, 	100			);
	add( ")", 			fnc_cparen, 	Clo, 	0			);
	add( "*", 			fnc_mul, 		BOp, 	400			);
	add( "**", 			fnc_pow, 		BOp, 	300			);
	add( "+", 			fnc_add, 		BOp, 	600			);
	add( "++", 			fnc_append, 	BOp, 	800			);
	add( ",", 			fnc_comma, 		BOp, 	prec_comma	);
	add( "-", 			fnc_sub, 		BOp, 	700			);
	add( ".", 			fnc_dot, 		PoF, 	200			);
	add( "/", 			fnc_div, 		BOp, 	500			);
	add( ":=", 			fnc_assign, 	BOp, 	2000		);
	add( ";", 			fnc_semi, 		PoF, 	prec_semi	);
	add( "<", 			fnc_lt, 		BOp, 	1200		);
	add( "<=", 			fnc_lte, 		BOp, 	1400		);
	add( "=", 			fnc_eq, 		BOp, 	1100		);
	add( "==", 			fnc_eq, 		BOp, 	1000		);
	add( "=>", 			fnc_arrow, 		BOp, 	prec_arrow	);
	add( ">", 			fnc_gt, 		BOp, 	1300		);
	add( ">=", 			fnc_gte, 		BOp, 	1500		);
	add( ">->",			fnc_syscall,	PrF,	0			);
	add( "??", 			fnc_present, 	PoF, 	1900		);
	add( "@", 			fnc_at, 		PoF, 	900			);
	add( "[", 			fnc_obracket, 	PrF, 	100			);
	add( "]", 			fnc_cbracket, 	Clo, 	0			);
	add( "_print", 		fnc__print, 	InL, 	0			);
	add( "_putchar", 	fnc__putchar,	InL, 	0			);
	add( "absent", 		fnc_absent, 	Imm, 	0			);
	add( "and", 		fnc_and, 		BOp, 	1700		);
	add( "const", 		fnc_const, 		PrF, 	0			);
	add( "define", 		fnc_define, 	PrF, 	0			);
	add( "do", 			fnc_do, 		PrF, 	0			);
	add( "else", 		fnc_else, 		PrF, 	0			);
	add( "elseif", 		fnc_elseif, 	PrF, 	0			);
	add( "elseunless", 	fnc_elseunless, PrF, 	0			);
	add( "enddefine", 	fnc_enddefine, 	PrF, 	0			);
	add( "endfn", 		fnc_endfn, 		PrF, 	0			);
	add( "endfor", 		fnc_endfor, 	PrF, 	0			);
	add( "endif", 		fnc_endif, 		PrF, 	0			);
	add( "endunless", 	fnc_endunless, 	PrF, 	0			);
	add( "false", 		fnc_false, 		Imm, 	0			);
	add( "fn", 			fnc_fn, 		PrF, 	0			);
	add( "for", 		fnc_for, 		PrF, 	0			);
	add( "if", 			fnc_if, 		PrF, 	0			);
	add( "not", 		fnc_not, 		POp, 	prec_not	);
	add( "or", 			fnc_or, 		BOp, 	1800		);
	add( "then", 		fnc_then, 		PrF, 	0			);
	add( "true",	 	fnc_true, 		Imm, 	0			);
	add( "unless", 		fnc_unless, 	PrF, 	0			);
	add( "until", 		fnc_until, 		PrF, 	0			);
	add( "var", 		fnc_var, 		PrF, 	0			);
	add( "while", 		fnc_while, 		PrF, 	0			);
	add( "{", 			fnc_obrace, 	PrF, 	100			);
	add( "||", 			fnc_absor, 		BOp, 	1800		);
	add( "}", 			fnc_cbrace, 	PoF, 	0			);
}
	
ItemClass * ItemMap::lookup( const char * name ) {
	std::map< std::string, ItemClass * >::iterator it = this->table.find( name );
	return it == this->table.end() ? NULL : it->second;
}

ItemClass * ItemMap::lookup( const std::string &name ) {
	std::map< std::string, ItemClass * >::iterator it = this->table.find( name );
	return it == this->table.end() ? NULL : it->second;
}

ItemMap itemMap;