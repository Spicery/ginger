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

#include "functor.hpp"
#include "key.hpp"

const char * functor_name( Functor tag ) {
    switch ( tag ) {
	case fnc___fnc_default : return "__fnc_default";
	case fnc_eof : return "eof";
	case fnc_absand : return "absand";
	case fnc_absent : return "absent";
	case fnc_absnot : return "absnot";
	case fnc_absor : return "absor";
	case fnc_add : return "add";
	case fnc_and : return "and";
	case fnc_anon : return "anon";
	case fnc_app : return "app";
	case fnc_append : return "append";
	case fnc_args : return "args";
	case fnc_arrow : return "arrow";
	case fnc_assign : return "assign";
	case fnc_bindings : return "bindings";
	case fnc_bool: return "bool";
	case fnc_char : return "char";
	case fnc_charseq : return "charseq";
	case fnc_conditions : return "conditions";
	case fnc_const : return "const";
	case fnc_dec: return "dec";
	case fnc_decr : return "decr";
	case fnc_decr_by : return "decr_by";
	case fnc_define : return "define";
	case fnc_div : return "div";
	case fnc_eq : return "eq";
	case fnc_fn : return "fn";
	case fnc_for : return "for";
	case fnc_from : return "from";
	case fnc_gt : return "gt";
	case fnc_gte : return "gte";
	case fnc_id : return "id";
	case fnc_if : return "if";
	case fnc_incr : return "incr";
	case fnc_incr_by : return "incr_by";
	case fnc_int : return "int";
	case fnc_list: return "list";
	case fnc_lt : return "lt";
	case fnc_lte : return "lte";
	case fnc_mul : return "mul";
	case fnc_neq : return "neq";
	case fnc_not : return "not";
	case fnc_or : return "or";
	case fnc_pow : return "pow";
	case fnc_present : return "present";
	case fnc_seq : return "seq";
	case fnc_string : return "string";
	case fnc_sub : return "sub";
	case fnc_syscall : return "syscall";
	case fnc_until : return "until";
	case fnc_var : return "var";
	case fnc_vector : return "vector";
	case fnc_while : return "while";
    }
    return "?";
}

Ref functor_value( enum Functor tag ) {
	switch ( tag ) {
	case fnc_absent:	return sys_absent;
	//case fnc_false:		return sys_false;
	//case fnc_true:		return sys_true;
	default:			throw "No value";
	}
	
}
