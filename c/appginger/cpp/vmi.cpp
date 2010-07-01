#include <vector>

#include "vmi.hpp"
#include "plant.hpp"
#include "key.hpp"
#include "mishap.hpp"
#include "printfn.hpp"
#include "instruction.hpp"

#include <stdio.h>

//#define DBG_VMI

#ifdef DBG_VMI
	#include "scanfunc.hpp"	
#endif


static void emitSPC( Plant plant, Instruction instr ) {
	const InstructionSet & ins = plant->instructionSet();
	Ref instr_ptr = ins.lookup( instr );
	plant->plantRef( instr_ptr );
}

static void emitRef( Plant plant, Ref ref ) {
	plant->plantRef( ref );
}

void vmiAPPSPC( Plant plant, Instruction instr ) {
	emitSPC( plant, instr );
}

void vmiSYS_CALL( Plant plant, Ref r ) {
	emitSPC( plant, vmc_syscall );
	emitRef( plant, r );
}

void vmiOPERATOR( Plant plant, Functor fnc ) {
	vmiAPPSPC(
		plant,
        fnc == fnc_eq ? vmc__eq :
        fnc == fnc_lt ? vmc__lt :
        fnc == fnc_lte ? vmc__lte :
        fnc == fnc_gt ? vmc__gt :
        fnc == fnc_gte ? vmc__gte :
        fnc == fnc_mul ? vmc__mul :
        fnc == fnc_div ? vmc__div :
        fnc == fnc_sub ? vmc__sub :
        fnc == fnc_add ? vmc__add :
        fnc == fnc_decr ? vmc__decr :
        fnc == fnc_incr ? vmc__incr :
        fnc == fnc_not ? vmc_not :
        fnc == fnc_neq ? vmc__neq :
        ( this_never_happens(), (Instruction)0 )
	);
}

void vmiINCR( Plant plant, int n ) {
	switch ( n ) {
		case 0: {
			return;
		}
		case 1: {
			emitSPC( plant, vmc__incr );
			return;
		}
		case -1: {
			emitSPC( plant, vmc__decr );
			return;
		}
		default: {
			emitSPC( plant, vmc__incr_by );
			emitRef( plant, ToRef( IntToSmall( n ) ) );
		}
	}
}

void vmiPOPID( Plant plant, Ident id ) {
	if ( id->is_local ) {
		emitSPC( plant, vmc_pop_local );
		emitRef( plant, ToRef( id->slot ) );
	} else {
		emitSPC( plant, vmc_pop_global );
		emitRef( plant, ToRef( id.get() ) );
	}
}

void vmiPUSHID( Plant plant, Ident id ) {
	if ( id->is_local ) {
		switch ( id->slot ) {
		case 0:
			emitSPC( plant, vmc_push_local0 );
			return;
		case 1:
			emitSPC( plant, vmc_push_local1 );
			return;
		default:	
			emitSPC( plant, vmc_push_local );
			emitRef( plant, ToRef( id->slot ) );
		}
	} else {
		emitSPC( plant, vmc_push_global );
		emitRef( plant, ToRef( id.get() ) );
	}
}

void vmiSET( Plant plant, int A ) {
	emitSPC( plant, vmc_set );
	emitRef( plant, ToRef( A ) );
}

void vmiCALLS( Plant plant ) {
	emitSPC( plant, vmc_calls );
}

void vmiEND_CALL_ID( Plant plant, int var, Ident ident ) {
	if ( ident->is_local ) {
		vmiEND( plant, var );
		vmiPUSHID( plant, ident );
		emitSPC( plant, vmc_calls );
	} else {
		emitSPC( plant, vmc_end_call_global );
		emitRef( plant, ToRef( var ) );
		emitRef( plant, ToRef( ident.get() ) );
	}
}

void vmiSET_CALL_ID( Plant plant, int in_arity, Ident ident ) {
	if ( ident->is_local ) {
		vmiSET( plant, in_arity );
		vmiPUSHID( plant, ident );
		emitSPC( plant, vmc_calls );
	} else {
		emitSPC( plant, vmc_set_call_global );
		emitRef( plant, ToRef( in_arity ) );
		emitRef( plant, ToRef( ident.get() ) );
	}
}

void vmiEND1_CALLS( Plant plant, int var ) {
	emitSPC( plant, vmc_end1_calls );
	emitRef( plant, ToRef( var ) );
}

void vmiSET_CALLS( Plant plant, int in_arity ) {
	emitSPC( plant, vmc_set_calls );
	emitRef( plant, ToRef( in_arity ) );
}

void vmiSTART( Plant plant, int v ) {
	emitSPC( plant, vmc_start );
	emitRef( plant, ToRef( v ) );
}

void vmiEND( Plant plant, int v ) {
	emitSPC( plant, vmc_end );
	emitRef( plant, ToRef( v ) );
}

void vmiCHECK1( Plant plant, int v ) {
	emitSPC( plant, vmc_check1 );
	emitRef( plant, ToRef( v ) );
}

//	Do we ever generate this?
void vmiCHECK0( Plant plant, int v ) {
	emitSPC( plant, vmc_check0 );
	emitRef( plant, ToRef( v ) );
}

void vmiPUSHQ( Plant plant, Ref obj ) {
	emitSPC( plant, vmc_pushq );
	emitRef( plant, obj );
}


void vmiRETURN( Plant plant ) {
	emitSPC( plant, vmc_return );
}

void vmiENTER( Plant plant ) {
	emitSPC(
		plant,
		plant->ninputs == 0 ? vmc_enter0 :
		plant->ninputs == 1 ? vmc_enter1 :
		vmc_enter
	);
}

void vmiFUNCTION( Plant plant, int N, int A ) {
	plant->save( N, A );
	
}

Ref vmiENDFUNCTION( Plant plant ) {
	Ref r;

	r = plant->detach();
	#ifndef DBG_VMI
		if ( plant->vm->getShowCode() ) {
			plant->vm->printfn( std::clog, r );
		}
	#else
		plant->vm->printfn( std::clog, r );
		Ref * p = RefToPtr4( r );
		std::clog << "Scanning ... " << std::endl;
		ScanFunc scan( plant->vm->instructionSet(), p );
		for (;;) {
			Ref * pc = scan.next();
			if ( !pc ) break;
			std::clog << "Ref at offset " << ( pc - p ) << std::endl;
		}
	#endif
	plant->restore();
	
	return r;
}

void vmiNOT( Plant plant ) {
	emitSPC( plant, vmc_not );
}

void vmiIFNOT( Plant plant, DestinationClass & d ) {
	emitSPC( plant, vmc_ifnot );
	d.destinationInsert();
}

void vmiIFSO( Plant plant, DestinationClass & d ) {
	emitSPC( plant, vmc_ifso );
	d.destinationInsert();
}

void vmiIF( bool sense, Plant plant, DestinationClass & dst ) {
	if ( sense ) {
		vmiIFSO( plant, dst );
	} else {
		vmiIFNOT( plant, dst );
	}
}

void vmiGOTO( Plant plant, DestinationClass & d ) {
	emitSPC( plant, vmc_goto );
	d.destinationInsert();
}

static bool eval_relop( char op, int a, int b ) {
	return(
		op == 'g' ? a >= b :
		op == '>' ? a > b :
		op == '=' ? a == b :
		op == '<' ? a < b :
		op == 'l' ? a <= b :
		op == '!' ? a != b :
		( this_never_happens(), false )
	);
}

static char rev_relop( char op ) {
	return(
		op == 'g' ? '<' :
		op == '>' ? 'l' :
		op == '=' ? '!' :
		op == '<' ? 'g' :
		op == 'l' ? '>' :
		op == '!' ? '=' :
		( this_never_happens(), '\0' )
	);
}

/*
//	Flag is
//		's'		slot
/		'i'		int
//
//	op is
//		'l'		<=
//		'<'		<
//		'='		==
//		'!'		!=
//		'>'		>
//		'g'		>=
*/

void vmiIF_RELOP( Plant plant, bool sense, char flag1, int arg1, char op, char flag2, int arg2, DestinationClass & dst ) {
	if ( sense ) {
		vmiIFSO_RELOP( plant, flag1, arg1, op, flag2, arg2, dst );
	} else {
		vmiIFNOT_RELOP( plant, flag1, arg1, op, flag2, arg2, dst );
	}
}

void vmiIFSO_RELOP( Plant plant, char flag1, int arg1, char op, char flag2, int arg2, DestinationClass & dst ) {
	if ( flag1 == 'i' && flag2 == 'i' ) {
		if ( eval_relop( op, arg1, arg2 ) ) vmiGOTO( plant, dst );
	} else if ( flag1 == 'i' && flag2 == 's' ) {
		vmiIFSO_RELOP( plant, flag2, arg2, rev_relop( op ), flag1, arg1, dst );
	} else if ( flag1 == 's' && flag2 == 'i' ) {
		emitSPC(
			plant,
		    op == 'g' ? vmc_gte_si :
		    op == '>' ? vmc_gt_si :
		    op == '=' ? vmc_eq_si :
			op == '!' ? vmc_neq_si :
		    op == '<' ? vmc_lt_si :
		    op == 'l' ? vmc_lte_si :
		    ( this_never_happens(), (Instruction)0 )
		);
		emitRef( plant, ToRef( arg1 ) );
		emitRef( plant, IntToSmall( arg2 ) );
		dst.destinationInsert();
	} else if ( flag1 == 's' && flag2 == 's' ) {
		emitSPC(
			plant,
		    op == 'g' ? vmc_gte_ss :
		    op == '>' ? vmc_gt_ss :
		    op == '=' ? vmc_eq_ss :
			op == '!' ? vmc_neq_ss :
		    op == '<' ? vmc_lt_ss :
		    op == 'l' ? vmc_lte_ss :
		    ( this_never_happens(), (Instruction)0 )
		);
		emitRef( plant, ToRef( arg1 ) );
		emitRef( plant, ToRef( arg2 ) );
		dst.destinationInsert();
	} else {
		this_never_happens();
	}
}

void vmiIFNOT_RELOP( Plant plant, char flag1, int arg1, char op, char flag2, int arg2, DestinationClass & dst ) {
	vmiIFSO_RELOP( plant, flag1, arg1, rev_relop( op ), flag2, arg2, dst );
}

void VmiRelOpFactory::setLeft( int arg ) {
	this->flag1 = 'i';
	this->int1 = arg;
}

void VmiRelOpFactory::setLeft( Ident id ) {
	this->flag1 = 's';
	this->ident1 = id;
}

void VmiRelOpFactory::setRight( int arg ) {
	this->flag2 = 'i';
	this->int2 = arg;
}

void VmiRelOpFactory::setRight( Ident id ) {
	this->flag2 = 's';
	this->ident2 = id;
}

void VmiRelOpFactory::setLT() {
	this->op = '<';
}

void VmiRelOpFactory::setGT() {
	this->op = '>';
}

void VmiRelOpFactory::setLTE() {
	this->op = 'l';
}

void VmiRelOpFactory::setGTE() {
	this->op = 'g';
}

void VmiRelOpFactory::setEQ() {
	this->op = '=';
}

void VmiRelOpFactory::setNEQ() {
	this->op = '!';
}

void VmiRelOpFactory::negate() {
	this->op = rev_relop( this->op );
}

void VmiRelOpFactory::compilePushLeft() {
	switch ( this->flag1 ) {
		case 'i': {
			emitRef( this->plant, IntToSmall( this->int1 ) );
			break;
		}
		case 's': {
			vmiPUSHID( this->plant, this->ident1 );
			break;
		}
		default: throw;			//	Never happens.
	}
}

void VmiRelOpFactory::compilePushRight() {
	switch ( this->flag2 ) {
		case 'i': {
			emitRef( this->plant, IntToSmall( this->int2 ) );
			break;
		}
		case 's': {
			vmiPUSHID( this->plant, this->ident2 );
			break;
		}
		default: throw;			//	Never happens.
	}
}

void VmiRelOpFactory::compileOp() {
	vmiOPERATOR( 
		this->plant,
		op == 'g' ? fnc_gte :
		op == '>' ? fnc_gt :
		op == '=' ? fnc_eq  :
		op == '!' ? fnc_neq :
		op == '<' ? fnc_lt :
		op == 'l' ? fnc_lte :
		( this_never_happens(), (Functor)0 )
	);
}


void VmiRelOpFactory::ifSo( DestinationClass &dst ) {
	if ( this->flag1 == '?' ) throw;
	if ( this->flag2 == '?' ) throw;
	if ( this->op == '?' ) throw;
	if ( 
		( this->flag1 != 's' || this->ident1->is_local ) &&
		( this->flag2 != 's' || !this->ident2->is_local )
	) {
		int arg1 = this->flag1 == 'i' ? this->int1 : this->ident1->slot;
		int arg2 = this->flag1 == 'i' ? this->int2 : this->ident2->slot;
		vmiIFSO_RELOP( this->plant, this->flag1, arg1, this->op, this->flag2, arg2, dst );
	} else {
		this->compilePushLeft();
		this->compilePushRight();
		this->compileOp();
		vmiIFSO( this->plant, dst );
	}
}

void VmiRelOpFactory::ifNot( DestinationClass &dst ) {
	if ( this->flag1 == '?' ) throw;
	if ( this->flag2 == '?' ) throw;
	if ( this->op == '?' ) throw;
	if ( this->flag1 == 's' && !this->ident1->is_local ) throw;
	if ( this->flag2 == 's' && !this->ident2->is_local ) throw;
	if ( 
		( this->flag1 != 's' || this->ident1->is_local ) &&
		( this->flag2 != 's' || !this->ident2->is_local )
	) {
		int arg1 = this->flag1 == 'i' ? this->int1 : this->ident1->slot;
		int arg2 = this->flag1 == 'i' ? this->int2 : this->ident2->slot;
		vmiIFNOT_RELOP( this->plant, this->flag1, arg1, this->op, this->flag2, arg2, dst );
	} else {
		this->compilePushLeft();
		this->compilePushRight();
		this->compileOp();
		vmiIFNOT( this->plant, dst );
	}
}
