package com.steelypip.appginger.codegen;

import java.io.PrintWriter;

public class GenericImplementation extends Implementation {

	@Override
	public Instruction _add_i() {
		return new GenericInstruction();
	}

	@Override
	public Instruction _decr_i() {
		return new GenericInstruction();
	}

	@Override
	public Instruction _div_i() {
		return new GenericInstruction();
	}

	@Override
	public Instruction _eq_i() {
		return new GenericInstruction();
	}

	@Override
	public Instruction _neq_i() {
		return new GenericInstruction();
	}

	@Override
	public Instruction _gt_i() {
		return new GenericInstruction();
	}

	@Override
	public Instruction _gte_i() {
		return new GenericInstruction();
	}

	@Override
	public Instruction _incr_by_ir() {
		return new GenericInstruction();
	}

	@Override
	public Instruction _incr_i() {
		return new GenericInstruction();
	}

	@Override
	public Instruction _lt_i() {
		return new GenericInstruction();
	}

	@Override
	public Instruction _lte_i() {
		return new GenericInstruction();
	}

	@Override
	public Instruction _mul_i() {
		return new GenericInstruction();
	}

	@Override
	public Instruction _sub_i() {
		return new GenericInstruction();
	}

	@Override
	public Instruction calls_i() {
		return new GenericInstruction();
	}

	@Override
	public Instruction check0_ir() {
		return new GenericInstruction();
	}

	@Override
	public Instruction check1_ir() {
		return new GenericInstruction();
	}

	@Override
	public Instruction end1_calls_ir() {
		return new GenericInstruction();
	}

	@Override
	public Instruction end_call_global_irv() {
		return new GenericInstruction();
	}

	@Override
	public Instruction end_ir() {
		return new GenericInstruction();
	}

	@Override
	public Instruction enter0_i() {
		return new GenericInstruction();
	}

	@Override
	public Instruction enter1_i() {
		return new GenericInstruction();
	}

	@Override
	public Instruction enter_i() {
		return new GenericInstruction();
	}

	@Override
	public Instruction eq_si_irrr() {
		return new GenericInstruction();
	}

	@Override
	public Instruction eq_ss_irrr() {
		return new GenericInstruction();
	}

	@Override
	public Instruction goto_ir() {
		return new GenericInstruction();
	}

	@Override
	public Instruction gt_si_irrr() {
		return new GenericInstruction();
	}

	@Override
	public Instruction gt_ss_irrr() {
		return new GenericInstruction();
	}

	@Override
	public Instruction gte_si_irrr() {
		return new GenericInstruction();
	}

	@Override
	public Instruction gte_ss_irrr() {
		return new GenericInstruction();
	}

	@Override
	public Instruction halt_i() {
		return new GenericInstruction();
	}

	@Override
	public Instruction ifnot_ir() {
		return new GenericInstruction();
	}

	@Override
	public Instruction ifso_ir() {
		return new GenericInstruction();
	}

	@Override
	public Instruction lt_si_irrr() {
		return new GenericInstruction();
	}

	@Override
	public Instruction lt_ss_irrr() {
		return new GenericInstruction();
	}

	@Override
	public Instruction lte_si_irrr() {
		return new GenericInstruction();
	}

	@Override
	public Instruction lte_ss_irrr() {
		return new GenericInstruction();
	}

	@Override
	public Instruction neq_si_irrr() {
		return new GenericInstruction();
	}

	@Override
	public Instruction neq_ss_irrr() {
		return new GenericInstruction();
	}

	@Override
	public Instruction not_i() {
		return new GenericInstruction();
	}

	@Override
	public Instruction pop_global_iv() {
		return new GenericInstruction();
	}

	@Override
	public Instruction pop_local_ir() {
		return new GenericInstruction();
	}

	@Override
	public Instruction push_global_iv() {
		return new GenericInstruction();
	}

	@Override
	public Instruction push_local0_i() {
		return new GenericInstruction();
	}

	@Override
	public Instruction push_local1_i() {
		return new GenericInstruction();
	}

	@Override
	public Instruction push_local_ir() {
		return new GenericInstruction();
	}

	@Override
	public Instruction pushq_ic() {
		return new GenericInstruction();
	}

	@Override
	public Instruction reset_i() {
		return new GenericInstruction();
	}

	@Override
	public Instruction return_i() {
		return new GenericInstruction();
	}

	@Override
	public Instruction set_call_global_irv() {
		return new GenericInstruction();
	}

	@Override
	public Instruction set_calls_ir() {
		return new GenericInstruction();
	}

	@Override
	public Instruction set_ir() {
		return new GenericInstruction();
	}

	@Override
	public Instruction start_ir() {
		return new GenericInstruction();
	}

	@Override
	public Instruction sysreturn_i() {
		return new GenericInstruction();
	}


	@Override
	public int getNumId() {
		throw new RuntimeException();
	}

	@Override
	public void generateInstructionSetNthCPP( PrintWriter output ) {
		throw new RuntimeException();
	}

	
}
