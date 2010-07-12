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

#ifndef FUNCTOR_HPP
#define FUNCTOR_HPP

#include "instruction.hpp"
#include "common.hpp"

typedef enum Functor {
	fnc___fnc_default,
	fnc_eof,
	fnc_absand,
	fnc_absent,
	fnc_absnot,
	fnc_absor,
	fnc_add,
	fnc_and,
	fnc_anon,
	fnc_app,
	fnc_append,
	fnc_args,
	fnc_arrow,
	fnc_assign,
	fnc_bindings,
	fnc_bool,
	fnc_char,
	fnc_charseq,
	fnc_conditions,
	fnc_const,
	fnc_dec,
	fnc_decr,
	fnc_decr_by,
	fnc_define,
	fnc_div,
	fnc_eq,
	fnc_fn,
	fnc_for,
	fnc_from,
	fnc_gt,
	fnc_gte,
	fnc_id,
	fnc_if,
	fnc_incr,
	fnc_incr_by,
	fnc_int,
	fnc_list,
	fnc_lt,
	fnc_lte,
	fnc_mul,
	fnc_neq,
	fnc_not,
	fnc_or,
	fnc_pow,
	fnc_present,
	fnc_seq,
	fnc_string,
	fnc_sub,
	fnc_syscall,
	fnc_until,
	fnc_var,
	fnc_vector,
	fnc_while
} Functor;

const char *functor_name( Functor fnc );
Ref functor_value( enum Functor tag );


#endif
