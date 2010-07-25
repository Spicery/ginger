package com.steelypip.appginger.codegen;

import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.List;

public abstract class Implementation extends GeneralInstructionSet {

	private List< Instruction > cached = null;
	
	public List< Instruction > instructions() {
		if ( this.cached != null ) return this.cached;
		final List< Instruction > answer = new ArrayList< Instruction >();
		answer.add( this._add_i().init( "_add", "i" ) );
		answer.add( this._decr_i().init( "_decr", "i" ) );
		answer.add( this._div_i().init( "_div", "i" ) );
		answer.add( this._eq_i().init( "_eq", "i" ) );
		answer.add( this._neq_i().init( "_neq", "i" ) );
		answer.add( this._gt_i().init( "_gt", "i" ) );
		answer.add( this._gte_i().init( "_gte","i" ) );
		answer.add( this._incr_i().init( "_incr", "i" ) );
		answer.add( this._incr_by_ir().init( "_incr_by", "ir" ) );
		answer.add( this._lt_i().init( "_lt", "i" ) );
		answer.add( this._lte_i().init( "_lte", "i" ) );
		answer.add( this._mul_i().init( "_mul", "i" ) );
		answer.add( this._sub_i().init( "_sub", "i" ) );
		answer.add( this.calls_i().init( "calls", "i" ) );
		answer.add( this.check0_ir().init( "check0", "ir" ) );
		answer.add( this.check1_ir().init( "check1", "ir" ) );
		answer.add( this.end_ir().init( "end", "ir" ) );
		answer.add( this.end1_calls_ir().init( "end1_calls", "ir" ) );
		answer.add( this.end_call_global_irv().init( "end_call_global", "irv" ) );
		answer.add( this.enter_i().init( "enter", "i" ) );
		answer.add( this.enter0_i().init( "enter0", "i" ) );
		answer.add( this.enter1_i().init( "enter1", "i" ) );
		answer.add( this.eq_si_irrr().init( "eq_si", "irrr" ) );
		answer.add( this.eq_ss_irrr().init( "eq_ss", "irrr" ) );
		answer.add( this.goto_ir().init( "goto", "ir" ) );
		answer.add( this.gt_si_irrr().init( "gt_si", "irrr" ) );
		answer.add( this.gt_ss_irrr().init( "gt_ss", "irrr" ) );
		answer.add( this.gte_si_irrr().init( "gte_si", "irrr" ) );
		answer.add( this.gte_ss_irrr().init( "gte_ss", "irrr" ) );
		answer.add( this.halt_i().init( "halt", "i" ) );
		answer.add( this.ifnot_ir().init( "ifnot", "ir" ) );
		answer.add( this.ifso_ir().init( "ifso", "ir" ) );
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
		answer.add( this.reset_i().init( "reset", "i" ) );
		answer.add( this.return_i().init( "return", "i" ) );
		answer.add( this.set_ir().init( "set", "ir" ) );
		answer.add( this.set_call_global_irv().init( "set_call_global", "irv" ) );
		answer.add( this.set_calls_ir().init( "set_calls", "ir" ) );
		answer.add( this.start_ir().init( "start", "ir" ) );
		answer.add( this.syscall_ir().init( "syscall", "ir" ) );
		answer.add( this.sysreturn_i().init( "sysreturn", "i" ) );
		return this.cached = answer;
	}
	
	/*void generateSpecialPrototypes( final PrintWriter output ) {
		for ( Instruction inst : this.instructions() ) {
			output.format( "extern Special spc_%s;\n", inst.getName() );
		}
	}*/
	
	/*void generateSpecialName( final PrintWriter output ) {
		output.format( "const char *special_name( Special spc ) {\n" );
		output.format( "    return(\n" );
		for ( Instruction inst : this.instructions() ) {
			final String name = inst.getName();
			output.format(  "        spc == spc_%s ? \"%s\" :\n", name, name );
		}
		output.format( "        \"?\"\n" );
		output.format( "    );\n" );
		output.format( "}\n\n" );
		
	}*/
	
	/*void generateClassify( final PrintWriter output ) {
		output.format(  "static const char *classify( Special x ) {\n" );
		for ( Instruction inst : this.instructions() ) {
			if ( !"i".equals( inst.getType() ) ) {
				output.format( "    if ( x == spc_%s ) return \"%s\";\n", inst.getName(), inst.getType() );
			}
		}
		output.format( "    return \"i\";\n" );
		output.format( "}\n\n" );
		
	}*/
	
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
