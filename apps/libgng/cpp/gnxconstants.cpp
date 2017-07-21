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

#include "gnxconstants.hpp"

namespace Ginger {
    const char * GNX_SPAN = "span";
    const char * GNX_COMMENT = "comment";

    //  deprecate ARITY in favour of EVAL_ARITY;
    const char * GNX_ARITY = "arity.eval";
    const char * GNX_EVAL_ARITY = "arity.eval";
    const char * GNX_PATTERN_ARITY = "arity.pattern";
    const char * GNX_ANALYSIS_TYPE = "arity.analysis.type";
    const char * GNX_ARGS_ARITY = "args.arity";
    const char * GNX_TAILCALL = "is.tail.call";
    const char * GNX_UID = "uid";
    const char * GNX_OUTER_LEVEL = "outer.level";
    const char * GNX_IS_OUTER = "is.outer";
    const char * GNX_SCOPE = "scope";
    const char * GNX_PROTECTED = "protected";
    const char * GNX_IS_TEMPORARY = "is.temporary";
    const char * GNX_IS_ASSIGNED = "is.assigned";
 
    const char * GNX_CONSTANT = "constant"; 
    const char * GNX_CONSTANT_TYPE = "type";
    const char * GNX_CONSTANT_VALUE = "value";

    const char * GNX_ID = "id";
    const char * GNX_VAR = "var";

    const char * GNX_VID_NAME = "name";
    const char * GNX_VID_DEF_PKG = "def.pkg";
    const char * GNX_VID_ENC_PKG = "enc.pkg";
    const char * GNX_VID_SCOPE = "scope";
    const char * GNX_VID_PROTECTED = "protected";
    const char * GNX_VID_SLOT = "slot";
    const char * GNX_VID_ALIAS = "alias";

    const char * GNX_MAKEREF = "makeref";
    const char * GNX_DEREF = "deref";
    const char * GNX_SETCONT = "setcont";

    const char * GNX_BLOCK = "block";
    const char * GNX_ABSAND = "absand";
    const char * GNX_ABSOR = "absor";
    const char * GNX_AND = "and";
    const char * GNX_BIND = "bind";
    const char * GNX_CROSS = "cross";
    const char * GNX_DO = "do";
    const char * GNX_ERASE = "erase";
    const char * GNX_FINALLY = "finally";
    const char * GNX_FOR = "for";
    const char * GNX_FROM = "from";
    const char * GNX_IF = "if";
    const char * GNX_IN = "in";
    const char * GNX_OK = "ok";
    const char * GNX_OR = "or";
    const char * GNX_SEQ = "seq";
    const char * GNX_SET = "set";
    const char * GNX_SWITCH = "switch";
    const char * GNX_WHERE = "where";
    const char * GNX_WHILE = "while";
    const char * GNX_ZIP = "zip";

    const char * GNX_LIST = "list";
    const char * GNX_LIST_APPEND = "list.append";
    const char * GNX_VECTOR = "vector";

    const char * GNX_ASSERT = "assert";
    const char * GNX_ASSERT_N = "n";
    const char * GNX_ASSERT_TYPE = "type";
    const char * GNX_ASSERT_TAILCALL = "tailcall";

    const char * GNX_APP = "app";

    const char * GNX_SYSFN = "sysfn";
    const char * GNX_SYSFN_VALUE = "value";

    const char * GNX_SYSAPP = "sysapp";
    const char * GNX_SYSAPP_NAME = "name";

    const char * GNX_SELF_APP = "self.app";

    const char * GNX_FN = "fn";
    const char * GNX_FN_NAME = "name";
    const char * GNX_FN_ARGS_COUNT = "args.count";
    const char * GNX_FN_LOCALS_COUNT = "locals.count";
    const char * GNX_SELF_CONSTANT = "self.constant";

    const char * GNX_FN_CODE = "fn.code";
    const char * GNX_FN_CODE_NAME = "name";
    const char * GNX_FN_CODE_ARGS_COUNT = "args.count";
    const char * GNX_FN_CODE_LOCALS_COUNT = "locals.count";

    const char * GNX_THROW = "throw";
    const char * GNX_THROW_EVENT = "event";
    const char * GNX_THROW_LEVEL = "level";
    const char * GNX_TRY = "try";
    const char * GNX_CATCH_THEN = "catch.then";
    const char * GNX_CATCH_THEN_EVENT = "event";
    const char * GNX_CATCH_RETURN = "catch.return";
    const char * GNX_CATCH_ELSE = "catch.else";

    const char * GNX_PROBLEM = "problem";
    const char * GNX_PROBLEM_CATEGORY = "category";
    const char * GNX_PROBLEM_MESSAGE = "message";
    const char * GNX_CULPRIT = "culprit";
    const char * GNX_CULPRIT_NAME = "name";
    const char * GNX_CULPRIT_VALUE = "value";

    const char * GNX_EMPTY_FN_NAME = "";

}
