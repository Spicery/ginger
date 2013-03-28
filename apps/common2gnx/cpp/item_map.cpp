#include <map>
#include <string>

#include <stddef.h>

#include "item_map.hpp"

#include "toktype.hpp"
#include "item.hpp"
#include "role.hpp"
#include "keywords.hpp"

//  Special constants to help with regular text layout.
#define Clo CloserRole
#define PrF PrefixRole
#define PoF PostfixRole
#define BoF BothfixRole
#define Lit LiteralRole
#define USy UnarySysRole
#define BFo BinaryFormRole
#define BSy BinarySysRole
#define SSA SuffixSysRole
#define InL InLineRole

void ItemMap::add( const bool mode_check, const char *n, TokType f, Role r, int p  ) {
	if ( mode_check ) this->table[ n ] = new ItemClass( n, f, r, p );
}

//	Declare syntactic properties.
ItemMap::ItemMap( const bool cstyle ) {
	add( true,	 	KW_DOLLAR,			tokty_envvar,       PrF,        0       );	// changed for ${VAR} case study. 
	add( true,	 	KW_SUB,				tokty_sub, 			BSy, 		prec_arith_sub		);
	add( true,	 	KW_ARROW_RIGHT,		tokty_assign,		BFo,		prec_assign	);
	add( true,	 	KW_ARROW_LEFT,		tokty_assignrev,	BFo,		prec_assign	);
	add( true,	 	KW_SET_RIGHT,		tokty_assign,		BFo,		prec_assign	);
	add( true,	 	KW_SET_LEFT,		tokty_assignrev,	BFo,		prec_assign	);
	add( true,	 	KW_COMMA, 			tokty_comma, 		BFo, 		prec_comma	);
	add( cstyle,	KW_COLON,			tokty_colon,		Clo,		0 		);
	add( !cstyle, 	KW_SEMICOLON, 		tokty_semi, 		PoF, 		prec_semi	);
	add( cstyle, 	KW_SEMICOLON, 		tokty_semi, 		Clo, 		0		);
	add( !cstyle, 	KW_2_SEMICOLON,		tokty_dsemi,		PoF,		prec_semi	);
	add( true,	 	KW_MAPLET_THIN, 	tokty_maplet, 		BSy, 		prec_arrow	);
	add( true,	 	KW_BIND_LEFT, 		tokty_bind, 		PoF, 		prec_assign	);
	add( true,	 	KW_BIND_RIGHT, 		tokty_bindrev, 		PoF, 		prec_assign	);
	add( true,      KW_BANG,            tokty_bang,         USy,        prec_bang    );
	add( true,	 	KW_2_BANG, 			tokty_dbang, 		SSA, 		prec_not	);
	add( true,		KW_TRAP,			tokty_trap,			PoF,		prec_trap		);
	add( true,	 	KW_ISNT_ABSENT, 	tokty_isntabsent, 	SSA, 		prec_isnt_absent	);
	add( true,	 	KW_EXPLODE,			tokty_explode,  	SSA,        prec_explode		);
	add( true,      KW_2_HAT,           tokty_dhat,         PrF,        prec_dhat     );
	add( true,	 	KW_INFIX_APPLY_1, 	tokty_dot, 			PoF, 		prec_apply_tight		);
	add( true,	 	KW_PAREN_OPEN, 		tokty_oparen, 		BoF, 		prec_tight		);
	add( true,	 	KW_FAT_PAREN_OPEN, 	tokty_fat_oparen, 	BoF, 		prec_tight		);
	add( true,	 	KW_PAREN_CLOSE, 	tokty_cparen, 		Clo, 		0		);
	add( true,	 	KW_LIST_OPEN, 		tokty_obracket, 	BoF, 		prec_tight		);
	add( true,	 	KW_FAT_BRACKET_OPEN, tokty_fat_obracket, PrF, 		prec_tight		);
	add( true,	 	KW_LIST_CLOSE, 		tokty_cbracket, 	Clo, 		0		);
	add( !cstyle, 	KW_MAP_OPEN, 		tokty_obrace, 		PrF, 		prec_tight		);
	add( cstyle, 	KW_BRACE_OPEN, 		tokty_obrace, 		Clo, 		prec_tight		);
	add( true,	 	KW_FAT_BRACE_OPEN, 	tokty_fat_obrace, 	PrF, 		prec_tight		);
	add( !cstyle,	KW_MAP_CLOSE, 		tokty_cbrace, 		Clo, 		0		);
	add( cstyle,	KW_BRACE_CLOSE, 	tokty_cbrace, 		Clo, 		0		);
	add( true,		KW_INFIX_APPLY_2, 	tokty_at, 			PoF, 		prec_apply_loose		);
	add( true,		KW_MUL, 			tokty_mul, 			BSy, 		prec_arith_mul		);
	add( true,		KW_POW, 			tokty_pow, 			BSy, 		prec_pow		);
	add( true,		KW_DIV, 			tokty_div, 			BSy, 		prec_div		);
	add( true,		KW_ABS_AND, 		tokty_absand, 		BFo, 		prec_abs_and	);
	add( true,		KW_FAT_PAREN_CLOSE, tokty_fat_cparen, 	Clo, 		0		);
	add( true,		KW_FAT_BRACKET_CLOSE, tokty_fat_cbracket, Clo, 		0		);
	add( true,		KW_FAT_BRACE_CLOSE, tokty_fat_cbrace, 	Clo, 		0		);
	add( true,		KW_ADD, 			tokty_add, 			BSy,		prec_arith_add		);
	add( true,		KW_APPEND, 			tokty_append, 		BSy, 		prec_append		);
	add( true,		KW_END_ELEMENT,		tokty_endelement,	Clo,		0		);
	add( true,		KW_APPEND_ALT, 		tokty_append, 		BSy, 		prec_append		);
	add( true,		KW_LT, 				tokty_lt, 			BSy, 		prec_lt	);
	add( true,		KW_LTE, 			tokty_lte, 			BSy, 		prec_lte	);
	add( !cstyle,	KW_EQUAL, 			tokty_equal, 		BSy, 		prec_equal	);
	add( cstyle,	KW_EQUAL, 			tokty_equal, 		BFo, 		prec_assign	);
	add( true,		KW_IDENTICAL, 		tokty_identical, 	BSy, 		prec_identical	);
	add( true,		KW_FN_ARROW, 		tokty_fnarrow, 		BSy, 		prec_arrow	);
	add( true,		KW_MAPLET, 			tokty_maplet, 		BSy, 		prec_arrow	);
	add( true,		KW_SYSCALL,			tokty_syscall,		PrF,		0		);		/// @todo remove??
	add( true,		KW_GT, 				tokty_gt, 			BSy, 		prec_gt	);
	add( true,		KW_GTE, 			tokty_gte, 			BSy, 		prec_gte	);
	add( true,		KW_LT_SLASH, 		tokty_ltslash,		Clo,		0		);
	add( true,		KW_SLASH_GT,		tokty_slashgt,		Clo,		0		);
	add( true,		KW_BAR,				tokty_bar,			Clo,		0		);
	add( true,		KW_ABS_OR,			tokty_absor, 		BFo, 		prec_abs_or	);
	add( true,		KW_ALSO,			tokty_also,			BFo,		prec_also	);
	add( true,		KW_AND,				tokty_and, 			BFo, 		prec_and	);
	add( !cstyle,	KW_BY,				tokty_by,			Clo, 		0		);
	add( true,		KW_CASE,			tokty_case,			Clo,		0		);
	add( true,		KW_CATCH,			tokty_catch, 		Clo, 		0		);
	add( cstyle,	KW_DEFAULT,			tokty_default, 		Clo, 		0		);
	add( !cstyle,	KW_DEFINE,			tokty_define, 		PrF, 		0		);
	add( !cstyle,	KW_DO,				tokty_do, 			Clo, 		0		);
	add( true,      KW_QUO,             tokty_quo,          BSy, 		prec_quo		);
	add( true,	 	KW_ELSE,			tokty_else, 		Clo, 		0		);
	add( !cstyle,	KW_ELSEIF,			tokty_elseif, 		Clo, 		0		);
	add( !cstyle,	KW_ELSEUNLESS,		tokty_elseunless, 	Clo, 		0		);
	add( !cstyle,	KW_ENDDEFINE,		tokty_enddefine, 	Clo, 		0		);
	add( !cstyle,	KW_ENDLAMBDA,			tokty_endfn, 		Clo, 		0		);
	add( !cstyle,	KW_ENDFOR,			tokty_endfor, 		Clo, 		0		);
	add( !cstyle,	KW_ENDIF,			tokty_endif, 		Clo, 		0		);
	add( !cstyle,	KW_ENDPACKAGE,		tokty_endpackage, 	Clo, 		0		);
	add( true,		KW_ENDRECORDCLASS,	tokty_endrecordclass, 	Clo, 		0		);
	add( !cstyle,	KW_ENDSWITCH,		tokty_endswitch, 	Clo, 		0		);
	add( !cstyle,	KW_ENDTRANSACTION, 	tokty_endtransaction, Clo,		0		);
	add( !cstyle,	KW_ENDTRY,			tokty_endtry,		Clo,		0		);
	add( !cstyle,	KW_ENDUNLESS,		tokty_endunless, 	Clo, 		0		);
	//add( true,		"escape",		tokty_escape, 		PrF, 		0		);
	//add( true,		"!!",			tokty_failover,		PrF, 		0		);
	add( !cstyle,	KW_LAMBDA,			tokty_fn,			PrF, 		0		);
	add( true,	 	KW_FOR,				tokty_for,			PrF, 		0		);
	add( true,	 	KW_FROM,			tokty_from,			PoF,		prec_from	);
	add( cstyle,	KW_FUNCTION,		tokty_function,		PrF, 		0		);
	add( true,	 	KW_IF,				tokty_if,			PrF, 		0		);
	add( true,	 	KW_IMPORT,			tokty_import,		PrF,		0		);
	add( true,	 	KW_IN,				tokty_in,			PoF,		prec_in	);
	add( true,		KW_MOD,				tokty_mod,			BSy, 		prec_mod		);
	add( true,	 	KW_OR,				tokty_or,			BFo, 		prec_or	);
	add( true,	 	KW_PACKAGE,			tokty_package,		PrF, 		0		);
	add( true,		KW_PANIC,			tokty_panic, 		PrF, 		0		);
	add( true, 		KW_RECORDCLASS,		tokty_recordclass,	PrF,		0		);
	add( true,	 	KW_RETURN,			tokty_return,		PrF, 		0		);
	add( true,		KW_SLOT,			tokty_slot,			Clo,		0		);
	add( true,      KW_SWITCH,			tokty_switch,       PrF,        0       );
	add( !cstyle,	KW_THEN,			tokty_then,			Clo, 		0		);
	add( true,		KW_THROW,			tokty_throw,		PrF,		0		);
	add( true,		KW_TO,				tokty_to,			PrF,		0		);
	add( true,		KW_TRANSACTION,		tokty_transaction,	PrF,		0		);
	add( true,		KW_TRY,				tokty_try,			PrF,		0		);
	add( !cstyle,	KW_UNLESS,			tokty_unless,		PrF, 		0		);
	add( true,		KW_UNTIL, 			tokty_until,		PrF, 		0		);
	add( true,		KW_VAL,				tokty_val,			PrF, 		0		);
	add( true,	 	KW_VAR,				tokty_var,			PrF,		0		);
	add( true,		KW_WHILE,			tokty_while,		PrF,		0		);
	add( !cstyle,	KW_WITH,			tokty_with,			Clo, 		0		);
}

ItemClass * ItemMap::lookup( const char * name ) {
	std::map< std::string, ItemClass * >::iterator it = this->table.find( name );
	return it == this->table.end() ? NULL : it->second;
}

ItemClass * ItemMap::lookup( const std::string &name ) {
	std::map< std::string, ItemClass * >::iterator it = this->table.find( name );
	return it == this->table.end() ? NULL : it->second;
}
