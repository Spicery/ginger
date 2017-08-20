/******************************************************************************\
    Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
    of the GNU General Public License. This file is part of Ginger.

    Ginger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Ginger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Ginger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

#ifndef GNX_CONSTANTS_HPP
#define GNX_CONSTANTS_HPP

namespace Ginger {
    extern const char * GNX_SPAN;
    extern const char * GNX_COMMENT;

    //  deprecate ARITY in favour of EVAL_ARITY;
    extern const char * GNX_ARITY;
    extern const char * GNX_EVAL_ARITY;
    extern const char * GNX_PATTERN_ARITY;
    extern const char * GNX_ANALYSIS_TYPE;
    extern const char * GNX_ARGS_ARITY;
    extern const char * GNX_TAILCALL;
    extern const char * GNX_UID;
    extern const char * GNX_OUTER_LEVEL;
    extern const char * GNX_IS_OUTER;
    extern const char * GNX_SCOPE;
    extern const char * GNX_PROTECTED;
    extern const char * GNX_IS_TEMPORARY;
    extern const char * GNX_IS_ASSIGNED;
 
    extern const char * GNX_CONSTANT; 
    extern const char * GNX_CONSTANT_TYPE;
    extern const char * GNX_CONSTANT_VALUE;

    extern const char * GNX_ID;
    extern const char * GNX_VAR;

    extern const char * GNX_VID_NAME;
    extern const char * GNX_VID_DEF_PKG;
    extern const char * GNX_VID_ENC_PKG;
    extern const char * GNX_VID_SCOPE;
    extern const char * GNX_VID_PROTECTED;
    extern const char * GNX_VID_SLOT;
    extern const char * GNX_VID_ALIAS;

    extern const char * GNX_MAKEREF;
    extern const char * GNX_DEREF;
    extern const char * GNX_SETCONT;

    extern const char * GNX_BLOCK;
    extern const char * GNX_ABSAND;
    extern const char * GNX_ABSOR;
    extern const char * GNX_AND;
    extern const char * GNX_BIND;
    extern const char * GNX_CROSS;
    extern const char * GNX_DO;
    extern const char * GNX_ERASE;
    extern const char * GNX_FAIL;
    extern const char * GNX_FINALLY;
    extern const char * GNX_FOR;
    extern const char * GNX_FROM;
    extern const char * GNX_IF;
    extern const char * GNX_IN;
    extern const char * GNX_OK;
    extern const char * GNX_ONCE;
    extern const char * GNX_OR;
    extern const char * GNX_SEQ;
    extern const char * GNX_SET;
    extern const char * GNX_SWITCH;
    extern const char * GNX_WHERE;
    extern const char * GNX_WHILE;
    extern const char * GNX_ZIP;

    extern const char * GNX_LIST;
    extern const char * GNX_LIST_APPEND;
    extern const char * GNX_VECTOR;

    extern const char * GNX_ASSERT;
    extern const char * GNX_ASSERT_N;
    extern const char * GNX_ASSERT_TYPE;
    extern const char * GNX_ASSERT_TAILCALL;

    extern const char * GNX_APP;

    extern const char * GNX_SYSFN;
    extern const char * GNX_SYSFN_VALUE;

    extern const char * GNX_SYSAPP;
    extern const char * GNX_SYSAPP_NAME;

    extern const char * GNX_SELF_APP;

    extern const char * GNX_FN;
    extern const char * GNX_FN_NAME;
    extern const char * GNX_FN_ARGS_COUNT;
    extern const char * GNX_FN_LOCALS_COUNT;
    extern const char * GNX_SELF_CONSTANT;

    extern const char * GNX_FN_CODE;
    extern const char * GNX_FN_CODE_NAME;
    extern const char * GNX_FN_CODE_ARGS_COUNT;
    extern const char * GNX_FN_CODE_LOCALS_COUNT;
    extern const char * GNX_SELF_CONSTANT;

    extern const char * GNX_THROW;
    extern const char * GNX_THROW_EVENT;
    extern const char * GNX_THROW_LEVEL;
    extern const char * GNX_TRY;
    extern const char * GNX_CATCH_THEN;
    extern const char * GNX_CATCH_THEN_EVENT;
    extern const char * GNX_CATCH_RETURN;
    extern const char * GNX_CATCH_ELSE;

    extern const char * GNX_PROBLEM;
    extern const char * GNX_PROBLEM_CATEGORY;
    extern const char * GNX_PROBLEM_MESSAGE;
    extern const char * GNX_CULPRIT;
    extern const char * GNX_CULPRIT_NAME;
    extern const char * GNX_CULPRIT_VALUE;

    extern const char * GNX_EMPTY_FN_NAME;

}

#endif
