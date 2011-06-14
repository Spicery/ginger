#ifndef TOKTYPE_HPP
#define TOKTYPE_HPP

typedef enum TokType {
	tokty_eof,
	tokty_sign,
	tokty_name,
//	tokty_cbrace,
//	tokty_cbracket,
//	tokty_cparen,
//	tokty_comma,
//	tokty_equals,
	tokty_number,
//	tokty_obrace,
//	tokty_obracket,
//	tokty_oparen,
	tokty_string,
	tokty_charseq,
	tokty_symbol
} TokType;

const char *tok_type_name( TokType fnc );

#endif

