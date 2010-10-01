#include <vector>

#include "vmi.hpp"
#include "plant.hpp"
#include "key.hpp"
#include "mishap.hpp"
#include "printfn.hpp"
#include "instruction.hpp"
#include "sysindirection.hpp"

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

static void emitValof( Plant plant, Valof *v ) {
	plant->plantRef( ToRef( v ) );
}

void vmiINSTRUCTION( Plant plant, Instruction instr ) {
	emitSPC( plant, instr );
}

void vmiSYS_CALL( Plant plant, SysCall * r ) {
	emitSPC( plant, vmc_syscall );
	emitRef( plant, ToRef( r ) );
}

/**
	Plants a non-garbage collectable reference as extra data for
	the system call, to be accessed via pc[-1].
	@param plant the code-planter
	@param r the system call
	@param data arbitrary data, size compatible with void*
*/
void vmiSYS_CALL_ARG( Plant plant, SysCall * sys, Ref ref ) {
	emitSPC( plant, vmc_syscall_arg );
	emitRef( plant, ToRef( sys ) );
	emitRef( plant, ref );
}

void vmiSYS_CALL_DAT( Plant plant, SysCall * sys, unsigned long data ) {
	emitSPC( plant, vmc_syscall_dat );
	emitRef( plant, ToRef( sys ) );
	emitRef( plant, ToRef( data ) );
}

void vmiSYS_CALL_ARGDAT( Plant plant, SysCall * sys, Ref ref, unsigned long data ) {
	emitSPC( plant, vmc_syscall_argdat );
	emitRef( plant, ToRef( sys ) );
	emitRef( plant, ref );
	emitRef( plant, ToRef( data ) );
}


void vmiSET_SYS_CALL( Plant plant, SysCall * r, int A ) {
	emitSPC( plant, vmc_set_syscall );
	emitRef( plant, ToRef( A ) );
	emitRef( plant, ToRef( r ) );
}

void vmiSYS_RETURN( Plant plant ) {
	emitSPC( plant, vmc_sysreturn );
}

void vmiOPERATOR( Plant plant, Functor fnc ) {
	vmiINSTRUCTION(
		plant,
        fnc == fnc_eq ? vmc_eq :
        fnc == fnc_lt ? vmc_lt :
        fnc == fnc_lte ? vmc_lte :
        fnc == fnc_gt ? vmc_gt :
        fnc == fnc_gte ? vmc_gte :
        fnc == fnc_mul ? vmc_mul :
        fnc == fnc_div ? vmc_div :
        fnc == fnc_sub ? vmc_sub :
        fnc == fnc_add ? vmc_add :
        fnc == fnc_decr ? vmc_decr :
        fnc == fnc_incr ? vmc_incr :
        fnc == fnc_not ? vmc_not :
        fnc == fnc_neq ? vmc_neq :
        ( this_never_happens(), (Instruction)0 )
	);
}

void vmiINCR( Plant plant, int n ) {
	switch ( n ) {
		case 0: {
			return;
		}
		case 1: {
			emitSPC( plant, vmc_incr );
			return;
		}
		case -1: {
			emitSPC( plant, vmc_decr );
			return;
		}
		default: {
			emitSPC( plant, vmc_incr_by );
			emitRef( plant, ToRef( LongToSmall( n ) ) );
		}
	}
}

void vmiCHAIN_LITE( Plant plant, Ref fn, long N ) {
	emitSPC( plant, vmc_chainlite );
	emitRef( plant, fn );
	emitRef( plant, ToRef( N ) );
}

/** vmiNEWID only affects mutable outer variables. If a
	variable is determined to be mutable and outer it is
	potentially shareable between different nested functions.
	
	If the shared flag is set, then the variable is implemented
	as a pointer to a special type of heap object, an Indirection.
	
	vmiNEWID allocates the Indirection, initialises its value to
	the constant -undef-, and assigns that to the slot.
	
	@param plant a code-planter
    @param id an ident-record 

*/
void vmiNEWID( Plant plant, Ident id ) {
	if ( id->isShared() ) {
		vmiSYS_CALL_DAT( plant, sysMakeIndirection, id->getFinalSlot() );
	}
}

/** vmiCOPYID only affects mutable outer variables. If the
	shared flag is set then the variable will be implemented
	as a pointer to an Indirection.
	
	vmiCOPYID causes the Indirection to be copied. The new
	copy is inserted into the variable's slot. This is useful
	in from-to loops where the outer variable is used to 
	maintain state information, which is really an optimisation.
	
 	@param plant a code-planter
    @param id an ident-record 
*/
void vmiCOPYID( Plant plant, Ident id ) {
	if ( id->isShared() ) {
		vmiSYS_CALL_DAT( plant, sysCopyIndirection, id->getFinalSlot() );
	}
}

void vmiPOP_INNER_SLOT( Plant plant, int slot ) {
	emitSPC( plant, vmc_pop_local );
	emitRef( plant, ToRef( slot ) );	
}

void vmiPOPID( Plant plant, Ident id ) {
	if ( id->isShared() ) {
		vmiSYS_CALL_DAT( plant, sysPopIndirection, id->getFinalSlot() );
	} else if ( id->isLocal() ) {
		vmiPOP_INNER_SLOT( plant, id->getFinalSlot() );
	} else {
		emitSPC( plant, vmc_pop_global );
		emitValof( plant, id->value_of );
	}
}

void vmiPUSH_INNER_SLOT( Plant plant, int slot ) {
	switch ( slot ) {
	case 0:
		emitSPC( plant, vmc_push_local0 );
		return;
	case 1:
		emitSPC( plant, vmc_push_local1 );
		return;
	default:	
		emitSPC( plant, vmc_push_local );
		emitRef( plant, ToRef( slot ) );
	}
}

void vmiPUSHID( Plant plant, Ident id ) {
	if ( id->isShared() ) {
		vmiSYS_CALL_DAT( plant, sysPushIndirection, id->getFinalSlot() );
	} else if ( id->isLocal() ) {
		vmiPUSH_INNER_SLOT( plant, id->getFinalSlot() );
	} else {
		emitSPC( plant, vmc_push_global );
		emitValof( plant, id->value_of );
	}
}

void vmiSET( Plant plant, int A ) {
	emitSPC( plant, vmc_set );
	emitRef( plant, ToRef( A ) );
}

void vmiINVOKE( Plant plant ) {
	emitSPC( plant, vmc_invoke );
	emitRef( plant, sys_absent );	//	Cache.
}

void vmiCALLS( Plant plant ) {
	emitSPC( plant, vmc_calls );
}

void vmiEND_CALL_ID( Plant plant, int var, Ident ident ) {
	if ( ident->isLocal() ) {
		vmiEND_MARK( plant, var );
		vmiPUSHID( plant, ident );
		emitSPC( plant, vmc_calls );
	} else {
		emitSPC( plant, vmc_end_call_global );
		emitRef( plant, ToRef( var ) );
		emitValof( plant, ident->value_of );
	}
}

void vmiSET_CALL_ID( Plant plant, int in_arity, Ident ident ) {
	if ( ident->isLocal() ) {
		vmiSET( plant, in_arity );
		vmiPUSHID( plant, ident );
		emitSPC( plant, vmc_calls );
	} else {
		emitSPC( plant, vmc_set_call_global );
		emitRef( plant, ToRef( in_arity ) );
		emitValof( plant, ident->value_of );
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

void vmiSTART_MARK( Plant plant, int v ) {
	emitSPC( plant, vmc_start_mark );
	emitRef( plant, ToRef( v ) );
}

void vmiEND_MARK( Plant plant, int v ) {
	emitSPC( plant, vmc_end_mark );
	emitRef( plant, ToRef( v ) );
}

void vmiCHECK_COUNT( Plant plant, int v ) {
	emitSPC( plant, vmc_check_count );
	emitRef( plant, ToRef( v ) );
}

void vmiCHECK_MARK1( Plant plant, int v ) {
	emitSPC( plant, vmc_check_mark1 );
	emitRef( plant, ToRef( v ) );
}

//	Do we ever generate this?
void vmiCHECK_MARK0( Plant plant, int v ) {
	emitSPC( plant, vmc_check_mark0 );
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

Ref vmiENDFUNCTION( Plant plant, bool in_heap, Ref fnkey ) {
	Ref r;

	r = plant->detach( in_heap, fnkey );
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

Ref vmiENDFUNCTION( Plant plant ) {
	return vmiENDFUNCTION( plant, true, sysFunctionKey );
}

Ref vmiENDFUNCTION( Plant plant, Ref fnkey ) {
	return vmiENDFUNCTION( plant, true, fnkey );
}

Ref vmiENDFUNCTION( Plant plant, bool in_heap ) {
	return vmiENDFUNCTION( plant, in_heap, in_heap ? sysFunctionKey : sysCoreFunctionKey );
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

static void vmiCMP_ID_CONSTANT( bool flag, Plant plant, Ident id, Ref r, DestinationClass & d ) {
	emitSPC( plant, ( flag ? vmc_eq_si : vmc_neq_si ) );
	if ( id->isLocal() ) {
		emitRef( plant, ToRef( id->getFinalSlot() ) );
	} else {
		emitValof( plant, id->value_of );
	}
	emitRef( plant, r );
	d.destinationInsert();
}

static void vmiCMP_ID_ID( bool flag, Plant plant, Ident id1, Ident id2, DestinationClass & d ) {
	emitSPC( plant, flag ? vmc_eq_ss : vmc_neq_ss );
	if ( id1->isLocal() ) {
		emitRef( plant, ToRef( id1->getFinalSlot() ) );
	} else {
		emitValof( plant, id1->value_of );
	}
	if ( id2->isLocal() ) {
		emitRef( plant, ToRef( id2->getFinalSlot() ) );
	} else {
		emitValof( plant, id2->value_of );
	}
	d.destinationInsert();
}

void vmiIF_EQ_ID_CONSTANT( Plant plant, Ident id, Ref constant, DestinationClass & d ) {
	vmiCMP_ID_CONSTANT( true, plant, id, constant, d );
}

void vmiIF_EQ_ID_ID( Plant plant, Ident id1, Ident id2, DestinationClass & d ) {
	vmiCMP_ID_ID( true, plant, id1, id2, d );
}

void vmiIF_NEQ_ID_CONSTANT( Plant plant, Ident id, Ref constant, DestinationClass & d ) {
	vmiCMP_ID_CONSTANT( false, plant, id, constant, d );
}

void vmiIF_NEQ_ID_ID( Plant plant, Ident id1, Ident id2, DestinationClass & d ) {
	vmiCMP_ID_ID( false, plant, id1, id2, d );
}

//void vmiEQQ( Plant plant, Ref ref ) {
//	emitSPC( plant, vmc_eqq );
//	emitRef( plant, ref );
//}

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
		emitRef( plant, LongToSmall( arg2 ) );
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
			emitRef( this->plant, LongToSmall( this->int1 ) );
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
			emitRef( this->plant, LongToSmall( this->int2 ) );
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
		( this->flag1 != 's' || this->ident1->isLocal() ) &&
		( this->flag2 != 's' || !this->ident2->isLocal() )
	) {
		int arg1 = this->flag1 == 'i' ? this->int1 : this->ident1->getFinalSlot();
		int arg2 = this->flag1 == 'i' ? this->int2 : this->ident2->getFinalSlot();
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
	if ( this->flag1 == 's' && !this->ident1->isLocal() ) throw;
	if ( this->flag2 == 's' && !this->ident2->isLocal() ) throw;
	if ( 
		( this->flag1 != 's' || this->ident1->isLocal() ) &&
		( this->flag2 != 's' || !this->ident2->isLocal() )
	) {
		int arg1 = this->flag1 == 'i' ? this->int1 : this->ident1->getFinalSlot();
		int arg2 = this->flag1 == 'i' ? this->int2 : this->ident2->getFinalSlot();
		vmiIFNOT_RELOP( this->plant, this->flag1, arg1, this->op, this->flag2, arg2, dst );
	} else {
		this->compilePushLeft();
		this->compilePushRight();
		this->compileOp();
		vmiIFNOT( this->plant, dst );
	}
}



