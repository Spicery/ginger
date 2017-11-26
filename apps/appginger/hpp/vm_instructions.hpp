

namespace Ginger {

	static const char * VM_SEQ = "seq";

	static const char * VM_ENTER = "enter";
	static const char * VM_ENTER0 = "enter0";
	static const char * VM_ENTER1 = "enter1";
	static const char * VM_RETURN = "return";
	static const char * VM_RETURN_IFSO = "return.ifso";
	static const char * VM_RETURN_IFNOT = "return.ifnot";

	static const char * VM_PUSHQ = "pushq";
	static const char * VM_PUSHQ_POP_LOCAL = "pushq.pop.local";
	static const char * VM_PUSHQ_POP_LOCAL_LOCAL = "local";
	static const char * VM_PUSHQ_RET = "pushq.ret";

	static const char * VM_POP_LOCAL = "pop.local";
	static const char * VM_POP_LOCAL_LOCAL = "local";
	static const char * VM_PUSH_LOCAL = "push.local";
	static const char * VM_PUSH_LOCAL_LOCAL = "local";
	static const char * VM_PUSH_LOCAL0 = "push.local0";
	static const char * VM_PUSH_LOCAL1 = "push.local1";
	static const char * VM_PUSH_LOCAL_RET = "push.local.ret";
	static const char * VM_PUSH_LOCAL_RET_LOCAL = "local";
	static const char * VM_PUSH_LOCAL0_RET = "push.local.ret";
	static const char * VM_PUSH_LOCAL1_RET = "push.local.ret";

	static const char * VM_PUSH_GLOBAL = "push.global";
	static const char * VM_POP_GLOBAL = "pop.global";
	static const char * VM_END_CALL_GLOBAL = "end.call.global";
	static const char * VM_END_CALL_GLOBAL_LOCAL = "local";
	static const char * VM_SET_COUNT_CALL_GLOBAL = "set.count.call.global";
	static const char * VM_SET_COUNT_CALL_GLOBAL_COUNT = "count";

	static const char * VM_SET_COUNT_CALL_LOCAL = "set.count.call.local";
	static const char * VM_SET_COUNT_CALL_LOCAL_LOCAL = "local";
	static const char * VM_SET_COUNT_CALL_LOCAL_COUNT = "count";

	static const char * VM_CALLS = "calls";
	static const char * VM_SET_COUNT_CALLS = "set.count.calls";
	static const char * VM_SET_COUNT_CALLS_COUNT = "count";
	static const char * VM_SELF_CALL = "self.call";
	static const char * VM_SELF_CONSTANT = "self.constant";
	static const char * VM_SELF_CALL_N = "self.call.n";
	static const char * VM_SELF_CALL_N_COUNT = "count";
	static const char * VM_END1_CALLS = "end1.calls";
	static const char * VM_END1_CALLS_LOCAL = "local";

	static const char * VM_AND = "and";
	static const char * VM_AND_TO = "to";
	static const char * VM_OR = "or";
	static const char * VM_OR_TO = "to";

	static const char * VM_ABSAND = "absand";
	static const char * VM_ABSAND_TO = "to";
	static const char * VM_ABSOR = "absor";
	static const char * VM_ABSOR_TO = "to";

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

	static const char * VM_NEQ_SI = "neq.si";
	static const char * VM_NEQ_SI_LOCAL = "local";
	static const char * VM_NEQ_SI_TO = "to";

	static const char * VM_NEQ_SS = "neq.ss";
	static const char * VM_NEQ_SS_LOCAL0 = "local0";
	static const char * VM_NEQ_SS_LOCAL1 = "local1";
	static const char * VM_NEQ_SS_TO = "to";

	static const char * VM_EQ_SI = "eq.si";
	static const char * VM_EQ_SI_LOCAL = "local";
	static const char * VM_EQ_SI_TO = "to";

	static const char * VM_EQ_SS = "eq.ss";
	static const char * VM_EQ_SS_LOCAL0 = "local0";
	static const char * VM_EQ_SS_LOCAL1 = "local1";
	static const char * VM_EQ_SS_TO = "to";

	static const char * VM_DUP = "dup";
	static const char * VM_ERASE = "erase";
	static const char * VM_ERASE_NUM = "erase.num";
	static const char * VM_ERASE_NUM_N = "n";
	static const char * VM_DECR = "decr";
	static const char * VM_INCR = "incr";
	static const char * VM_INCR_BY = "incr.by";
	static const char * VM_INCR_BY_BY = "by";

	static const char * VM_INCR_LOCAL_BY = "incr.local.by";
	static const char * VM_INCR_LOCAL_BY_LOCAL = "local";
	static const char * VM_INCR_LOCAL_BY_BY = "by";

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
	static const char * VM_CHECK_COUNT = "check.count";
	static const char * VM_CHECK_COUNT_COUNT = "count";

	static const char * VM_GOTO = "goto";
	static const char * VM_GOTO_TO = "to";
	static const char * VM_BYPASS = "bypass";
	static const char * VM_BYPASS_TO = "to";
	static const char * VM_IFNOT = "ifnot";
	static const char * VM_IFNOT_TO = "to";
	static const char * VM_IFSO = "ifso";
	static const char * VM_IFSO_TO = "to";

	static const char * VM_IFSO_LOCAL = "ifso.local";
	static const char * VM_IFSO_LOCAL_TO = "to";
	static const char * VM_IFSO_LOCAL_LOCAL = "local";

	static const char * VM_IFNOT_LOCAL = "ifnot.local";
	static const char * VM_IFNOT_LOCAL_TO = "to";
	static const char * VM_IFNOT_LOCAL_LOCAL = "local";

	static const char * VM_SYSCALL = "syscall";
	static const char * VM_SYSCALL_NAME = "name";
	static const char * VM_SYSRETURN = "sysreturn";

	static const char * VM_SET_COUNT_SYSCALL = "set.count.syscall";
	static const char * VM_SET_COUNT_SYSCALL_NAME = "name";
	static const char * VM_SET_COUNT_SYSCALL_COUNT = "count";

	static const char * VM_ESCAPE = "escape";
	static const char * VM_FAIL = "fail";

	static const char * VM_GETITERATOR = "getiterator";

	static const char * VM_FIELD = "field";
	static const char * VM_FIELD_N = "n";
}
