#include <map>
#include <string>

#include "item_map.hpp"

#include "functor.hpp"
#include "item.hpp"
#include "role.hpp"

//  Special constants to help with regular text layout.
#define Pat PatternRole
#define Clo CloserRole
#define PrF PrefixRole
#define PoF PostfixRole
#define BoF BothfixRole
#define Lit LiteralRole
#define USA UnarySysRole
#define BFo BinaryFormRole
#define BSA BinarySysRole
#define SOp SuffixOpRole
#define InL InLineRole

void ItemMap::add( const char *n, Functor f, Role r, int p  ) {
	this->table[ n ] = new ItemClass( n, f, r, p );
}
	
	
ItemMap::ItemMap() {
	add( "!!", 			fnc_absnot, 	USA, 		prec_not	);
	add( "&&", 			fnc_absand, 	BSA, 		1700		);
	add( "(", 			fnc_oparen, 	BoF, 		100			);
	add( ")", 			fnc_cparen, 	Clo, 		0			);
	add( "*", 			fnc_mul, 		BSA, 		400			);
	add( "**", 			fnc_pow, 		BSA, 		300			);
	add( "+", 			fnc_add, 		BSA,		600			);
	add( "++", 			fnc_append, 	BSA, 		800			);
	add( ",", 			fnc_comma, 		BFo, 		prec_comma	);
	add( "-", 			fnc_sub, 		BSA, 		700			);
	add( ".", 			fnc_dot, 		PoF, 		200			);
	add( "/", 			fnc_div, 		BSA, 		500			);
	add( ":=", 			fnc_bind, 		Pat, 		2000		);
	add( "->",			fnc_assign,		BFo,		2000		);
	add( ";", 			fnc_semi, 		PoF, 		prec_semi	);
	add( "<", 			fnc_lt, 		BSA, 			1200		);
	add( "<=", 			fnc_lte, 		BSA, 		1400		);
	add( "=", 			fnc_equal, 		BSA, 		1100		);
	add( "==", 			fnc_identical, 	BSA, 		1000		);
	add( "=>", 			fnc_arrow, 		BSA, 		prec_arrow	);
	add( ">", 			fnc_gt, 		BSA, 		1300		);
	add( ">=", 			fnc_gte, 		BSA, 		1500		);
	add( ">->",			fnc_syscall,	PrF,		0			);
	add( "??", 			fnc_present, 	PoF, 		1900		);
	add( "@", 			fnc_at, 		PoF, 		900			);
	add( "[", 			fnc_obracket, 	PrF, 		100			);
	add( "]", 			fnc_cbracket, 	Clo, 		0			);
	add( "absent", 		fnc_absent, 	Lit, 		0			);
	add( "and", 		fnc_and, 		BFo, 		1700		);
	add( "const", 		fnc_const, 		PrF, 		0			);
	add( "define", 		fnc_define, 	PrF, 		0			);
	add( "do", 			fnc_do, 		PrF, 		0			);
	add( "else", 		fnc_else, 		PrF, 		0			);
	add( "elseif", 		fnc_elseif, 	PrF, 		0			);
	add( "elseunless", 	fnc_elseunless, PrF, 		0			);
	add( "enddefine", 	fnc_enddefine, 	PrF, 		0			);
	add( "endfn", 		fnc_endfn, 		PrF, 		0			);
	add( "endfor", 		fnc_endfor, 	PrF, 		0			);
	add( "endif", 		fnc_endif, 		PrF, 		0			);
	add( "endunless", 	fnc_endunless, 	PrF, 		0			);
	add( "false", 		fnc_bool, 		Lit, 		0			);
	add( "fn", 			fnc_fn, 		PrF, 		0			);
	add( "for", 		fnc_for, 		PrF, 		0			);
	add( "from",		fnc_from, 		Pat,		2000		);
	add( "if", 			fnc_if, 		PrF, 		0			);
	add( "not", 		fnc_not, 		USA, 		0			);
	add( "or", 			fnc_or, 		BFo, 		1800		);
	add( "present",		fnc_present,	Lit,		0			);
	add( "then", 		fnc_then, 		PrF, 		0			);
	add( "true",	 	fnc_bool, 		Lit, 		0			);
	add( "to",			fnc_to, 		PrF,		0			);
	add( "unless", 		fnc_unless, 	PrF, 		0			);
	add( "until", 		fnc_until, 		PrF, 		0			);
	add( "var", 		fnc_var, 		PrF, 		0			);
	add( "while", 		fnc_while, 		PrF, 		0			);
	add( "{", 			fnc_obrace, 	PrF, 		100			);
	add( "||", 			fnc_absor, 		BFo, 		1800		);
	add( "}", 			fnc_cbrace, 	PoF, 		0			);
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