package com.steelypip.appginger.codegen;

import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.List;

public abstract class Implementation extends GeneralInstructionSet {

	private List< Instruction > cached = null;
	
	public List< Instruction > instructions() {
		if ( this.cached != null ) return this.cached;
		final List< Instruction > answer = new ArrayList< Instruction >();
		answer.add( this.add_i().init( "add", "i" ) );
		answer.add( this.decr_i().init( "decr", "i" ) );
		answer.add( this.div_i().init( "div", "i" ) );
		answer.add( this.eq_i().init( "eq", "i" ) );
		//answer.add( this.eqq_ir().init( "eqq", "ir" ) );
		answer.add( this.neq_i().init( "neq", "i" ) );
		answer.add( this.gt_i().init( "gt", "i" ) );
		answer.add( this.gte_i().init( "gte","i" ) );
		answer.add( this.incr_i().init( "incr", "i" ) );
		answer.add( this.incr_by_ir().init( "incr_by", "ir" ) );
		answer.add( this.lt_i().init( "lt", "i" ) );
		answer.add( this.lte_i().init( "lte", "i" ) );
		answer.add( this.mul_i().init( "mul", "i" ) );
		answer.add( this.sub_i().init( "sub", "i" ) );
		answer.add( this.calls_i().init( "calls", "i" ) );
		answer.add( this.chainlite_icr().init( "chainlite", "icr" )  );
		answer.add( this.check_count_ir().init( "check_count", "ir" ) );
		answer.add( this.check_mark0_ir().init( "check_mark0", "ir" ) );
		answer.add( this.check_mark1_ir().init( "check_mark1", "ir" ) );
		answer.add( this.end_mark_ir().init( "end_mark", "ir" ) );
		answer.add( this.end1_calls_ir().init( "end1_calls", "ir" ) );
		answer.add( this.end_call_global_irv().init( "end_call_global", "irv" ) );
		answer.add( this.enter_i().init( "enter", "i" ) );
		answer.add( this.enter0_i().init( "enter0", "i" ) );
		answer.add( this.enter1_i().init( "enter1", "i" ) );
		answer.add( this.eq_si_irrr().init( "eq_si", "irrr" ) );
		answer.add( this.eq_ss_irrr().init( "eq_ss", "irrr" ) );
		answer.add( this.erase_i().init( "erase", "i" ) );
		answer.add( this.field_ir().init( "field", "ir" ) );
		answer.add( this.getiterator_i().init( "getiterator", "i" ) );
		answer.add( this.goto_ir().init( "goto", "ir" ) );
		answer.add( this.gt_si_irrr().init( "gt_si", "irrr" ) );
		answer.add( this.gt_ss_irrr().init( "gt_ss", "irrr" ) );
		answer.add( this.gte_si_irrr().init( "gte_si", "irrr" ) );
		answer.add( this.gte_ss_irrr().init( "gte_ss", "irrr" ) );
		answer.add( this.halt_i().init( "halt", "i" ) );
		answer.add( this.ifnot_ir().init( "ifnot", "ir" ) );
		answer.add( this.ifso_ir().init( "ifso", "ir" ) );
		answer.add( this.invoke_icc().init( "invoke", "icc" ) );
		answer.add( this.listiterate_i().init( "listiterate", "i" ) );
		answer.add( this.lt_si_irrr().init( "lt_si", "irrr" ) );
		answer.add( this.lt_ss_irrr().init( "lt_ss", "irrr" ) );
		answer.add( this.lte_si_irrr().init( "lte_si", "irrr" ) );
		answer.add( this.lte_ss_irrr().init( "lte_ss", "irrr" ) );
		answer.add( this.neq_si_irrr().init( "neq_si", "irrr" ) );
		answer.add( this.neq_ss_irrr().init( "neq_ss", "irrr" ) );
		answer.add( this.not_i().init( "not", "i" ) );
		answer.add( this.pop_global_iv().init( "pop_global", "iv" ) );
		answer.add( this.pop_local_ir().init( "pop_local", "ir") );
		answer.add( this.push_global_iv().init( "push_global", "iv" ) );
		answer.add( this.push_local_ir().init( "push_local", "ir" ) );
		answer.add( this.push_local0_i().init( "push_local0", "i" ) );
		answer.add( this.push_local1_i().init( "push_local1", "i" ) );
		answer.add( this.pushq_ic().init( "pushq", "ic" ) );
		answer.add( this.push_local_ret_ir().init( "push_local_ret", "ir" ) );
		answer.add( this.push_local0_ret_i().init( "push_local0_ret", "i" ) );
		answer.add( this.push_local1_ret_i().init( "push_local1_ret", "i" ) );
		answer.add( this.pushq_ret_ic().init( "pushq_ret", "ic" ) );
		answer.add( this.reset_i().init( "reset", "i" ) );
		answer.add( this.return_i().init( "return", "i" ) );
		answer.add( this.return_ifso_i().init( "return_ifso", "i" ) );
		answer.add( this.return_ifnot_i().init( "return_ifnot", "i" ) );
		answer.add( this.self_call_i().init( "self_call", "i" ) );
		answer.add( this.self_call_n_ir().init( "self_call_n", "ir" ) );
		answer.add( this.self_constant_i().init( "self_constant", "i" ) );
		answer.add( this.set_ir().init( "set", "ir" ) );
		answer.add( this.set_call_global_irv().init( "set_call_global", "irv" ) );
		answer.add( this.set_calls_ir().init( "set_calls", "ir" ) );
		answer.add( this.set_syscall_irr().init( "set_syscall", "irr" ) );
		answer.add( this.start_mark_ir().init( "start_mark", "ir" ) );
		answer.add( this.stringiterate_i().init( "stringiterate", "i" ) );
		answer.add( this.syscall_ir().init( "syscall", "ir" ) );
		answer.add( this.syscall_arg_irc().init( "syscall_arg", "irc" ) );
		answer.add( this.syscall_dat_irr().init( "syscall_dat", "irr" ) );
		answer.add( this.syscall_argdat_ircr().init( "syscall_argdat", "ircr" ) );
		answer.add( this.sysreturn_i().init( "sysreturn", "i" ) );
		answer.add( this.vectoriterate_i().init( "vectoriterate", "i" ) );
		answer.add( this.mixediterate_i().init( "mixediterate", "i" ) );
		return this.cached = answer;
	}
	
	void generateInstructionSetHPP( final PrintWriter output ) {
		for ( Instruction inst : this.instructions() ) {
			final String name = inst.getName();
			output.format( "	Ref spc_%s;\n", name );
		}
		
	}
	
	void generateInstructionHPP( final PrintWriter output ) {
		String sep = "";
		for ( Instruction inst : this.instructions() ) {
			final String name = inst.getName();
			output.format(  "%s    vmc_%s", sep, name );
			sep = ",\n";
		}
		output.println();
	}
	
	void generateInstructionSetCPP( final PrintWriter output ) {
		output.format( "const char * InstructionSet::name( Ref spc ) const {\n" );
		output.format( "    return(\n" );
		for ( Instruction inst : this.instructions() ) {
			final String name = inst.getName();
			output.format(  "        spc == spc_%s ? \"%s\" :\n", name, name );
		}
		output.format( "        \"?\"\n" );
		output.format( "    );\n" );
		output.format( "}\n\n" );
		
		output.format(  "const char * InstructionSet::signature( Ref x ) const {\n" );
		for ( Instruction inst : this.instructions() ) {
			if ( !"i".equals( inst.getType() ) ) {
				output.format( "    if ( x == spc_%s ) return \"%s\";\n", inst.getName(), inst.getType() );
			}
		}
		output.format( "    return \"i\";\n" );
		output.format( "}\n\n" );
	
		output.format( "Ref InstructionSet::lookup( Instruction instr ) const {\n" );
		output.format( "    switch ( instr ) {\n" );
		for ( Instruction inst : this.instructions() ) {
			final String iname = inst.getName();
			output.format( "    case vmc_%s: return this->spc_%s;\n", iname, iname );
		}
		output.format( "    }\n" );
		output.format( "    throw Mishap( \"Bad instruction\" );\n" );
		output.format( "}\n\n" );
	
		
	}
	
	void generateImplementationCPP( final PrintWriter output ) {
		// Do nothing.
	}
	
	
	
	//public abstract void generateInstructionSetNthHPP( final PrintWriter output );
	public abstract void generateInstructionSetNthCPP( final PrintWriter output );
	public final void generateCode( final PrintWriter output ) {}		
	public abstract int getNumId();
}
