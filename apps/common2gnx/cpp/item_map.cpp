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

void ItemMap::add( const bool mode_check, const char *n, TokType f, Role r, int p  ) {
	if ( mode_check ) this->table[ n ] = new ItemClass( n, f, r, p );
}

//	Declare syntactic properties.
ItemMap::ItemMap( const bool cstyle ) {
	add( true,	 	"$",			tokty_envvar,       PrF,        0           );	// changed for ${VAR} case study. 
	add( true,	 	"-",			tokty_sub, 			BSA, 		700			);
	add( true,	 	">->",			tokty_assign,		BFo,		2000		);
	add( true,	 	",", 			tokty_comma, 		BFo, 		prec_comma	);
	add( true,	 	";", 			tokty_semi, 		PoF, 		prec_semi	);
	add( true,	 	":-", 			tokty_maplet, 		BSA, 		2000		);
	add( true,	 	":=", 			tokty_bind, 		Pat, 		2000		);
	add( true,	 	"=:", 			tokty_bindrev, 		PoF, 		2000		);
	add( true,		"!",			tokty_tag,			Clo,		0			);
	add( true,	 	"!!", 			tokty_absnot, 		USA, 		prec_not	);
	add( true,		"?!",			tokty_trap,			PoF,		250			);
	add( true,	 	"??", 			tokty_present, 		PoF, 		1900		);
	add( true,	 	"...",			tokty_explode,  	SOp,        250			);
	add( true,	 	".", 			tokty_dot, 			PoF, 		200			);
	add( true,	 	"(", 			tokty_oparen, 		BoF, 		100			);
	add( true,	 	"(%", 			tokty_fat_oparen, 	BoF, 		100			);
	add( true,	 	")", 			tokty_cparen, 		Clo, 		0			);
	add( true,	 	"[", 			tokty_obracket, 	PrF, 		100			);
	add( true,	 	"[%", 			tokty_fat_obracket, PrF, 		100			);
	add( true,	 	"]", 			tokty_cbracket, 	Clo, 		0			);
	add( true,	 	"{", 			tokty_obrace, 		PrF, 		100			);
	add( true,	 	"{%", 			tokty_fat_obrace, 	PrF, 		100			);
	add( true,	 	"}", 			tokty_cbrace, 		Clo, 		0			);
	add( true,		"@", 			tokty_at, 			PoF, 		900			);
	add( true,		"*", 			tokty_mul, 			BSA, 		400			);
	add( true,		"**", 			tokty_pow, 			BSA, 		300			);
	add( true,		"/", 			tokty_div, 			BSA, 		500			);
	add( true,		"&&", 			tokty_absand, 		BSA, 		1700		);
	add( true,		"%)", 			tokty_fat_cparen, 	Clo, 		0			);
	add( true,		"%]", 			tokty_fat_cbracket, Clo, 		0			);
	add( true,		"%}", 			tokty_fat_cbrace, 	Clo, 		0			);
	add( true,		"+", 			tokty_add, 			BSA,		600			);
	add( true,		"++", 			tokty_append, 		BSA, 		800			);
	add( true,		"<>", 			tokty_append, 		BSA, 		800			);
	add( true,		"<", 			tokty_lt, 			BSA, 		1200		);
	add( true,		"<=", 			tokty_lte, 			BSA, 		1400		);
	add( true,		"=", 			tokty_equal, 		BSA, 		1100		);
	add( true,		"==", 			tokty_identical, 	BSA, 		1000		);
	add( true,		"=>", 			tokty_arrow, 		BSA, 		prec_arrow	);
	add( true,		"->", 			tokty_maplet, 		BSA, 		prec_arrow	);
	add( true,		">->",			tokty_syscall,		PrF,		0			);
	add( true,		">", 			tokty_gt, 			BSA, 		1300		);
	add( true,		">=", 			tokty_gte, 			BSA, 		1500		);
	add( true,		"</", 			tokty_ltslash,		Clo,		0			);
	add( true,		"/>",			tokty_slashgt,		Clo,		0			);
	add( true,		"|",			tokty_bar,			Clo,		0			);
	add( true,		"||",			tokty_absor, 		BFo, 		1800		);
	add( true,		"and",			tokty_and, 			BFo, 		1700		);
	add( true,		"catch",		tokty_catch, 		Clo, 		0			);
	add( true,		"catchelse",	tokty_catchelse, 	Clo, 		0			);
	add( !cstyle,	"define",		tokty_define, 		PrF, 		0			);
	add( !cstyle,	"do",			tokty_do, 			Clo, 		0			);
	add( true,	 	"else",			tokty_else, 		Clo, 		0			);
	add( !cstyle,	"elseif",		tokty_elseif, 		Clo, 		0			);
	add( !cstyle,	"elseunless",	tokty_elseunless, 	Clo, 		0			);
	add( !cstyle,	"enddefine",	tokty_enddefine, 	Clo, 		0			);
	add( !cstyle,	"endfn",		tokty_endfn, 		Clo, 		0			);
	add( !cstyle,	"endfor",		tokty_endfor, 		Clo, 		0			);
	add( !cstyle,	"endif",		tokty_endif, 		Clo, 		0			);
	add( !cstyle,	"endpackage",	tokty_endpackage, 	Clo, 		0			);
	add( !cstyle,	"endunless",	tokty_endunless, 	Clo, 		0			);
	add( !cstyle,	"fn",			tokty_fn,			PrF, 		0			);
	add( true,	 	"for",			tokty_for,			PrF, 		0			);
	add( true,	 	"from",			tokty_from,			Pat,		2000		);
	add( cstyle,	"function",		tokty_function,		PrF, 		0			);
	add( true,	 	"if",			tokty_if,			PrF, 		0			);
	add( true,	 	"import",		tokty_import,		PrF,		0			);
	add( true,	 	"in",			tokty_in,			Pat,		2000		);
	add( true,	 	"or",			tokty_or,			BFo, 		1800		);
	add( true,	 	"package",		tokty_package,		PrF, 		0			);
	add( true,	 	"return",		tokty_return,		PrF, 		0			);
	add( !cstyle,	"then",			tokty_then,			Clo, 		0			);
	add( true,		"throw",		tokty_throw,		PrF,		0			);
	add( true,		"to",			tokty_to,			PrF,		0			);
	add( true,		"unless",		tokty_unless,		PrF, 		0			);
	add( true,		"until", 		tokty_until,		PrF, 		0			);
	add( true,		"val",			tokty_val,			PrF, 		0			);
	add( true,	 	"var",			tokty_var,			PrF,		0			);
	add( true,		"while",		tokty_while,		PrF,		0			);
	add( true,		"with",			tokty_with,			Clo, 		0			);
}

ItemClass * ItemMap::lookup( const char * name ) {
	std::map< std::string, ItemClass * >::iterator it = this->table.find( name );
	return it == this->table.end() ? NULL : it->second;
}

ItemClass * ItemMap::lookup( const std::string &name ) {
	std::map< std::string, ItemClass * >::iterator it = this->table.find( name );
	return it == this->table.end() ? NULL : it->second;
}
