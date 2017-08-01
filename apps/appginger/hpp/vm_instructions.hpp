

namespace Ginger {

	static const char * VM_ENTER = "enter";
	static const char * VM_RETURN = "return";
	static const char * VM_RETURN_IFSO = "return.ifso";
	static const char * VM_RETURN_IFNOT = "return.ifnot";

	static const char * VM_POP_LOCAL = "pop.local";
	static const char * VM_POP_LOCAL_LOCAL = "local";
	static const char * VM_PUSH_LOCAL = "push.local";
	static const char * VM_PUSH_LOCAL_LOCAL = "local";
	static const char * VM_PUSH_LOCAL_RET = "push.local.ret";
	static const char * VM_PUSH_LOCAL_RET_LOCAL = "local";

	static const char * VM_CALLS = "calls";
	static const char * VM_SELF_CALL = "self.call";
	static const char * VM_SELF_CONSTANT = "self.constant";

	static const char * VM_FAIL = "fail";

	static const char * VM_ADD = "add";
	static const char * VM_MUL = "mul";
	static const char * VM_SUB = "sub";
	static const char * VM_DIV = "div";
	static const char * VM_QUO = "quo";
	static const char * VM_REM = "rem";
	static const char * VM_NEG = "neg";
	static const char * VM_POS = "pos";
	static const char * VM_NOT = "not";

	static const char * VM_LT = "lt";
	static const char * VM_LTE = "lte";
	static const char * VM_GT = "gt";
	static const char * VM_GTE = "gte";
	static const char * VM_EQ = "eq";
	static const char * VM_NEQ = "neq";

	static const char * VM_DUP = "dup";
	static const char * VM_ERASE = "erase";
	static const char * VM_INCR = "incr";
	static const char * VM_DECR = "decr";

	static const char * VM_START_MARK = "start.mark";
	static const char * VM_START_MARK_LOCAL = "local";
	static const char * VM_END_MARK = "end.mark";
	static const char * VM_END_MARK_LOCAL = "local";
	static const char * VM_SET_COUNT_MARK = "set.count.mark";
	static const char * VM_SET_COUNT_MARK_LOCAL = "local";
	static const char * VM_ERASE_MARK = "erase.mark";
	static const char * VM_ERASE_MARK_LOCAL = "local";
	static const char * VM_CHECK_MARK_GTE = "check.mark.gte";
	static const char * VM_CHECK_MARK_GTE_LOCAL = "local";
	static const char * VM_CHECK_MARK_GTE_COUNT = "count";
	static const char * VM_CHECK_MARK = "check.mark";
	static const char * VM_CHECK_MARK_LOCAL = "local";
	static const char * VM_CHECK_MARK_COUNT = "count";
	static const char * VM_CHECK_MARK0 = "check.mark0";
	static const char * VM_CHECK_MARK0_LOCAL = "local";
	static const char * VM_CHECK_MARK1 = "check.mark1";
	static const char * VM_CHECK_MARK1_LOCAL = "local";

	static const char * VM_GOTO = "goto";
	static const char * VM_GOTO_TO = "to";
	static const char * VM_BYPASS = "bypass";
	static const char * VM_BYPASS_TO = "to";
	static const char * VM_IFNOT = "ifnot";
	static const char * VM_IFNOT_TO = "to";
	static const char * VM_IFSO = "ifso";
	static const char * VM_IFSO_TO = "to";

	static const char * VM_SYSAPP = "sysapp";
	static const char * VM_SYSAPP_NAME = "name";

	static const char * VM_SET_COUNT_SYSAPP = "set.count.sysapp";
	static const char * VM_SET_COUNT_SYSAPP_NAME = "name";
	static const char * VM_SET_COUNT_SYSAPP_COUNT = "count";

}
