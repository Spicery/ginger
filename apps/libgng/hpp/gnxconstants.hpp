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

#ifndef GNX_CONSTANTS_HPP
#define GNX_CONSTANTS_HPP

#define SPAN            "span"
#define COMMENT         "comment"

#define ARITY 			"arity"
#define ARGS_ARITY		"args.arity"
#define TAILCALL		"is.tail.call"
#define UID				"uid"
#define OUTER_LEVEL		"outer.level"
#define IS_OUTER		"is.outer"
#define SCOPE			"scope"
#define PROTECTED		"protected"
#define IS_ASSIGNED		"is.assigned"

#define CONSTANT		"constant"
#define CONSTANT_TYPE	"type"
#define CONSTANT_VALUE	"value"

#define	ID				"id"
#define VAR				"var"

#define VID_NAME		"name"
#define VID_DEF_PKG		"def.pkg"
#define VID_SCOPE		"scope"
#define VID_PROTECTED	"protected"
#define VID_SLOT		"slot"

#define MAKEREF			"makeref"
#define DEREF			"deref"
#define SETCONT			"setcont"

#define BLOCK           "block"
#define ABSAND          "absand"
#define ABSOR           "absor"
#define AND             "and"
#define BIND            "bind"
#define CROSS           "cross"
#define DO              "do"
#define ERASE           "erase"
#define FINALLY         "finally"
#define FOR             "for"
#define FROM            "from"
#define IF              "if"
#define IN              "in"
#define OK              "ok"
#define OR              "or"
#define SEQ             "seq"
#define SET             "set"
#define SWITCH          "switch"
#define WHERE           "where"
#define WHILE           "while"
#define ZIP             "zip"

#define LIST			"list"
#define LIST_APPEND		"list.append"
#define VECTOR			"vector"

#define ASSERT			"assert"
#define ASSERT_N		"n"
#define ASSERT_TYPE		"type"
#define ASSERT_TAILCALL "tailcall"

#define APP				"app"

#define SYSFN			"sysfn"
#define SYSFN_VALUE		"value"

#define SYSAPP			"sysapp"
#define SYSAPP_NAME		"name"

#define SELF_APP		"self.app"

#define	FN				"fn"
#define FN_NAME			"name"
#define FN_ARGS_COUNT	"args.count"
#define FN_LOCALS_COUNT	"locals.count"
#define SELF_CONSTANT	"self.constant"

#define THROW			"throw"
#define THROW_EVENT     "event"
#define THROW_LEVEL     "level"
#define TRY             "try"

#define PROBLEM			"problem"
#define PROBLEM_SEVERITY		"severity"
#define PROBLEM_MESSAGE	"message"
#define CULPRIT			"culprit"
#define CULPRIT_NAME	"name"
#define CULPRIT_VALUE	"value"

#define EMPTY_FN_NAME   ""

#endif
