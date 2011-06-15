/******************************************************************************\
	Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
	of the GNU General Public License. This file is part of AppGinger.

    AppGinger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AppGinger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AppGinger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

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

