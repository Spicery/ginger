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

static void this_never_happens() {
	throw Ginger::Unreachable( __FILE__, __LINE__ );
}

void PlantClass::vmiINSTRUCTION( Instruction instr ) {
	this->emitSPC( instr );
}

void PlantClass::vmiSYS_CALL( SysCall * r ) {
	this->emitSPC( vmc_syscall );
	this->emitRef( ToRef( r ) );
}

/**
	Plants a non-garbage collectable reference as extra data for
	the system call, to be accessed via pc[-1].
	@param this the code-planter
	@param r the system call
	@param data arbitrary data, size compatible with void*
*/
void PlantClass::vmiSYS_CALL_ARG( SysCall * sys, Ref ref ) {
	this->emitSPC( vmc_syscall_arg );
	this->emitRef( ToRef( sys ) );
	this->emitRef( ref );
}

void PlantClass::vmiSYS_CALL_DAT( SysCall * sys, unsigned long data ) {
	this->emitSPC( vmc_syscall_dat );
	this->emitRef( ToRef( sys ) );
	this->emitRef( ToRef( data ) );
}

void PlantClass::vmiSYS_CALL_ARGDAT( SysCall * sys, Ref ref, unsigned long data ) {
	this->emitSPC( vmc_syscall_argdat );
	this->emitRef( ToRef( sys ) );
	this->emitRef( ref );
	this->emitRef( ToRef( data ) );
}


void PlantClass::vmiSET_SYS_CALL( SysCall * r, int A ) {
	this->emitSPC( vmc_set_syscall );
	this->emitRef( ToRef( A ) );
	this->emitRef( ToRef( r ) );
}

void PlantClass::vmiSYS_RETURN() {
	this->emitSPC( vmc_sysreturn );
}

void PlantClass::vmiOPERATOR( Functor fnc ) {
	this->vmiINSTRUCTION(
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

void PlantClass::vmiFIELD( long index ) {
	this->emitSPC( vmc_field );
	this->emitRef( ToRef( index ) );
}


void PlantClass::vmiINCR( int n ) {
	switch ( n ) {
		case 0: {
			return;
		}
		case 1: {
			this->emitSPC( vmc_incr );
			return;
		}
		case -1: {
			this->emitSPC( vmc_decr );
			return;
		}
		default: {
			this->emitSPC( vmc_incr_by );
			this->emitRef( ToRef( LongToSmall( n ) ) );
		}
	}
}

void PlantClass::vmiCHAIN_LITE( Ref fn, long N ) {
	this->emitSPC( vmc_chainlite );
	this->emitRef( fn );
	this->emitRef( ToRef( N ) );
}

/** vmiNEWID only affects mutable outer variables. If a
	variable is determined to be mutable and outer it is
	potentially shareable between different nested functions.
	
	If the shared flag is set, then the variable is implemented
	as a pointer to a special type of heap object, an Indirection.
	
	vmiNEWID allocates the Indirection, initialises its value to
	the constant -undef-, and assigns that to the slot.
	
	@param this a code-planter
    @param id an ident-record 

*/
void PlantClass::vmiNEWID( Ident id ) {
	if ( id->isShared() ) {
		this->vmiSYS_CALL_DAT( sysMakeIndirection, id->getFinalSlot() );
	}
}

/** vmiCOPYID only affects mutable outer variables. If the
	shared flag is set then the variable will be implemented
	as a pointer to an Indirection.
	
	vmiCOPYID causes the Indirection to be copied. The new
	copy is inserted into the variable's slot. This is useful
	in from-to loops where the outer variable is used to 
	maintain state information, which is really an optimisation.
	
 	@param this a code-planter
    @param id an ident-record 
*/
void PlantClass::vmiCOPYID( Ident id ) {
	if ( id->isShared() ) {
		this->vmiSYS_CALL_DAT( sysCopyIndirection, id->getFinalSlot() );
	}
}

void PlantClass::vmiPOP_INNER_SLOT( int slot ) {
	this->emitSPC( vmc_pop_local );
	this->emitRef( ToRef( slot ) );	
}

void PlantClass::vmiPOPID( Ident id ) {
	if ( id->isShared() ) {
		this->vmiSYS_CALL_DAT( sysPopIndirection, id->getFinalSlot() );
	} else if ( id->isLocal() ) {
		this->vmiPOP_INNER_SLOT( id->getFinalSlot() );
	} else {
		this->emitSPC( vmc_pop_global );
		this->emitValof( id->value_of );
	}
}

void PlantClass::vmiPUSH_INNER_SLOT( int slot ) {
	switch ( slot ) {
	case 0:
		this->emitSPC( vmc_push_local0 );
		return;
	case 1:
		this->emitSPC( vmc_push_local1 );
		return;
	default:	
		this->emitSPC( vmc_push_local );
		this->emitRef( ToRef( slot ) );
	}
}

void PlantClass::vmiPUSHID( Ident id ) {
	if ( id->isShared() ) {
		this->vmiSYS_CALL_DAT( sysPushIndirection, id->getFinalSlot() );
	} else if ( id->isLocal() ) {
		this->vmiPUSH_INNER_SLOT( id->getFinalSlot() );
	} else {
		this->emitSPC( vmc_push_global );
		this->emitValof( id->value_of );
	}
}

void PlantClass::vmiDEREF() {
	this->vmiSYS_CALL( sysIndirectionCont );
}

void PlantClass::vmiMAKEREF() {
	this->vmiSYS_CALL( sysNewIndirection );
}

void PlantClass::vmiSETCONT() {
	this->vmiSYS_CALL( sysSetIndirectionCont );
}

void PlantClass::vmiSET( int A ) {
	this->emitSPC( vmc_set );
	this->emitRef( ToRef( A ) );
}

void PlantClass::vmiINVOKE() {
	this->emitSPC( vmc_invoke );
	this->emitRef( SYS_ABSENT );	//	Cache.
	this->emitRef( SYS_ABSENT );	//	Method table.
}

void PlantClass::vmiCALLS() {
	this->emitSPC( vmc_calls );
}

void PlantClass::vmiEND_CALL_ID( int var, Ident ident ) {
	if ( ident->isLocal() ) {
		this->vmiEND_MARK( var );
		this->vmiPUSHID( ident );
		this->emitSPC( vmc_calls );
	} else {
		this->emitSPC( vmc_end_call_global );
		this->emitRef( ToRef( var ) );
		this->emitValof( ident->value_of );
	}
}

void PlantClass::vmiSET_CALL_ID( int in_arity, Ident ident ) {
	if ( ident->isLocal() ) {
		this->vmiSET( in_arity );
		this->vmiPUSHID( ident );
		this->emitSPC( vmc_calls );
	} else {
		this->emitSPC( vmc_set_call_global );
		this->emitRef( ToRef( in_arity ) );
		this->emitValof( ident->value_of );
	}
}

void PlantClass::vmiEND1_CALLS( int var ) {
	this->emitSPC( vmc_end1_calls );
	this->emitRef( ToRef( var ) );
}

void PlantClass::vmiSET_CALLS( int in_arity ) {
	this->emitSPC( vmc_set_calls );
	this->emitRef( ToRef( in_arity ) );
}

void PlantClass::vmiSTART_MARK( int v ) {
	this->emitSPC( vmc_start_mark );
	this->emitRef( ToRef( v ) );
}

void PlantClass::vmiEND_MARK( int v ) {
	this->emitSPC( vmc_end_mark );
	this->emitRef( ToRef( v ) );
}

void PlantClass::vmiCHECK_COUNT( int v ) {
	this->emitSPC( vmc_check_count );
	this->emitRef( ToRef( v ) );
}

void PlantClass::vmiCHECK_MARK1( int v ) {
	this->emitSPC( vmc_check_mark1 );
	this->emitRef( ToRef( v ) );
}

//	Do we ever generate this?
void PlantClass::vmiCHECK_MARK0( int v ) {
	this->emitSPC( vmc_check_mark0 );
	this->emitRef( ToRef( v ) );
}

void PlantClass::vmiPUSHQ( Ref obj ) {
	this->emitSPC( vmc_pushq );
	this->emitRef( obj );
}


void PlantClass::vmiRETURN() {
	this->emitSPC( vmc_return );
}

void PlantClass::vmiENTER() {
	this->emitSPC(
		this->ninputs == 0 ? vmc_enter0 :
		this->ninputs == 1 ? vmc_enter1 :
		vmc_enter
	);
}

void PlantClass::vmiFUNCTION( int N, int A ) {
	this->vm->gcVeto();
	this->save( N, A );
}

Ref PlantClass::vmiENDFUNCTION( bool in_heap, Ref fnkey ) {
	Ref r;

	r = this->detach( in_heap, fnkey );
	#ifndef DBG_VMI
		if ( this->vm->getShowCode() ) {
			this->vm->printfn( std::clog, r );
		}
	#else
		this->vm->printfn( std::clog, r );
		Ref * p = RefToPtr4( r );
		std::clog << "Scanning ... " << std::endl;
		ScanFunc scan( this->vm->instructionSet(), p );
		for (;;) {
			Ref * pc = scan.next();
			if ( !pc ) break;
			std::clog << "Ref at offset " << ( pc - p ) << std::endl;
		}
	#endif
	this->restore();
	
	this->vm->gcLiftVeto();
	return r;
}

Ref PlantClass::vmiENDFUNCTION() {
	return this->vmiENDFUNCTION( true, sysFunctionKey );
}

Ref PlantClass::vmiENDFUNCTION( Ref fnkey ) {
	return this->vmiENDFUNCTION( true, fnkey );
}

Ref PlantClass::vmiENDFUNCTION( bool in_heap ) {
	return this->vmiENDFUNCTION( in_heap, in_heap ? sysFunctionKey : sysCoreFunctionKey );
}

void PlantClass::vmiNOT() {
	this->emitSPC( vmc_not );
}

void PlantClass::vmiIFNOT( DestinationClass & d ) {
	this->emitSPC( vmc_ifnot );
	d.destinationInsert();
}

void PlantClass::vmiIFSO( DestinationClass & d ) {
	this->emitSPC( vmc_ifso );
	d.destinationInsert();
}

void PlantClass::vmiIF( bool sense, DestinationClass & dst ) {
	if ( sense ) {
		this->vmiIFSO( dst );
	} else {
		this->vmiIFNOT( dst );
	}
}

void PlantClass::vmiGOTO( DestinationClass & d ) {
	this->emitSPC( vmc_goto );
	d.destinationInsert();
}

void PlantClass::vmiCMP_ID_CONSTANT( bool flag, Ident id, Ref r, DestinationClass & d ) {
	this->emitSPC( ( flag ? vmc_eq_si : vmc_neq_si ) );
	if ( id->isLocal() ) {
		this->emitRef( ToRef( id->getFinalSlot() ) );
	} else {
		this->emitValof( id->value_of );
	}
	this->emitRef( r );
	d.destinationInsert();
}

void PlantClass::vmiCMP_ID_ID( bool flag, Ident id1, Ident id2, DestinationClass & d ) {
	this->emitSPC( flag ? vmc_eq_ss : vmc_neq_ss );
	if ( id1->isLocal() ) {
		this->emitRef( ToRef( id1->getFinalSlot() ) );
	} else {
		this->emitValof( id1->value_of );
	}
	if ( id2->isLocal() ) {
		this->emitRef( ToRef( id2->getFinalSlot() ) );
	} else {
		this->emitValof( id2->value_of );
	}
	d.destinationInsert();
}

void PlantClass::vmiIF_EQ_ID_CONSTANT( Ident id, Ref constant, DestinationClass & d ) {
	this->vmiCMP_ID_CONSTANT( true, id, constant, d );
}

void PlantClass::vmiIF_EQ_ID_ID( Ident id1, Ident id2, DestinationClass & d ) {
	this->vmiCMP_ID_ID( true, id1, id2, d );
}

void PlantClass::vmiIF_NEQ_ID_CONSTANT( Ident id, Ref constant, DestinationClass & d ) {
	this->vmiCMP_ID_CONSTANT( false, id, constant, d );
}

void PlantClass::vmiIF_NEQ_ID_ID( Ident id1, Ident id2, DestinationClass & d ) {
	this->vmiCMP_ID_ID( false, id1, id2, d );
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

void PlantClass::vmiIF_RELOP( bool sense, char flag1, int arg1, char op, char flag2, int arg2, DestinationClass & dst ) {
	if ( sense ) {
		this->vmiIFSO_RELOP( flag1, arg1, op, flag2, arg2, dst );
	} else {
		this->vmiIFNOT_RELOP( flag1, arg1, op, flag2, arg2, dst );
	}
}

void PlantClass::vmiIFSO_RELOP( char flag1, int arg1, char op, char flag2, int arg2, DestinationClass & dst ) {
	if ( flag1 == 'i' && flag2 == 'i' ) {
		if ( eval_relop( op, arg1, arg2 ) ) this->vmiGOTO( dst );
	} else if ( flag1 == 'i' && flag2 == 's' ) {
		this->vmiIFSO_RELOP( flag2, arg2, rev_relop( op ), flag1, arg1, dst );
	} else if ( flag1 == 's' && flag2 == 'i' ) {
		this->emitSPC(
		    op == 'g' ? vmc_gte_si :
		    op == '>' ? vmc_gt_si :
		    op == '=' ? vmc_eq_si :
			op == '!' ? vmc_neq_si :
		    op == '<' ? vmc_lt_si :
		    op == 'l' ? vmc_lte_si :
		    ( this_never_happens(), (Instruction)0 )
		);
		this->emitRef( ToRef( arg1 ) );
		this->emitRef( LongToSmall( arg2 ) );
		dst.destinationInsert();
	} else if ( flag1 == 's' && flag2 == 's' ) {
		this->emitSPC(
		    op == 'g' ? vmc_gte_ss :
		    op == '>' ? vmc_gt_ss :
		    op == '=' ? vmc_eq_ss :
			op == '!' ? vmc_neq_ss :
		    op == '<' ? vmc_lt_ss :
		    op == 'l' ? vmc_lte_ss :
		    ( this_never_happens(), (Instruction)0 )
		);
		this->emitRef( ToRef( arg1 ) );
		this->emitRef( ToRef( arg2 ) );
		dst.destinationInsert();
	} else {
		this_never_happens();
	}
}

void PlantClass::vmiIFNOT_RELOP( char flag1, int arg1, char op, char flag2, int arg2, DestinationClass & dst ) {
	this->vmiIFSO_RELOP( flag1, arg1, rev_relop( op ), flag2, arg2, dst );
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
			this->plant->emitRef( LongToSmall( this->int1 ) );
			break;
		}
		case 's': {
			this->plant->vmiPUSHID( this->ident1 );
			break;
		}
		default: throw;			//	Never happens.
	}
}

void VmiRelOpFactory::compilePushRight() {
	switch ( this->flag2 ) {
		case 'i': {
			this->plant->emitRef( LongToSmall( this->int2 ) );
			break;
		}
		case 's': {
			this->plant->vmiPUSHID( this->ident2 );
			break;
		}
		default: throw;			//	Never happens.
	}
}

void VmiRelOpFactory::compileOp() {
	this->plant->vmiOPERATOR( 
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
		this->plant->vmiIFSO_RELOP( this->flag1, arg1, this->op, this->flag2, arg2, dst );
	} else {
		this->compilePushLeft();
		this->compilePushRight();
		this->compileOp();
		this->plant->vmiIFSO( dst );
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
		this->plant->vmiIFNOT_RELOP( this->flag1, arg1, this->op, this->flag2, arg2, dst );
	} else {
		this->compilePushLeft();
		this->compilePushRight();
		this->compileOp();
		this->plant->vmiIFNOT( dst );
	}
}



