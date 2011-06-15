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
		case tokty_charseq: return "charseq";
		case tokty_symbol: return "symbol";
		default: return "?";
    }
    throw "Unreachable";
}
