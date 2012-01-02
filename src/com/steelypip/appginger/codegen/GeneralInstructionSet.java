package com.steelypip.appginger.codegen;

public abstract class GeneralInstructionSet {


	public Instruction add_i() {
		return new GenericInstruction();
	}


	public Instruction decr_i() {
		return new GenericInstruction();
	}


	public Instruction div_i() {
		return new GenericInstruction();
	}


	public Instruction eq_i() {
		return new GenericInstruction();
	}

//	public Instruction eqq_ir() {
//		return new GenericInstruction();
//	}

	public Instruction neq_i() {
		return new GenericInstruction();
	}


	public Instruction gt_i() {
		return new GenericInstruction();
	}


	public Instruction gte_i() {
		return new GenericInstruction();
	}


	public Instruction incr_by_ir() {
		return new GenericInstruction();
	}


	public Instruction incr_i() {
		return new GenericInstruction();
	}


	public Instruction lt_i() {
		return new GenericInstruction();
	}


	public Instruction lte_i() {
		return new GenericInstruction();
	}


	public Instruction mul_i() {
		return new GenericInstruction();
	}

	public Instruction sub_i() {
		return new GenericInstruction();
	}


	public Instruction calls_i() {
		return new GenericInstruction();
	}
	
	public Instruction chainlite_icr() {
		return new GenericInstruction();
	}


	public Instruction check_mark0_ir() {
		return new GenericInstruction();
	}


	public Instruction check_count_ir() {
		return new GenericInstruction();
	}

	public Instruction check_mark1_ir() {
		return new GenericInstruction();
	}


	public Instruction end1_calls_ir() {
		return new GenericInstruction();
	}


	public Instruction end_call_global_irv() {
		return new GenericInstruction();
	}


	public Instruction end_mark_ir() {
		return new GenericInstruction();
	}


	public Instruction enter0_i() {
		return new GenericInstruction();
	}


	public Instruction enter1_i() {
		return new GenericInstruction();
	}


	public Instruction enter_i() {
		return new GenericInstruction();
	}


	public Instruction eq_si_irrr() {
		return new GenericInstruction();
	}


	public Instruction eq_ss_irrr() {
		return new GenericInstruction();
	}
	
	public Instruction erase_i() {
		return new GenericInstruction();
	}

	public Instruction field_ir() {
		return new GenericInstruction();
	}


	public Instruction goto_ir() {
		return new GenericInstruction();
	}

	public Instruction getiterator_i() {
		return new GenericInstruction();
	}


	public Instruction gt_si_irrr() {
		return new GenericInstruction();
	}


	public Instruction gt_ss_irrr() {
		return new GenericInstruction();
	}


	public Instruction gte_si_irrr() {
		return new GenericInstruction();
	}


	public Instruction gte_ss_irrr() {
		return new GenericInstruction();
	}


	public Instruction halt_i() {
		return new GenericInstruction();
	}


	public Instruction ifnot_ir() {
		return new GenericInstruction();
	}
	
	public Instruction ifso_ir() {
		return new GenericInstruction();
	}
	
	public Instruction invoke_icc() {
		return new GenericInstruction();
	}
	
	public Instruction listiterate_i() {
		return new GenericInstruction();
	}

	public Instruction lt_si_irrr() {
		return new GenericInstruction();
	}


	public Instruction lt_ss_irrr() {
		return new GenericInstruction();
	}


	public Instruction lte_si_irrr() {
		return new GenericInstruction();
	}


	public Instruction lte_ss_irrr() {
		return new GenericInstruction();
	}


	public Instruction neq_si_irrr() {
		return new GenericInstruction();
	}


	public Instruction neq_ss_irrr() {
		return new GenericInstruction();
	}
	
	public Instruction not_i() {
		return new GenericInstruction();
	}


	public Instruction pop_global_iv() {
		return new GenericInstruction();
	}


	public Instruction pop_local_ir() {
		return new GenericInstruction();
	}


	public Instruction push_global_iv() {
		return new GenericInstruction();
	}


	public Instruction push_local0_i() {
		return new GenericInstruction();
	}


	public Instruction push_local1_i() {
		return new GenericInstruction();
	}


	public Instruction push_local_ir() {
		return new GenericInstruction();
	}


	public Instruction pushq_ic() {
		return new GenericInstruction();
	}

	public Instruction push_local0_ret_i() {
		return new GenericInstruction();
	}


	public Instruction push_local1_ret_i() {
		return new GenericInstruction();
	}


	public Instruction push_local_ret_ir() {
		return new GenericInstruction();
	}


	public Instruction pushq_ret_ic() {
		return new GenericInstruction();
	}


	public Instruction reset_i() {
		return new GenericInstruction();
	}


	public Instruction return_i() {
		return new GenericInstruction();
	}
	
	public Instruction return_ifso_i() {
		return new GenericInstruction();
	}
	
	public Instruction return_ifnot_i() {
		return new GenericInstruction();
	}
	
	public Instruction self_call_i() {
		return new GenericInstruction();		
	}
	
	public Instruction self_call_n_ir() {
		return new GenericInstruction();	
	}
	
	public Instruction self_constant_i() {
		return new GenericInstruction();		
	}


	public Instruction set_call_global_irv() {
		return new GenericInstruction();
	}


	public Instruction set_calls_ir() {
		return new GenericInstruction();
	}


	public Instruction set_ir() {
		return new GenericInstruction();
	}

	public Instruction set_syscall_irr() {
		return new GenericInstruction();
	}




	public Instruction start_mark_ir() {
		return new GenericInstruction();
	}
	
	public Instruction stringiterate_i() {
		return new GenericInstruction();	
	}
	
	public Instruction syscall_ir() {
		return new GenericInstruction();	
	}
	
	public Instruction syscall_arg_irc() {
		return new GenericInstruction();	
	}
	
	public Instruction syscall_dat_irr() {
		return new GenericInstruction();	
	}
	
	public Instruction syscall_argdat_ircr() {
		return new GenericInstruction();	
	}
	
	public Instruction sysreturn_i() {
		return new GenericInstruction();	
	}
	
	public Instruction vectoriterate_i() {
		return new GenericInstruction();	
	}
	
	public Instruction mixediterate_i() {
		return new GenericInstruction();	
	}
	

}
