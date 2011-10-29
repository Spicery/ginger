#include <map>
#include <string>

#include "item_map.hpp"

#include "toktype.hpp"
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

void ItemMap::add( const char *n, TokType f, Role r, int p  ) {
	this->table[ n ] = new ItemClass( n, f, r, p );
}
	
	
ItemMap::ItemMap() {
	//	add( "${",          tokty_envvar,       PrF,        0           );	//	declare syntactic properties.
	add( "-", 			tokty_sub, 			BSA, 		700			);
	add( "->",			tokty_assign,		BFo,		2000		);
	add( ",", 			tokty_comma, 		BFo, 		prec_comma	);
	add( ";", 			tokty_semi, 		PoF, 		prec_semi	);
	add( ":-", 			tokty_maplet, 		BSA, 		2000		);
	add( ":=", 			tokty_bind, 		Pat, 		2000		);
	add( "!!", 			tokty_absnot, 		USA, 		prec_not	);
	add( "??", 			tokty_present, 		PoF, 		1900		);
	add( "...",         tokty_explode,  	SOp,        250			);
	add( ".", 			tokty_dot, 			PoF, 		200			);
	add( "(", 			tokty_oparen, 		BoF, 		100			);
	add( "(%", 			tokty_fat_oparen, 	BoF, 		100			);
	add( ")", 			tokty_cparen, 		Clo, 		0			);
	add( "[", 			tokty_obracket, 	PrF, 		100			);
	add( "[%", 			tokty_fat_obracket, PrF, 		100			);
	add( "]", 			tokty_cbracket, 	Clo, 		0			);
	add( "{", 			tokty_obrace, 		PrF, 		100			);
	add( "{%", 			tokty_fat_obrace, 	PrF, 		100			);
	add( "}", 			tokty_cbrace, 		Clo, 		0			);
	add( "@", 			tokty_at, 			PoF, 		900			);
	add( "*", 			tokty_mul, 			BSA, 		400			);
	add( "**", 			tokty_pow, 			BSA, 		300			);
	add( "/", 			tokty_div, 			BSA, 		500			);
	add( "&&", 			tokty_absand, 		BSA, 		1700		);
	add( "%)", 			tokty_fat_cparen, 	Clo, 		0			);
	add( "%]", 			tokty_fat_cbracket, Clo, 		0			);
	add( "%}", 			tokty_fat_cbrace, 	Clo, 		0			);
	add( "+", 			tokty_add, 			BSA,		600			);
	add( "++", 			tokty_append, 		BSA, 		800			);
	add( "<", 			tokty_lt, 			BSA, 		1200		);
	add( "<=", 			tokty_lte, 			BSA, 		1400		);
	add( "=", 			tokty_equal, 		BSA, 		1100		);
	add( "==", 			tokty_identical, 	BSA, 		1000		);
	add( "=>", 			tokty_arrow, 		BSA, 		prec_arrow	);
	add( ">->",			tokty_syscall,		PrF,		0			);
	add( ">", 			tokty_gt, 			BSA, 		1300		);
	add( ">=", 			tokty_gte, 			BSA, 		1500		);
	add( "|",           tokty_bar,          Clo,        0           );
	add( "||", 			tokty_absor, 		BFo, 		1800		);
	add( "and", 		tokty_and, 			BFo, 		1700		);
	add( "define", 		tokty_define, 		PrF, 		0			);
	add( "do", 			tokty_do, 			PrF, 		0			);
	add( "else", 		tokty_else, 		PrF, 		0			);
	add( "elseif", 		tokty_elseif, 		PrF, 		0			);
	add( "elseunless", 	tokty_elseunless, 	PrF, 		0			);
	add( "enddefine", 	tokty_enddefine, 	PrF, 		0			);
	add( "endfn", 		tokty_endfn, 		PrF, 		0			);
	add( "endfor", 		tokty_endfor, 		Clo, 		0			);
	add( "endif", 		tokty_endif, 		PrF, 		0			);
	add( "endpackage", 	tokty_endpackage, 	PrF, 		0			);
	add( "endunless", 	tokty_endunless, 	PrF, 		0			);
	add( "fn", 			tokty_fn, 			PrF, 		0			);
	add( "for", 		tokty_for, 			PrF, 		0			);
	add( "from",		tokty_from, 		Pat,		2000		);
	add( "if", 			tokty_if, 			PrF, 		0			);
	add( "import",		tokty_import, 		PrF,		0			);
	add( "in",			tokty_in,    		Pat,		2000		);
	add( "or", 			tokty_or, 			BFo, 		1800		);
	add( "package", 	tokty_package, 		PrF, 		0			);
	add( "then", 		tokty_then, 		PrF, 		0			);
	add( "to",			tokty_to, 			PrF,		0			);
	add( "unless", 		tokty_unless, 		PrF, 		0			);
	add( "until", 		tokty_until, 		PrF, 		0			);
	add( "val", 		tokty_val, 			PrF, 		0			);
	add( "var", 		tokty_var, 			PrF, 		0			);
	add( "while", 		tokty_while, 		PrF, 		0			);
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
