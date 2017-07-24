/******************************************************************************\
	Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
	of the GNU General Public License. This file is part of Ginger.

	Ginger is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Ginger is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Ginger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

#include "debug.hpp"

#include "numbers.hpp"
#include "codegen.hpp"

#include <iostream>
#include <sstream>

#include <stddef.h>
#include <string.h>

#include "numbers.hpp"
#include "gnxconstants.hpp"
#include "mnx.hpp"
#include "mishap.hpp"
#include "arity.hpp"

#include "label.hpp"
#include "instruction.hpp"
#include "heap.hpp"
#include "machine.hpp"
#include "key.hpp"
#include "arity.hpp"
#include "makesysfn.hpp"
#include "misclayout.hpp"
#include "sys.hpp"
#include "syssymbol.hpp"
#include "syscheck.hpp"
#include "sysexception.hpp"
#include "syslist.hpp"
#include "sysindirection.hpp"
#include "maybe.hpp"

namespace Ginger {
using namespace std;


#ifndef NULL
#define NULL 0
#endif

CodeGenClass::CodeGenClass( Machine vm ) {
	this->nlocals = 0;
	this->ninputs = 0;
	this->current_slot = 0;
	this->vm = vm;
}

void CodeGenClass::continueFrom( LabelClass * contn ) {
	if ( contn == CONTINUE_LABEL ) {
		//	Nothing needed, drop through.
	} else if ( contn->isReturn() ) {
		this->vmiRETURN();
	} else {
		this->vmiGOTO( contn );
	}
}

const InstructionSet & CodeGenClass::instructionSet() {
	return this->vm->instructionSet();
}

LabelClass * CodeGenClass::newLabel() {
	this->label_vector.push_back( LabelClass( this ) );
	return &this->label_vector.back();
}

void CodeGenClass::emitSPC( Instruction instr ) {
	const InstructionSet & ins = this->instructionSet();
	Ref instr_ptr = ins.lookup( instr );
	this->codegenRef( instr_ptr );
}

void CodeGenClass::emitRef( Ref ref ) {
	this->codegenRef( ref );
}

void CodeGenClass::emitValof( Valof * v ) {
	this->codegenRef( ToRef( v ) );
}

Ref * sysCheckExplodeN( Ref * pc, MachineClass * vm ) {
	const long N = ToLong( pc[ -1 ] );
	Ref * stack = vm->vp;
	pc = sysExplode( pc, vm );
	if ( N == vm->vp - stack ) return pc;
	throw Mishap( "Wrong number of arguments from explode" ).culprit( "Expected", N ).culprit( "Actual", vm->vp - stack );
}

Ref * sysCheckExplodeGteN( Ref * pc, MachineClass * vm ) {
	const long N = ToLong( pc[ -1 ] );
	Ref * stack = vm->vp;
	pc = sysExplode( pc, vm );
	if ( N <= vm->vp - stack ) return pc;
	throw Mishap( "Wrong number of arguments from explode" ).culprit( "Expected at least", N ).culprit( "Actual", vm->vp - stack );
}

//	Check that what is on the stack is consistent with a given arity.
void CodeGenClass::vmiCHECK_EXPLODE( Arity arity ) {
	const int N = arity.count();
	if ( arity.isExact() ) {
		this->emitSPC( vmc_syscall_arg );
		this->emitRef( ToRef( sysCheckExplodeN ) );
		this->emitRef( IntToRef( N ) );
	} else if ( N > 0 ) {
		//	N.B. We only plant code if the N > 0 as N == 0 is trivially true.
		this->emitSPC( vmc_syscall_arg );
		this->emitRef( ToRef( sysCheckExplodeGteN ) );
		this->emitRef( IntToRef( N ) );
	}
}

void CodeGenClass::emitVAR_REF( Gnx id ) {
	if ( id->hasAttribute( GNX_VID_SCOPE, "local" ) ) {
		this->emitRef( IntToRef( id->attributeToInt( GNX_VID_SLOT ) ) );
	} else {
		this->emitValof( resolveGlobal( id ) );
	}
}

void CodeGenClass::emitVIDENT_REF( const VIdent & id ) {
	if ( id.isLocal() ) {
		this->emitRef( IntToRef( id.getSlot() ) );
	} else if ( id.isGlobal() ) {
		this->emitValof( id.getValof() );
	} else if ( id.isConstant() ) {
		this->emitRef( id.getRef() );
	} else {
		throw SystemError( "Internal error (1)" );
	}
}

//	TODO: Avoid compiling the message string into the heap. What would be 
//	a lot cuter is to plant a pointer to an "external" object.
//		class SysFunObj {
//			virtual Ref * run( Ref * pc, MachineClass * vm ) = 0;
//			virtual ~SysFunObj() { deregister & free any resources }
//		}
//	This would need some integration with the garbage collector along
//	with a new instruction-field marker.
void CodeGenClass::vmiFAIL() {
	this->emitSPC( vmc_fail );
}

void CodeGenClass::vmiINSTRUCTION( Instruction instr ) {
	this->emitSPC( instr );
}

void CodeGenClass::vmiSELF_CONSTANT() {
	this->emitSPC( vmc_self_constant );
}

void CodeGenClass::vmiSELF_CALL() {
	this->emitSPC( vmc_self_call );
}

void CodeGenClass::vmiSELF_CALL_N( const int n ) {
	this->emitSPC( vmc_self_call_n );
	this->emitRef( IntToRef( n ) );
}

void CodeGenClass::vmiSYS_CALL( SysCall * r ) {
	this->emitSPC( vmc_syscall );
	this->emitRef( ToRef( r ) );
}

/**
	CodeGens a non-garbage collectable reference as extra data for
	the system call, to be accessed via pc[-1].
	@param this the code-codegener
	@param r the system call
	@param data arbitrary data, size compatible with void*
*/
void CodeGenClass::vmiSYS_CALL_ARG( SysCall * sys, Ref ref ) {
	this->emitSPC( vmc_syscall_arg );
	this->emitRef( ToRef( sys ) );
	this->emitRef( ref );
}

void CodeGenClass::vmiSYS_CALL_DAT( SysCall * sys, unsigned long data ) {
	this->emitSPC( vmc_syscall_dat );
	this->emitRef( ToRef( sys ) );
	this->emitRef( ToRef( data ) );
}

void CodeGenClass::vmiSYS_CALL_ARGDAT( SysCall * sys, Ref ref, unsigned long data ) {
	this->emitSPC( vmc_syscall_argdat );
	this->emitRef( ToRef( sys ) );
	this->emitRef( ref );
	this->emitRef( ToRef( data ) );
}


void CodeGenClass::vmiSET_SYS_CALL( SysCall * r, int A ) {
	this->emitSPC( vmc_set_count_syscall );
	this->emitRef( IntToRef( A ) );
	this->emitRef( ToRef( r ) );
}

void CodeGenClass::vmiSYS_RETURN() {
	this->emitSPC( vmc_sysreturn );
}


void CodeGenClass::vmiFIELD( long index ) {
	this->emitSPC( vmc_field );
	this->emitRef( ToRef( index ) );
}


void CodeGenClass::vmiINCR( long n ) {
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
			//	We have to be a little careful here because we only support
			//	Smalls and not the entire range of Long.
			this->emitSPC( vmc_incr_by );
			if ( canFitInSmall( n ) ) {
				this->emitRef( LongToSmall( n ) );
			} else {
				throw Mishap( "Increment is out of range" ).culprit( "Increment", n );
			}
		}
	}
}

void CodeGenClass::vmiCHAIN_LITE( Ref fn, long N ) {
	this->emitSPC( vmc_chainlite );
	this->emitRef( fn );
	this->emitRef( ToRef( N ) );
}


Valof * CodeGenClass::resolveGlobal( Gnx id ) {
	Package * def_pkg = this->vm->getPackage( id->attribute( GNX_VID_DEF_PKG ) );
	return def_pkg->fetchAbsoluteValof( id->attribute( GNX_VID_NAME ) );
}

void CodeGenClass::vmiPOP_INNER_SLOT( int slot ) {
	this->emitSPC( vmc_pop_local );
	this->emitRef( IntToRef( slot ) );	
}

void CodeGenClass::vmiPOP( const VIdent & id, const bool assign_vs_bind ) {
	if ( id.isLocal() ) {
		this->vmiPOP_INNER_SLOT( id.getSlot() );
	} else if ( id.isGlobal() ) {
		Valof * v = id.getValof();
		if ( assign_vs_bind && v->isProtected() ) {
			throw Ginger::Mishap( "Assigning to a protected variable" ).culprit( "Variable", v->getNameString() );
		}
		this->emitSPC( vmc_pop_global );
		this->emitValof( v );
	} else {
		throw SystemError( "Internal Error (2)" );
	}
}

/*bool CodeGenClass::vmiTRY_POP( const VIdent & id, const bool assign_vs_bind, LabelClass * dst, LabelClass * contn ) {
	if ( id.isLocal() ) {
		this->vmiPOP_INNER_SLOT( id.getSlot() );
		this->continueFrom( dst );
		return false;
	} else if ( id.isGlobal() ) {
		Valof * v = id.getValof();
		if ( assign_vs_bind && v->isProtected() ) {
			throw Ginger::Mishap( "Assigning to a protected variable" ).culprit( "Variable", v->getNameString() );
		}
		this->emitSPC( vmc_pop_global );
		this->emitValof( v );
		this->continueFrom( dst );
		return false;
	} else if ( id.isConstant() ) {
		this->vmiIFEQTO( id.getRef(), dst, contn );
		return true;
	} else {
		throw SystemError( "Internal Error (2)" );
	}
}*/

void CodeGenClass::vmiPOP( Gnx var_or_id, const bool assign_vs_bind ) {
	#ifdef DBG_CODEGEN
		cerr << "POP ";
		var_or_id->render( cerr );
		cerr << endl;
	#endif
	VIdent vid( this, var_or_id );
	this->vmiPOP( vid, assign_vs_bind );
}

void CodeGenClass::vmiPUSH( Gnx var_or_id ) {
	#ifdef DBG_CODEGEN
		cerr << "PUSH ";
		var_or_id->render( cerr );
		cerr << endl;		
	#endif
	VIdent vid( this, var_or_id );
	this->vmiPUSH( vid );
}

void CodeGenClass::vmiPUSH( const VIdent & vid ) {
	if ( vid.isConstant() ) {
		this->vmiPUSHQ( vid.getRef() );
	} else if ( vid.isLocal() ) {
		this->vmiPUSH_INNER_SLOT( vid.getSlot() );
	} else if ( vid.isGlobal() ) {
		this->emitSPC( vmc_push_global );
		this->emitValof( vid.getValof() );
	} else {
		throw SystemError( "Internal Error( 3 )" );
	}
}

void CodeGenClass::vmiPUSH( const VIdent & vid, LabelClass * contn ) {
	if ( vid.isConstant() ) {
		this->vmiPUSHQ( vid.getRef(), contn );
	} else if ( vid.isLocal() ) {
		this->vmiPUSH_INNER_SLOT( vid.getSlot(), contn );
	} else if ( vid.isGlobal() ) {
		this->emitSPC( vmc_push_global );
		this->emitValof( vid.getValof() );
		this->continueFrom( contn );
	} else {
		throw SystemError( "Internal Error (4)" );
	}
}

void CodeGenClass::vmiPUSH_INNER_SLOT( int slot ) {
	switch ( slot ) {
	case 0:
		this->emitSPC( vmc_push_local0 );
		return;
	case 1:
		this->emitSPC( vmc_push_local1 );
		return;
	default:	
		this->emitSPC( vmc_push_local );
		this->emitRef( IntToRef( slot ) );
	}
}

void CodeGenClass::vmiPUSH_INNER_SLOT( int slot, LabelClass * contn ) {
	const bool is_ret = contn != NULL and contn->isReturn();
	switch ( slot ) {
		case 0:
			this->emitSPC( is_ret ? vmc_push_local0_ret : vmc_push_local0 );
			return;
		case 1:
			this->emitSPC( is_ret ? vmc_push_local1_ret : vmc_push_local1 );
			return;
		default:	
			this->emitSPC( is_ret ? vmc_push_local_ret : vmc_push_local );
			this->emitRef( IntToRef( slot ) );
	}
}

void CodeGenClass::vmiDEREF() {
	this->vmiSYS_CALL( sysIndirectionCont );
}

void CodeGenClass::vmiMAKEREF() {
	this->vmiSYS_CALL( sysNewIndirection );
}

void CodeGenClass::vmiSETCONT() {
	this->vmiSYS_CALL( sysSetIndirectionCont );
}

void CodeGenClass::vmiSET_COUNT_TO_MARK( int A ) {
	this->emitSPC( vmc_set_count_mark );
	this->emitRef( IntToRef( A ) );
}

void CodeGenClass::vmiINVOKE() {
	this->emitSPC( vmc_invoke );
	this->emitRef( SYS_ABSENT );	//	Cache.
	this->emitRef( SYS_ABSENT );	//	Method table.
}

void CodeGenClass::vmiCALLS() {
	this->emitSPC( vmc_calls );
}

void CodeGenClass::vmiEND_CALL_ID( int var, const VIdent & ident ) {
	if ( ident.isLocal() ) {
		this->vmiSET_COUNT_TO_MARK( var );
		this->vmiPUSH( ident );
		this->emitSPC( vmc_calls );
	} else if ( ident.isGlobal() ) {
		this->emitSPC( vmc_end_call_global );
		this->emitRef( IntToRef( var ) );
		this->emitVIDENT_REF( ident );
	} else {
		throw UnreachableError();
	}
}

void CodeGenClass::vmiSET_CALL_ID( int in_arity, const VIdent & ident ) {
	if ( ident.isLocal() ) {
		this->emitSPC( vmc_set_count_call_local );
	} else if ( ident.isGlobal() ) {
		this->emitSPC( vmc_set_count_call_global );
	} else {
		throw UnreachableError();
	}
	this->emitRef( IntToRef( in_arity ) );
	this->emitVIDENT_REF( ident );
}

//	Not very efficient :(
void CodeGenClass::vmiSET_CALL_INNER_SLOT( int in_arity, int slot ) {
	this->vmiSET_COUNT_TO_MARK( in_arity );
	this->vmiPUSH_INNER_SLOT( slot );
	this->emitSPC( vmc_calls );
}


void CodeGenClass::vmiEND1_CALLS( int var ) {
	this->emitSPC( vmc_end1_calls );
	this->emitRef( IntToRef( var ) );
}

void CodeGenClass::vmiSET_CALLS( int in_arity ) {
	this->emitSPC( vmc_set_count_calls );
	this->emitRef( IntToRef( in_arity ) );
}

void CodeGenClass::vmiSTART_MARK( int v ) {
	this->emitSPC( vmc_start_mark );
	this->emitRef( IntToRef( v ) );
}

void CodeGenClass::vmiEND_MARK( int v ) {
	this->emitSPC( vmc_end_mark );
	this->emitRef( IntToRef( v ) );
}

void CodeGenClass::vmiERASE_MARK( int var ) {
	this->emitSPC( vmc_erase_mark );
	this->emitRef( IntToRef( var ) );
}

void CodeGenClass::vmiERASE() {
	this->emitSPC( vmc_erase );
}

void CodeGenClass::vmiDUP() {
	this->emitSPC( vmc_dup );
}

void CodeGenClass::vmiCHECK_COUNT( int v ) {
	this->emitSPC( vmc_check_count );
	this->emitRef( IntToRef( v ) );
}

void CodeGenClass::vmiCHECK_MARK( int v, Arity a ) {
	if ( a.isExact() ) {
		this->vmiCHECK_MARK( v, a.count() );
	} else {
		this->emitSPC( vmc_check_mark_gte );
		this->emitRef( IntToRef( v ) );
		this->emitRef( IntToRef( a.count() ) );		
	}
}

void CodeGenClass::vmiCHECK_MARK( int v, int N ) {
	if ( N == 0 ) {
		this->vmiCHECK_MARK0( v );
	} else if ( N == 1 ) {
		this->vmiCHECK_MARK1( v );
	} else {
		this->emitSPC( vmc_check_mark );
		this->emitRef( IntToRef( v ) );
		this->emitRef( IntToRef( N ) );
	}
}

void CodeGenClass::vmiCHECK_MARK_ELSE( int v, int N, LabelClass * fail_label ) {
	//	TODO: this is not at all good code. Opportunity to improve.
	LabelClass cont_label( this );
	this->emitSPC( vmc_neq_si );
	this->emitRef( IntToRef( v ) );
	this->emitRef( LongToSmall( N ) );
	cont_label.labelInsert();
	this->vmiERASE_MARK( v );
	this->continueFrom( fail_label );
	cont_label.labelSet();
}

void CodeGenClass::vmiCHECK_MARK1( int v ) {
	this->emitSPC( vmc_check_mark1 );
	this->emitRef( IntToRef( v ) );
}

//	Do we ever generate this?
void CodeGenClass::vmiCHECK_MARK0( int v ) {
	this->emitSPC( vmc_check_mark0 );
	this->emitRef( IntToRef( v ) );
}

void CodeGenClass::vmiPUSHQ( Ref obj ) {
	this->emitSPC( vmc_pushq );
	this->emitRef( obj );
}

void CodeGenClass::vmiPUSHQ_RETURN( Ref obj ) {
	this->emitSPC( vmc_pushq_ret );
	this->emitRef( obj );
}

void CodeGenClass::vmiPUSHQ( Ref obj, LabelClass * contn ) {
	if ( contn != NULL and contn->isReturn() ) {
		this->vmiPUSHQ_RETURN( obj );
	} else {
		this->vmiPUSHQ( obj );
		this->continueFrom( contn );
	}
}

void CodeGenClass::vmiPUSHQ_STRING( const std::string & s, LabelClass * contn ) {
	this->vmiPUSHQ( this->vm->heap().copyString( s.c_str() ) );
	this->continueFrom( contn );
}

void CodeGenClass::vmiPUSHQ_SYMBOL( const std::string & s, LabelClass * contn ) {
	this->vmiPUSHQ( refMakeSymbol( s ) );
	this->continueFrom( contn );
}



void CodeGenClass::vmiRETURN() {
	this->emitSPC( vmc_return );
}

void CodeGenClass::vmiHALT() {
	this->emitSPC( vmc_halt );
}

void CodeGenClass::vmiENTER() {
	this->emitSPC(
		this->ninputs == 0 ? vmc_enter0 :
		this->ninputs == 1 ? vmc_enter1 :
		vmc_enter
	);
}

void CodeGenClass::vmiFUNCTION( int N, int A ) {
	this->vmiFUNCTION( EMPTY_FN_NAME, N, A );
}

void CodeGenClass::vmiFUNCTION( const string name, int N, int A ) {
	this->vm->gcVeto();
	this->save( name, N, A );
}



Ref CodeGenClass::vmiENDFUNCTION( bool in_heap, Ref fnkey ) {
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

Ref CodeGenClass::vmiENDFUNCTION() {
	return this->vmiENDFUNCTION( true, sysFunctionKey );
}

Ref CodeGenClass::vmiENDFUNCTION( Ref fnkey ) {
	return this->vmiENDFUNCTION( true, fnkey );
}

Ref CodeGenClass::vmiENDFUNCTION( bool in_heap ) {
	return this->vmiENDFUNCTION( in_heap, in_heap ? sysFunctionKey : sysCoreFunctionKey );
}

void CodeGenClass::vmiNOT() {
	this->emitSPC( vmc_not );
}

void CodeGenClass::vmiAND( LabelClass * dst ) {
	if ( dst == CONTINUE_LABEL ) throw SystemError( "vmiAND passed CONTINUE_LABEL" );
	this->emitSPC( vmc_and );
	dst->labelInsert();
}

void CodeGenClass::vmiOR( LabelClass * dst ) {
	if ( dst == CONTINUE_LABEL ) throw SystemError( "vmiOR passed CONTINUE_LABEL" );
	this->emitSPC( vmc_or );
	dst->labelInsert();
}

void CodeGenClass::vmiABS_AND( LabelClass * dst ) {
	if ( dst == CONTINUE_LABEL ) throw SystemError( "vmiABS_AND passed CONTINUE_LABEL" );
	this->emitSPC( vmc_absand );
	dst->labelInsert();
}

void CodeGenClass::vmiABS_OR( LabelClass * dst ) {
	if ( dst == CONTINUE_LABEL ) throw SystemError( "vmiABS_OR passed CONTINUE_LABEL" );
	this->emitSPC( vmc_absor );
	dst->labelInsert();
}

void CodeGenClass::vmiIFSO( LabelClass * dst ) {
	if ( dst == CONTINUE_LABEL ) {
		throw SystemError( "vmiIFSO called with CONTINUE_LABEL" );
	} else if ( dst->isntReturn() ) {
		this->emitSPC( vmc_ifso );
		dst->labelInsert();
	} else {
		this->emitSPC( vmc_return_ifso );
	}	
}

void CodeGenClass::vmiIFNOT( LabelClass * dst ) {
	if ( dst == CONTINUE_LABEL ) {
		throw SystemError( "vmiIFNOT called with CONTINUE_LABEL" );
	} else if ( dst->isntReturn() ) {
		this->emitSPC( vmc_ifnot );
		dst->labelInsert();
	} else {
		this->emitSPC( vmc_return_ifnot );
	}	
}


void CodeGenClass::vmiIFTEST( const bool sense, LabelClass * dst ) {
	if ( sense ) {
		this->vmiIFSO( dst );
	} else {
		this->vmiIFNOT( dst );
	}
}

/**
 * 	@param dst destination label if top of stack non-false.
 *  @param contn continuation label if top of stack false.
 */
void CodeGenClass::vmiIFSO( LabelClass * d, LabelClass * contn ) {
	if ( contn == CONTINUE_LABEL ) {
		if ( d == CONTINUE_LABEL ) {
			this->emitSPC( vmc_erase );
		} else if ( d->isntReturn() ) {
			this->emitSPC( vmc_ifso );
			d->labelInsert();
		} else {
			this->emitSPC( vmc_return_ifso );
		}
	} else if ( d == CONTINUE_LABEL ) {
		if ( contn->isntReturn() ) {
			this->emitSPC( vmc_ifnot );
			contn->labelInsert();
		} else {
			this->emitSPC( vmc_return_ifnot );
		}
	} else {
		this->emitSPC( vmc_ifso );
		d->labelInsert();
		this->continueFrom( contn );			
	}
}

/**
 * 	@param dst destination label if top of stack false.
 *  @param contn continuation label if top of stack non-false.
 */
void CodeGenClass::vmiIFNOT( LabelClass * d, LabelClass * contn ) {
	this->vmiIFSO( contn, d );
}

void CodeGenClass::vmiIFEQ( LabelClass * dst ) {
	if ( dst == CONTINUE_LABEL ) {
		//	No branching
		this->vmiERASE_NUM( 2 );
	} else if ( dst->isntReturn() ) {
		this->emitSPC( vmc_eq );
		this->emitSPC( vmc_ifso );
		dst->labelInsert();
	} else {
		this->emitSPC( vmc_eq );
		this->emitSPC( vmc_return_ifso );
	}
}

void CodeGenClass::vmiIFNEQ( LabelClass * dst ) {
	if ( dst == CONTINUE_LABEL ) {
		//	No branching
		this->vmiERASE_NUM( 2 );
	} else if ( dst->isntReturn() ) {
		this->emitSPC( vmc_eq );
		this->emitSPC( vmc_ifnot );
		dst->labelInsert();
	} else {
		this->emitSPC( vmc_eq );
		this->emitSPC( vmc_return_ifnot );
	}
}

void CodeGenClass::vmiIFNEQTO( Ref ref, LabelClass * dst, LabelClass *contn ) {
	if ( dst == contn ) {
		this->vmiERASE();
		this->continueFrom( dst );
	} else if ( dst == CONTINUE_LABEL ) {
		this->vmiIFEQTO( ref, contn, dst );
	} else {
		this->emitSPC( vmc_pushq );
		this->emitRef( ref );
		this->vmiIFNEQ( dst );
	}
}

void CodeGenClass::vmiIFEQTO( Ref ref, LabelClass * dst, LabelClass *contn ) {
	if ( dst == contn ) {
		this->vmiERASE();
		this->continueFrom( dst );
	} else if ( dst == CONTINUE_LABEL ) {
		this->vmiIFNEQTO( ref, contn, dst );
	} else {
		this->emitSPC( vmc_pushq );
		this->emitRef( ref );
		this->vmiIFEQ( dst );
	}
}

void CodeGenClass::vmiERASE_NUM( long n ) {
	if ( n == 0 ) {
		//	Do nothing
	} else if ( n == 1 ) {
		this->emitSPC( vmc_erase );
	} else {
		this->emitSPC( vmc_erase_num );
		this->emitRef( ToRef( n ) );
	}
}

/**
 * 	@param dst destination label if top of stack agrees with sense.
 *  @param contn continuation label if top of stack disagrees with sense.
 */
void CodeGenClass::vmiIF( bool sense, LabelClass * dst, LabelClass * contn ) {
	if ( sense ) {
		this->vmiIFSO( dst, contn );
	} else {
		this->vmiIFNOT( dst, contn );
	}
}

/**
 * 	@param dst A proper label (not CONTINUE_LABEL)
 */
void CodeGenClass::vmiGOTO( LabelClass * dst ) {
	if ( dst == CONTINUE_LABEL ) {
		throw SystemError( "Trying to GOTO a fake label" );
	} else if ( dst->isReturn() ) {
		this->emitSPC( vmc_return );
	} else {
		this->emitSPC( vmc_goto );
		dst->labelInsert();
	}
}

/**
 * 	@param dst A proper label (not CONTINUE_LABEL)
 */
void CodeGenClass::vmiBYPASS( LabelClass * dst ) {
	if ( dst == CONTINUE_LABEL ) {
		throw SystemError( "Trying to BYPASS a fake label" );
	} else {
		this->emitSPC( vmc_bypass );
		dst->labelInsert();
	}
}

void CodeGenClass::vmiDECR( const long d ) {
	this->vmiINCR( -d );
}

/**
 * 	@param dst A proper label (not CONTINUE_LABEL)
 */
void CodeGenClass::vmiTEST( 
	const VIdent & vid0, 
	CMP_OP cmp_op, 
	const VIdent & vid1, 
	LabelClass * dst 
) { 
	if ( dst == CONTINUE_LABEL ) {
		throw SystemError( "vmiTEST called with CONTINUE_LABEL" );
	} else if ( vid0.isLocal() and vid1.isLocal() ) {
		this->vmiINSTRUCTION( cmpLocalLocalInstruction( cmp_op ) );
		this->emitVIDENT_REF( vid0 );
		this->emitVIDENT_REF( vid1 );
		dst->labelInsert();
	} else if ( vid0.isLocal() and vid1.isConstant() and IsSmall( vid1.getRef() ) ) {
		this->vmiINSTRUCTION( cmpLocalSmallInstruction( cmp_op ) );
		this->emitVIDENT_REF( vid0 );
		this->emitVIDENT_REF( vid1 );
		dst->labelInsert();
	} else if ( vid1.isLocal() and vid0.isConstant() and IsSmall( vid0.getRef() ) ) {
		this->vmiTEST( vid1, revCmpOp( cmp_op ), vid0, dst );
	} else {
		this->vmiPUSH( vid0 );
		this->vmiPUSH( vid1 );
		this->vmiINSTRUCTION( cmpOpInstruction( cmp_op ) );
		this->emitSPC( vmc_ifso );
		dst->labelInsert();		
	}
}


#define REFBITS ( 8 * sizeof( Ref ) )

Ref CodeGenClass::detach( const bool in_heap, Ref fnkey ) {

	//	Add one for the count and then follow with a succession of bytes.
	unsigned long N = 1 + ( this->props.size() + sizeof( Ref ) - 1 ) / sizeof( Ref );
	
	unsigned long L = this->code_data->size();
	unsigned long preflight_size = OFFSET_FROM_FN_LENGTH_TO_KEY + 1 + L + N;
	
	//	The preflighted size must fit into WORDBITS-8 bits.
	if ( ( preflight_size & ~TAGGG_MASK ) != 0 ) {
		throw Ginger::Mishap( "Procedure too large" );
	}
	
	if ( in_heap ) {

		Ref * fake_pc = NULL;
		XfrClass xfr( fake_pc, *this->vm, preflight_size );
	
		//	L has to be a "procedure-length" value if heap scanning is
		//	to be preserved - and I would like that for a variety of
		//	reasons.
		//
		xfr.xfrRef( ToRef( ( L << TAGGG ) | FUNC_LEN_TAGGG ) );  //	tagged for heap scanning
		xfr.xfrRef( SYS_ABSENT );					//	placeholder for Data Pool.
		xfr.xfrRef( IntToRef( this->nresults ) );		//	raw R
		xfr.xfrRef( IntToRef( this->nlocals ) );		//	raw N
		xfr.xfrRef( IntToRef( this->ninputs ) );		//	raw A
		
	
		xfr.setOrigin();
		xfr.xfrRef( fnkey );
	
		xfr.xfrVector( *this->code_data );	//	alt

		xfr.xfrRef( ToRef( this->props.size() ) );
		xfr.xfrString( this->props );
	
		return xfr.makeRef();
	} else {
		Ref * permanentStore = new Ref[ preflight_size ];		//	This store is never reclaimed.
		Ref * p = permanentStore;
		*p++ = ToRef( ( L << TAGGG ) | FUNC_LEN_TAGGG );
		*p++ = SYS_ABSENT;										// 	Data Pool NOT allowed out of heap.
		*p++ = IntToRef( this->nresults );
		*p++ = IntToRef( this->nlocals );
		*p++ = IntToRef( this->ninputs );
		Ref * func = p;
		if ( fnkey != sysCoreFunctionKey ) throw UnreachableError();
		*p++ = sysCoreFunctionKey;
		for ( std::vector< Ref >::iterator it = this->code_data->begin(); it != this->code_data->end(); ++it ) {
			*p++ = *it;
		}

		*p = ToRef( this->props.size() );
		memcpy( p + 1, this->props.c_str(), this->props.size() );
		p += N;
		
		return Ptr4ToRef( func );
	}
}

int CodeGenClass::tmpvar() {
	int n = this->current_slot;
	this->current_slot += 1;
	if ( this->current_slot > this->nlocals ) {
		this->nlocals = this->current_slot;
	}
	return n;
}


static bool isVIdentable( Gnx gnx ) {
	const string & nm = gnx->name();
	return nm == GNX_VAR or nm == GNX_ID or nm == GNX_CONSTANT;
}

static bool isConstantSmallIntGnx( Gnx gnx ) {
	if ( gnx->name() != GNX_CONSTANT || not gnx->hasAttribute( GNX_CONSTANT_TYPE, "int" ) ) return false;
	Maybe< long > v = gnx->maybeAttributeToLong( GNX_CONSTANT_VALUE );
	if ( not v.isValid() ) return false;
	return canFitInSmall( v.fastValue() );
}

void CodeGenClass::compileIfTest( bool sense, Gnx mnx, LabelClass * dst, LabelClass * contn ) {
	bool done = false;

	//	This next section should be refactored.
	if ( mnx->name() == GNX_SYSAPP ) {
		SysMap::iterator it = SysMap::systemFunctionsMap().find( mnx->attribute( GNX_SYSAPP_NAME ) );
		if ( it != SysMap::systemFunctionsMap().end() ) {
			const SysInfo & info = it->second;
			if ( 
				info.isCmpOp() and mnx->size() == 2 and
				isVIdentable( mnx->getChild( 0 ) ) and
				isVIdentable( mnx->getChild( 1 ) )
			) {
				VIdent vid0( this, mnx->getChild( 0 ) );
				VIdent vid1( this, mnx->getChild( 1 ) );
				this->compileComparison( sense, vid0, info.cmp_op, vid1, dst, contn );
				done = true; 	//	*** Processed the special case ***
			}
		}
	}

	if ( not done ) {
		if ( mnx->name() == GNX_OR && mnx->size() == 2 ) {
			LabelClass e( this );
			this->compileIfTest( sense, mnx->getChild( 0 ), e.jumpToJump( dst ), CONTINUE_LABEL );
			this->compileIfTest( sense, mnx->getChild( 1 ), e.jumpToJump( dst ), e.jumpToJump( contn ) );
			e.labelSet();
		} else if ( mnx->name() == GNX_AND && mnx->size() == 2 ) {
			LabelClass e( this );
			this->compileIfTest( sense, mnx->getChild( 0 ), CONTINUE_LABEL, e.jumpToJump( contn ) );
			this->compileIfTest( sense, mnx->getChild( 1 ), e.jumpToJump( dst ), e.jumpToJump( contn ) );
			e.labelSet();
		} else {
			this->compile1( mnx, CONTINUE_LABEL );
			this->vmiIF( sense, dst, contn );
		}
	}
}

void CodeGenClass::compileIfNot( Gnx mnx, LabelClass * dst, LabelClass * contn ) {
	this->compileIfTest( false, mnx, dst, contn );
}

void CodeGenClass::compileIfSo( Gnx mnx, LabelClass * dst, LabelClass * contn ) {
	this->compileIfTest( true, mnx, dst, contn );
}

void CodeGenClass::compileComparison( 
	bool sense, 
	const VIdent & vid0, 
	CMP_OP cmp_op,
	const VIdent & vid1, 
	LabelClass * dst, 
	LabelClass * contn 
) {
	if ( sense ) {
		this->vmiTEST( vid0, cmp_op, vid1, dst );
	} else {
		this->vmiTEST( vid0, revCmpOp( cmp_op ), vid1, dst );
	}
	this->continueFrom( contn );
}

void CodeGenClass::compileComparison( 
	const VIdent & vid0, 
	CMP_OP cmp_op,
	const VIdent & vid1, 
	LabelClass * dst, 
	LabelClass * contn 
) {
	if ( dst == contn ) {
		//	No point in test!
		this->continueFrom( contn );
	} else if ( dst == CONTINUE_LABEL ) {
		this->vmiTEST( vid0, revCmpOp( cmp_op ), vid1, contn );
	} else {
		this->vmiTEST( vid0, cmp_op, vid1, dst );
		this->continueFrom( contn );
	}
}

bool CompileQuery::isValidQuery( Gnx query ) {
	const string & nm = query->name();
	const int N = query->size();
	return(
		( nm == GNX_IN && N == 2 ) 				||
		( nm == GNX_FROM && 2 <= N && N <= 4 ) 	||
		( nm == GNX_WHERE && N == 2 ) 			||
		( nm == GNX_WHILE && N == 2 ) 			||
		( nm == GNX_DO && N == 2 ) 				||
		( nm == GNX_FINALLY && N == 2 ) 		||
		( nm == GNX_ZIP && N == 2 ) 			||
		( nm == GNX_CROSS && N == 2 ) 			||
		( nm == GNX_BIND && N == 2 )			||
		( nm == GNX_OK && N == 0 )
	);
}


void CompileQuery::compileQueryDecl( Gnx query ) {
	const string & nm = query->name();
	const int N = query->size();
	if ( ( ( nm == GNX_DO || nm == GNX_WHERE || nm == GNX_FINALLY ) && N == 2 ) || ( nm == GNX_WHILE && N == 2 ) ) {
		this->compileQueryDecl( query->getChild( 0 )  );
	} else if ( ( nm == GNX_CROSS || nm == GNX_ZIP ) && N == 2 ) {
		this->compileQueryDecl( query->getChild( 0 ) );
		this->compileQueryDecl( query->getChild( 1 ) );
	} else if ( nm == GNX_FROM && N >= 2 ) {
		Gnx var( query->getChild( 0 ) );
		int tmp_loop_var = this->newLoopVar( new VIdent( this->codegen, var ) );
		query->putAttribute( "tmp.loop.var", tmp_loop_var );
	} else if ( nm == GNX_IN && N >= 2 ) {
		std::vector< Gnx > vars;
		Gnx var( query->getChild( 0 ) );	
		if ( this->codegen->tryFlatten( var, GNX_VAR, vars ) && vars.size() == 1 ) {
			int tmp_loop_var = this->newLoopVar( new VIdent( this->codegen, vars[0] ) );
			query->putAttribute( "tmp.loop.var", tmp_loop_var );
		} else {
			throw SystemError( "Full BIND not implemented" ).culprit( "Name", var->name() );
		}
	} else if ( nm == GNX_BIND && N == 2 ) {
		int lo = -1;
		int hi = lo;
		std::vector< Gnx > vars;
		if ( this->codegen->tryFlatten( query->getChild( 0 ), GNX_VAR, vars ) ) {
			for ( std::vector< Gnx >::iterator it = vars.begin(); it != vars.end(); ++it ) {
				Gnx vc = *it;
				int tmp_loop_var = this->newLoopVar( new VIdent( this->codegen, vc ) );
				if ( lo < 0 ) lo = tmp_loop_var;
				hi = tmp_loop_var + 1;
			}
		}
		query->putAttribute( "tmp.bind.lo", lo );
		query->putAttribute( "tmp.bind.hi", hi );
		int tmp = this->codegen->tmpvar();
		query->putAttribute( "tmp.bind.var", tmp );
	} else if ( nm == GNX_ONCE && N == 0 ) {
		int tmp_loop_var = this->codegen->tmpvar();
		query->putAttribute( "tmp.once.var", tmp_loop_var );		
	} else if ( ( nm == GNX_OK || nm == GNX_FAIL ) && N == 0 ) {
		//	Nothing to do.
	} else {
		throw SystemError( "Not implemented general queries" ).culprit( "Name", nm ).culprit( "N", (long)N );
	}
}


void CompileQuery::compileQueryInit( Gnx query, LabelClass * contn ) {
	const string & nm = query->name();
	const int N = query->size();
	if ( ( ( nm == GNX_DO || nm == GNX_WHERE || nm == GNX_FINALLY ) && N == 2 ) || ( nm == GNX_WHILE && N == 2 ) ) {
		this->compileQueryInit( query->getChild( 0 ), contn );
	} else if ( nm == GNX_CROSS && N == 2 ) {
		int tmp_cross_needs_test_lhs = this->codegen->tmpvar();
		query->putAttribute( "tmp.cross.needs.test.lhs", tmp_cross_needs_test_lhs );
		this->codegen->vmiPUSHQ( SYS_TRUE );
		this->codegen->vmiPOP_INNER_SLOT( tmp_cross_needs_test_lhs );
		this->compileQueryInit( query->getChild( 0 ), contn );
	} else if ( nm == GNX_ZIP && N == 2 ) {
		this->compileQueryInit( query->getChild( 0 ), CONTINUE_LABEL );
		this->compileQueryInit( query->getChild( 1 ), contn );
	} else if ( nm == GNX_FROM && N >= 2 ) {
		Gnx var( query->getChild( 0 ) );		
		Gnx start_expr( query->getChild( 1 ) );
		this->codegen->compile1( start_expr, CONTINUE_LABEL );
		this->codegen->vmiPOP( var, false );
		
		if ( N >= 3 ) {
			//	We have a FROM and BY part at least. If the BY part
			//	is the constant 1 then we will optimise it away.
			Gnx by_expr( query->getChild( 2 ) );
			if ( 
				//	TODO: refactor this into a simple call.
				not( 
					by_expr->hasName( GNX_CONSTANT ) &&
					by_expr->hasAttribute( GNX_CONSTANT_TYPE, "int" ) &&
					by_expr->hasAttribute( GNX_CONSTANT_VALUE, "1" )
				)
			) {
				int tmp_by_expr = this->codegen->tmpvar();
				query->putAttribute( "tmp.by.expr", tmp_by_expr );
				this->codegen->compile1( by_expr, CONTINUE_LABEL );
				this->codegen->vmiPOP_INNER_SLOT( tmp_by_expr );
			}
		}

		if ( N >= 4 ) {
			//	We have FROM, BY and TO parts.
			Gnx end_expr( query->getChild( 3 ) );
			int tmp_end_expr = this->codegen->tmpvar();
			query->putAttribute( "tmp.end.expr", tmp_end_expr );
			this->codegen->compile1( end_expr, CONTINUE_LABEL );
			this->codegen->vmiPOP_INNER_SLOT( tmp_end_expr );
		}

		this->codegen->continueFrom( contn );
		
	} else if ( nm == GNX_IN && N >= 2 ) {
		this->codegen->compile1( query->getChild( 1 ), CONTINUE_LABEL );
		this->codegen->vmiINSTRUCTION( vmc_getiterator );
		int tmp_next_fn = this->codegen->tmpvar();
		int tmp_context = this->codegen->tmpvar();
		int tmp_state = this->codegen->tmpvar();
		query->putAttribute( "tmp.next.fn", tmp_next_fn );
		query->putAttribute( "tmp.context", tmp_context );
		query->putAttribute( "tmp.state", tmp_state );
		this->codegen->vmiPOP_INNER_SLOT( tmp_next_fn );
		this->codegen->vmiPOP_INNER_SLOT( tmp_context );
		this->codegen->vmiPOP_INNER_SLOT( tmp_state );
		this->codegen->continueFrom( contn );
	} else if ( nm == GNX_BIND && N == 2 ) {
		const int tmp = query->attributeToInt( "tmp.bind.var" );
		this->codegen->vmiPUSHQ( SYS_TRUE );
		this->codegen->vmiPOP_INNER_SLOT( tmp );
		this->codegen->continueFrom( contn );
	} else if ( nm == GNX_ONCE && N == 0 ) {
		const int tmp = query->attributeToInt( "tmp.once.var" );
		this->codegen->vmiPUSHQ( SYS_TRUE );
		this->codegen->vmiPOP_INNER_SLOT( tmp );
		this->codegen->continueFrom( contn );
	} else if ( ( nm == GNX_OK || GNX_FAIL ) && N == 0 ) {
		this->codegen->continueFrom( contn );
	} else {
		throw SystemError( "Not implemented general queries [2]" ).culprit( "Name", nm ).culprit( "N", static_cast< long >( N ) );
	}
}

/**	Compiles a query so that if the query evaluates to true we jump to label 
	'dst' otherwise we continue at label 'contn'.
*/
void CompileQuery::compileQueryTest( Gnx query, LabelClass * dst, LabelClass * contn ) {
	const string & nm = query->name();
	const int N = query->size();
	if ( nm == GNX_DO && N == 2 ) {
		this->compileQueryTest( query->getChild( 0 ), dst, contn );
	} else if ( nm == GNX_CROSS && N == 2 ) {
		
		int tmp_cross_needs_test_lhs = query->attributeToInt( "tmp.cross.needs.test.lhs" );
		
		LabelClass outer_loop_label( this->codegen );
		LabelClass inner_loop_label( this->codegen );
		LabelClass done_label( this->codegen );
		
		outer_loop_label.labelSet();

		this->codegen->vmiPUSH_INNER_SLOT( tmp_cross_needs_test_lhs );
		this->codegen->vmiIFNOT( &inner_loop_label );

		this->compileQueryTest( query->getChild( 0 ), CONTINUE_LABEL, done_label.jumpToJump( contn ) );
		this->codegen->vmiPUSHQ( SYS_FALSE );
		this->codegen->vmiPOP_INNER_SLOT( tmp_cross_needs_test_lhs );

		this->compileQueryBody( query->getChild( 0 ), CONTINUE_LABEL );
		this->compileQueryInit( query->getChild( 1 ), CONTINUE_LABEL );

		inner_loop_label.labelSet();

		this->compileQueryTest( query->getChild( 1 ), done_label.jumpToJump( dst ), CONTINUE_LABEL );
		this->codegen->vmiPUSHQ( SYS_TRUE );
		this->codegen->vmiPOP_INNER_SLOT( tmp_cross_needs_test_lhs );
		this->compileQueryAdvn( query->getChild( 0 ), &outer_loop_label );

		done_label.labelSet();

	} else if ( nm == GNX_ZIP && N == 2 ) {
		LabelClass done_label( this->codegen );
		this->compileQueryTest( query->getChild( 0 ), CONTINUE_LABEL, done_label.jumpToJump( contn ) );
		this->compileQueryTest( query->getChild( 1 ), dst, contn );
		done_label.labelSet();
	} else if ( nm == GNX_WHILE && N == 2 ) {
		LabelClass done_label( this->codegen );
		this->compileQueryTest( query->getChild( 0 ), CONTINUE_LABEL, & done_label );
		this->codegen->compileIfSo( query->getChild( 1 ), done_label.jumpToJump( dst ), contn );
		done_label.labelSet();
	} else if ( nm == GNX_WHERE && N == 2 ) {
		Gnx lhs = query->getChild( 0 );
		Gnx rhs = query->getChild( 1 );
		if ( lhs->name() == GNX_OK ) {
			this->codegen->compileIfSo( rhs, dst, contn );
		} else {
			LabelClass start_label( this->codegen );
			LabelClass done_label( this->codegen );
			start_label.labelSet();
			this->compileQueryTest( lhs, CONTINUE_LABEL, done_label.jumpToJump( contn ) );
			this->codegen->compileIfSo( rhs, done_label.jumpToJump( dst ), CONTINUE_LABEL );
			this->compileQueryAdvn( lhs, &start_label );
			done_label.labelSet();
		}
	} else if ( nm == GNX_FINALLY && N == 2 ) {
		LabelClass done_label( this->codegen );
		this->compileQueryTest( query->getChild( 0 ), done_label.jumpToJump( dst ), CONTINUE_LABEL );
		this->codegen->compileGnx( query->getChild( 1 ), contn );
		done_label.labelSet();
	} else if ( nm == GNX_FROM ) {
		const int N = query->size();
		if ( N >= 3 ) {
			VIdent end_expr( query->attributeToInt( "tmp.end.expr" ) );
			this->codegen->compileComparison( this->getLoopVar( query->attributeToInt( "tmp.loop.var" ) ), CMP_LTE, end_expr, dst, contn );
		} else if ( dst != CONTINUE_LABEL ) {
			this->codegen->vmiGOTO( dst );
		}
	} else if ( nm == GNX_IN ) {
		int tmp_state = query->attributeToInt( "tmp.state" );
		int tmp_context = query->attributeToInt( "tmp.context" );
		int tmp_next_fn = query->attributeToInt( "tmp.next.fn" );

		this->codegen->vmiPUSH_INNER_SLOT( tmp_state );	
		this->codegen->vmiPUSH_INNER_SLOT( tmp_context );	
		this->codegen->vmiSET_CALL_INNER_SLOT( 2, tmp_next_fn );		
		this->codegen->vmiPOP_INNER_SLOT( tmp_state );

		VIdent & vid = this->getLoopVar( query->attributeToInt( "tmp.loop.var" ) );
		this->codegen->vmiPOP( vid, false );
		VIdent id_tmp_state( tmp_state );
		VIdent termin( SYS_TERMIN );
		this->codegen->compileComparison( id_tmp_state, CMP_NEQ, termin, dst, contn );
	} else if ( nm == GNX_BIND && N == 2 ) {
		const int lo = query->attributeToInt( "tmp.bind.lo" );
		const int hi = query->attributeToInt( "tmp.bind.hi" );
		const int tmp = query->attributeToInt( "tmp.bind.var" );
		Gnx lhs = query->getChild( 0 );
		Gnx rhs = query->getChild( 1 );
		std::vector< Gnx > vars;
		if ( this->codegen->tryFlatten( lhs, GNX_VAR, vars ) ) {
			this->codegen->vmiPUSH_INNER_SLOT( tmp );
			LabelClass done_label( this->codegen );

			this->codegen->vmiIFNOT( done_label.jumpToJump( contn ) );
			this->codegen->vmiPUSHQ( SYS_FALSE );
			this->codegen->vmiPOP_INNER_SLOT( tmp );
			this->codegen->compileNelse( rhs, vars.size(), CONTINUE_LABEL, done_label.jumpToJump( contn ) );
			
			for ( int count = hi - 1; count >= lo; count-- ) {
				this->codegen->vmiPOP( this->getLoopVar( count ), false );
			}
			
			this->codegen->continueFrom( dst );
			done_label.labelSet();
		} else {
			throw Ginger::Mishap( "BIND not fully implemented [3]" );
		}
	} else if ( nm == GNX_ONCE && N == 0 ) {
		const int tmp = query->attributeToInt( "tmp.once.var" );
		this->codegen->vmiPUSH_INNER_SLOT( tmp );
		this->codegen->vmiIFSO( dst, contn );
	} else if ( nm == GNX_OK && N == 0 ) {
		this->codegen->continueFrom( dst );
	} else if ( nm == GNX_FAIL && N == 0 ) {
		this->codegen->continueFrom( contn );
	} else {
		throw SystemError( "Not implemented general queries" );
	}
}

void CompileQuery::compileQueryBody( Gnx query, LabelClass * contn ) {
	const string & nm = query->name();
	const int N = query->size();
	if ( nm == GNX_DO && N == 2 ) {
		this->compileQueryBody( query->getChild( 0 ), CONTINUE_LABEL );
		this->codegen->compileGnx( query->getChild( 1 ), contn );
	} else if ( nm == GNX_CROSS && N == 2 ) {
		this->compileQueryBody( query->getChild( 1 ), contn );
	} else if (
		( nm == GNX_WHERE && N == 2 ) || 
		( nm == GNX_WHILE && N == 2 ) ||
		( nm == GNX_FINALLY && N == 2 )
	) {
		this->compileQueryBody( query->getChild( 0 ), contn );
	} else if ( nm == GNX_ZIP && N == 2 ) {
		this->compileQueryBody( query->getChild( 0 ), CONTINUE_LABEL );
		this->compileQueryBody( query->getChild( 1 ), contn );
	}
}

void CompileQuery::compileQueryAdvn( Gnx query, LabelClass * contn ) {
	const string & nm = query->name();
	const int N = query->size();
	if ( 
		( ( nm == GNX_DO || nm == GNX_WHERE || nm == GNX_FINALLY ) && N == 2 ) || 
		( nm == GNX_WHILE && N == 2 )
	) {
		this->compileQueryAdvn( query->getChild( 0 ), contn );
	} else if ( nm == GNX_ZIP && N == 2 ) {
		this->compileQueryAdvn( query->getChild( 0 ), CONTINUE_LABEL );
		this->compileQueryAdvn( query->getChild( 1 ), contn );
	} else if ( nm == GNX_CROSS && N == 2 ) {
		this->compileQueryAdvn( query->getChild( 1 ), contn );
	} else if ( nm == GNX_FROM ) {

		//	Obvious candidate for a merged instruction.
		VIdent & lv = this->getLoopVar( query->attributeToInt( "tmp.loop.var" ) );
		this->codegen->vmiPUSH( lv );
		if ( query->hasAttribute( "tmp.by.expr" ) ) {
			VIdent by( query->attributeToInt( "tmp.by.expr" ) );
			this->codegen->vmiPUSH( by );
			this->codegen->vmiINSTRUCTION( vmc_add );
		} else {
			this->codegen->vmiINSTRUCTION( vmc_incr );
		}
		this->codegen->vmiPOP( lv, false );

	} else if ( nm == GNX_ONCE ) {
		const int tmp = query->attributeToInt( "tmp.once.var" );
		this->codegen->vmiPUSHQ( SYS_FALSE );
		this->codegen->vmiPOP_INNER_SLOT( tmp );
	} else if ( nm == GNX_IN || nm == GNX_BIND || nm == GNX_OK ) {
		//	Nothing.
	} else {
		throw SystemError( "Not implemented general queries" );
	}

	//	Otherwise continue onto the test phase.
	this->codegen->continueFrom( contn );
}

void CompileQuery::compileQueryFini( Gnx query, LabelClass * contn ) {
	const string & nm = query->name();
	const int N = query->size();
	if ( nm == GNX_DO && N == 2 ) {
		this->compileQueryFini( query->getChild( 0 ), contn );
	} else {
		this->codegen->continueFrom( contn );
	}
}

void CompileQuery::compileFor( Gnx query, LabelClass * contn ) {
	LabelClass body_label( this->codegen );
	LabelClass test_label( this->codegen );
	this->compileQueryDecl( query );
	this->compileQueryInit( query, &test_label );
	body_label.labelSet();
	this->compileQueryBody( query, CONTINUE_LABEL );
	this->compileQueryAdvn( query, CONTINUE_LABEL );
	test_label.labelSet();
	this->compileQueryTest( query, &body_label, CONTINUE_LABEL );
	this->compileQueryFini( query, contn );
}

/*	
//	TODO: Restore this (or something like it) when we implement
//	instruction-objects.
static std::string contextHint( Gnx query ) {
	string context = "cause ";
	context += query->attribute( "context", "-" );
	context += " ";
	context += "at ";
	context += query->attribute( "span", "-" );
	context += " ";
	context += "from " + query->attribute( "src", "-" );
	return context;
}
*/

void CompileQuery::compileNakedQuery( Gnx query, LabelClass * contn ) {
	LabelClass ok_label( this->codegen );
	this->compileQueryDecl( query );
	this->compileQueryInit( query, CONTINUE_LABEL );
	this->compileQueryTest( query, &ok_label, CONTINUE_LABEL );
	this->codegen->vmiFAIL();
	ok_label.labelSet();
	this->compileQueryBody( query, contn );
}

void CodeGenClass::compileGnxSwitch( const int offset, const int switch_slot, int tmp_slot, Gnx mnx, LabelClass * contn ) {
	const int a = mnx->size() - offset;
	if ( a == 0 ) {
		this->continueFrom( contn );
	} else if ( a == 1 ) {
		this->compileGnx( mnx->getChild( offset ), contn );
	} else if ( a == 2 ) {
		LabelClass doneLab( this );
		if ( tmp_slot == -1 ) {
			//	Allocate tmp_slot on first use.
			tmp_slot = this->tmpvar();
		}
		this->compile1( mnx->getChild( offset ), CONTINUE_LABEL );
		this->vmiPOP_INNER_SLOT( tmp_slot );
		this->vmiTEST( switch_slot, CMP_NEQ, tmp_slot, doneLab.jumpToJump( contn ) );
		this->compileGnx( mnx->getChild( offset + 1 ), contn );
		doneLab.labelSet();		
	} else {
		LabelClass elseLab( this );
		LabelClass doneLab( this );

		if ( tmp_slot == -1 ) {
			//	Allocate tmp_slot on first use.
			tmp_slot = this->tmpvar();
		}
		this->compile1( mnx->getChild( offset ), CONTINUE_LABEL );
		this->vmiPOP_INNER_SLOT( tmp_slot );
		this->vmiTEST( switch_slot, CMP_NEQ, tmp_slot, &elseLab );

		this->compileGnx( mnx->getChild( offset + 1 ), doneLab.jumpToJump( contn ) );

		elseLab.labelSet();
		this->compileGnxSwitch( offset + 2, switch_slot, tmp_slot, mnx, contn );

		doneLab.labelSet();

	}
}

void CodeGenClass::compileGnxSwitch( Gnx mnx, LabelClass * contn ) {
	int switch_slot = this->tmpvar();
	this->compile1( mnx->getChild( 0 ), CONTINUE_LABEL );
	this->vmiPOP_INNER_SLOT( switch_slot );
	//	N.B. -1 is an invalid tmpvar.
	this->compileGnxSwitch( 1, switch_slot, -1, mnx, contn );
}

void CodeGenClass::compileGnxIf( int offset, Gnx mnx, LabelClass * contn ) {
	const int a = mnx->size() - offset;
	if ( a == 0 ) {
		this->continueFrom( contn );
	} else if ( a == 1 ) {
		this->compileGnx( mnx->getChild( offset ), contn );
	} else if ( a == 2 ) {
		Gnx if_part = mnx->getChild( offset );
		Gnx then_part = mnx->getChild( offset + 1 );
		if ( CompileQuery::isValidQuery( if_part ) ) {
			MnxBuilder ifthen;
			ifthen.start( GNX_DO );
			ifthen.add( if_part );
			ifthen.add( then_part );
			ifthen.end();
			this->compileGnx( ifthen.build(), contn );
		} else {
			LabelClass doneLab( this );
			this->compileIfNot( mnx->getChild( offset ), doneLab.jumpToJump( contn ), CONTINUE_LABEL );
			this->compileGnx( mnx->getChild( offset + 1 ), contn );
			doneLab.labelSet();
		}
	} else {
		Gnx if_part = mnx->getChild( offset );
		Gnx then_part = mnx->getChild( offset + 1 );
		if ( CompileQuery::isValidQuery( if_part) ) {
			LabelClass done_label( this );
			LabelClass else_label( this );
			CompileQuery cq( this );
			cq.compileQueryDecl( if_part );
			cq.compileQueryInit( if_part, CONTINUE_LABEL );
			cq.compileQueryTest( if_part, CONTINUE_LABEL, & else_label );
			this->compileGnx( then_part, done_label.jumpToJump( contn ));
			else_label.labelSet();
			this->compileGnxIf( offset + 2, mnx, contn );
			done_label.labelSet();
		} else {
			LabelClass elseLab( this );
			LabelClass doneLab( this );
			this->compileIfNot( mnx->getChild( offset ), &elseLab, CONTINUE_LABEL );
			this->compileGnx( mnx->getChild( offset + 1 ), doneLab.jumpToJump( contn ) );
			elseLab.labelSet();
			this->compileGnxIf( offset + 2, mnx, contn );
			doneLab.labelSet();
		}
	}
}

void CodeGenClass::compileGnxIf( Gnx mnx, LabelClass * contn ) {
	this->compileGnxIf( 0, mnx, contn );
}

void CodeGenClass::compileGnxFn( Gnx mnx, LabelClass * contn ) {
	int args_count = mnx->attributeToInt( GNX_FN_ARGS_COUNT );
	int locals_count = mnx->attributeToInt( GNX_FN_LOCALS_COUNT );
	this->vmiFUNCTION( mnx->attribute( GNX_FN_NAME, GNX_EMPTY_FN_NAME ), locals_count, args_count );
	this->vmiENTER();
	LabelClass retn( this, true );
	this->compileGnx( mnx->getChild( 1 ), &retn );
	retn.labelSet();
	if ( retn.hasBeenUsed() ) {
		this->vmiRETURN();
	}
	this->vmiPUSHQ( this->vmiENDFUNCTION() );
	this->continueFrom( contn );
}

void CodeGenClass::compileChildrenChecked( Gnx mnx, Arity arity ) {
	Ginger::Arity args_arity( mnx->attribute( GNX_ARGS_ARITY, "+0" ) );
	if ( arity == args_arity ) {
		this->compileChildren( mnx, CONTINUE_LABEL );
	} else if ( arity.isExact() ) {
		int v = this->tmpvar();
		this->vmiSTART_MARK( v );
		this->compileChildren( mnx, CONTINUE_LABEL );
		this->vmiSET_COUNT_TO_MARK( v );
		this->vmiCHECK_COUNT( arity.count() );
	} else {	
		throw UnreachableError();
	}
}

void CodeGenClass::compileGnxSysApp( Gnx mnx, LabelClass * contn ) {
	const string & nm = mnx->attribute( GNX_SYSAPP_NAME );
	SysMap::iterator it = SysMap::systemFunctionsMap().find( nm );
	if ( it != SysMap::systemFunctionsMap().end() ) {
		const SysInfo & info = it->second;
		if ( info.isSysCall() ) {
			int v = this->tmpvar();
			this->vmiSTART_MARK( v );
			this->compileChildren( mnx, CONTINUE_LABEL );
			this->vmiSET_COUNT_TO_MARK( v );
			this->vmiSYS_CALL( info.syscall );
		} else if ( info.isVMOp() ) {
			//cerr << "VM OP" << endl;
			bool c0 = false;
			bool c1 = false;
			//cerr << "nm == +? " << nm << "? " << ( nm == "+" ) << endl;
			//cerr << "mnx->size() == 2? " << ( mnx->size() == 2 ) << endl;
			if ( 
				nm == "+" and mnx->size() == 2 and 
				( ( c0 = isConstantSmallIntGnx( mnx->getChild( 0 ) ) ) or ( c1 = isConstantSmallIntGnx( mnx->getChild( 1 ) ) ) )
			) {			
				//	TODO: We should shift the operator into being an incr/decr.
				//	Check which one gets optimised.
				this->compile1( mnx->getChild( c0 ? 1 : 0 ), CONTINUE_LABEL );
				this->vmiINCR( mnx->getChild( c0 ? 0 : 1 )->attributeToLong( GNX_CONSTANT_VALUE ) );
			} else if ( 
				nm == "-" and mnx->size() == 2 and isConstantSmallIntGnx( mnx->getChild( 1 ) )
			) {				
				//	TODO: We should shift the operator into being an incr/decr.
				this->compile1( mnx->getChild( 0 ), CONTINUE_LABEL );
				this->vmiDECR( mnx->getChild( 1 )->attributeToLong( GNX_CONSTANT_VALUE ) );
			} else {
				this->compileChildrenChecked( mnx, info.in_arity );
				this->vmiINSTRUCTION( info.instruction );
			}
		} else if ( info.isCmpOp() ) {
			this->compileChildrenChecked( mnx, info.in_arity );
			this->vmiINSTRUCTION( cmpOpInstruction( info.cmp_op ) );
		}
	} else {
		throw SystemError( "Unknown system call" ).culprit( "Name", mnx->attribute( GNX_SYSAPP_NAME ) );
	}
	this->continueFrom( contn );
}

void CodeGenClass::compileGnxApp( Gnx mnx, LabelClass * contn ) {
	Gnx fn( mnx->getChild( 0 ) );
	Gnx args( mnx->getChild( 1 ) );
	Ginger::Arity aargs( args->attribute( GNX_ARITY, "+0" ) );
	
	if ( fn->name() == GNX_ID ) {
		VIdent vid( this, fn );
		if ( aargs.isntExact() ) { 
			int v = this->tmpvar();
			this->vmiSTART_MARK( v );
			this->compileGnx( args, CONTINUE_LABEL );
			this->vmiEND_CALL_ID( v, vid );
		} else {
			this->compileGnx( args, CONTINUE_LABEL );
			this->vmiSET_CALL_ID( aargs.count(), vid );
		}
	} else if ( aargs.isntExact() ) {
		int v = this->tmpvar();
		this->vmiSTART_MARK( v );
		this->compileGnx( args, CONTINUE_LABEL );
		this->compile1( fn, CONTINUE_LABEL );
		this->vmiEND1_CALLS( v );
	} else {
		this->compileGnx( args, CONTINUE_LABEL );
		this->compile1( fn, CONTINUE_LABEL );
		this->vmiSET_CALLS( aargs.count() );
	}
	
	this->continueFrom( contn );
}

void CodeGenClass::compileGnxFor( Gnx query, LabelClass * contn ) {
	CompileQuery cq( this );
	cq.compileFor( query, contn );
}

void CodeGenClass::compileGnxDeref( Gnx mnx, LabelClass * contn ) {
	VIdent id( this, mnx->getChild( 0 ) );
	this->vmiPUSH( id );
	this->vmiDEREF();
}

void CodeGenClass::compileGnxMakeRef( Gnx mnx, LabelClass * contn ) {
	this->compile1( mnx->getChild( 0 ), contn );
	this->vmiMAKEREF();
}

void CodeGenClass::compileGnxSetCont( Gnx mnx, LabelClass * contn ) {
	VIdent id( this, mnx->getChild( 1 ) );
	this->compile1( mnx->getChild( 0 ), contn );
	this->vmiPUSH( id );
	this->vmiSETCONT();	
}

void CodeGenClass::compileGnxSelfCall( Gnx mnx, LabelClass * contn ) {
	Ginger::Arity args_arity( mnx->attribute( GNX_ARGS_ARITY, "+0" ) );
	if ( args_arity.isExact() ) {
		this->compileChildren( mnx, CONTINUE_LABEL );
		this->vmiSELF_CALL_N( args_arity.count() ); 
	} else {
		int v = this->tmpvar();
		this->vmiSTART_MARK( v );
		this->compileChildren( mnx, CONTINUE_LABEL );
		this->vmiSET_COUNT_TO_MARK( v );
		this->vmiSELF_CALL();
	}
}


Ref CodeGenClass::calcConstant( Gnx mnx ) {
	#ifdef DBG_CODEGEN
		std::cout << "calcConstant: " << mnx->toString() << std::endl;
	#endif
	const string & type = mnx->attribute( GNX_CONSTANT_TYPE );
	if ( type == "int" ) {
		Maybe< long > v( mnx->maybeAttributeToLong( GNX_CONSTANT_VALUE ) );
		if ( v.isValid() && canFitInSmall( v.fastValue() ) ) {
			return LongToRef( v.fastValue() );
		} else {
			return this->vm->heap().copyBigInt( mnx->attribute( GNX_CONSTANT_VALUE ).c_str() );
		}
	} else if ( type == "bool" ) {
		return mnx->hasAttribute( GNX_CONSTANT_VALUE, "false" ) ? SYS_FALSE : SYS_TRUE;
	} else if ( type == "absent" ) {
		return SYS_ABSENT;
	} else if ( type == "termin" ) {
		return SYS_TERMIN;
	} else if ( type == "undefined" ) {
		return SYS_UNDEFINED;
	} else if ( type == "indeterminate" ) {
		return SYS_INDETERMINATE;
	} else if ( type == "char" ) {
		const string & s = mnx->attribute( GNX_CONSTANT_VALUE );
		if ( not s.empty() ) {
			return CharToCharacter( s[ 0 ] );
		} else {
			throw SystemError( "Invalid character string" );
		}
	} else if ( type == "string" ) {
		return this->vm->heap().copyString( mnx->attribute( GNX_CONSTANT_VALUE ).c_str() );
	} else if ( type == "double" ) {
		gngdouble_t d;
		const std::string& numtext( mnx->attribute( GNX_CONSTANT_VALUE ) );
		if ( numtext == "infinity" || numtext == "+infinity" || numtext == "inf" ) {
			return this->vm->heap().copyDouble( 1.0 / 0.0 );
		} else if ( numtext == "-infinity" || numtext == "-inf" ) {
			return this->vm->heap().copyDouble( -1.0 / 0.0 );
		} else if ( numtext == "nullity" || numtext == "nan" ) {
			return this->vm->heap().copyDouble( 0.0 / 0.0 );
		} else {
			std::istringstream i( numtext );
			if ( not ( i >> d ) ) {
				throw Ginger::Mishap( "Format of double precision number incorrect" ).culprit( "Number", numtext );
			} else {
				char c;
				if ( i >> c && c == '%' ) {
					d *= 0.01;
				}
				return this->vm->heap().copyDouble( d );
			}
		}
	} else if ( type == "symbol" ) {
		return refMakeSymbol( mnx->attribute( GNX_CONSTANT_VALUE ) );
	} else if ( type == GNX_SYSFN and mnx->hasAttribute( GNX_SYSFN_VALUE ) ) {
		Ref r = makeSysFn( this, mnx->attribute( GNX_SYSFN_VALUE ), SYS_UNDEFINED );
		if ( r == SYS_UNDEFINED ) {
			throw Ginger::Mishap( "No such system function" ).culprit( "Function", mnx->attribute( GNX_SYSFN_VALUE ) );
		}
		return r;
	} else if ( type == "sysclass" ) {
		/*
		<constant type="sysclass" value="Absent"/>          ### class for absent
		<constant type="sysclass" value="Bool"/>            ### class for true & false
		<constant type="sysclass" value="Small"/>        	### class for 'small' integers
		<constant type="sysclass" value="Float"/>           ### class for floats
		<constant type="sysclass" value="String"/>          ### class for strings
		<constant type="sysclass" value="Char"/>            ### class for characters
		<constant type="sysclass" value="Nil"/>             ### class for nil
		<constant type="sysclass" value="Pair"/>            ### class for list pairs
		<constant type="sysclass" value="Vector"/>          ### class for vectors
		<constant type="sysclass" value="Class"/>           ### class for classes
		*/
		const std::string cname( mnx->attribute( GNX_CONSTANT_VALUE ) );
		Ref r = nameToKey( cname );
		if ( r != SYS_ABSENT ) {
			return r;
		} else {
			//	TODO: Function keys - not as easy as the documentation makes out.
			//	TODO: Map keys - same deal
			//			<sysclass value="Fn"/>              ### class for function objects
			throw SystemError( "System class not recognised" ).culprit( "Expression", mnx->toString() );
		}
	} else {
		throw SystemError( "System constant not recognised" ).culprit( "Expression", mnx->toString() );
	}
}

void CodeGenClass::compileGnxConstant( Gnx mnx, LabelClass * contn ) {
	this->vmiPUSHQ( this->calcConstant( mnx ), contn );
}

static void throwProblem( Gnx mnx ) {
	Ginger::Mishap mishap( mnx->attribute( GNX_PROBLEM_MESSAGE ), mnx->attribute( GNX_PROBLEM_CATEGORY, "S" ) );
	MnxChildIterator mnxit( mnx );
	while ( mnxit.hasNext() ) {
		Gnx & culprit = mnxit.next();
		if ( culprit->hasName( GNX_CULPRIT ) && culprit->hasAttribute( GNX_CULPRIT_NAME ) && culprit->hasAttribute( GNX_CULPRIT_VALUE ) ) {
			mishap.culprit( culprit->attribute( GNX_CULPRIT_NAME ), culprit->attribute( GNX_CULPRIT_VALUE ) );
		}
	}
	throw mishap;
}

/**
	Boolean test, bool_vs_abs == true
	Absent test,  bool_vs_abs == false
	AND and_vs_or == true
	OR  and_vs_or == false
 */

void CodeGenClass::compileBoolAbsAndOr( bool bool_vs_abs, bool and_vs_or, Gnx mnx, LabelClass * contn ) {
	LabelClass e( this );
	LabelClass * end = e.jumpToJump( contn );
	this->compile1( mnx->getChild( 0 ), CONTINUE_LABEL );
	if ( bool_vs_abs ) {
		if ( and_vs_or ) {
			this->vmiAND( end );
		} else {
			this->vmiOR( end );
		}	
	} else {
		if ( and_vs_or ) {
			this->vmiABS_AND( end );
		} else {
			this->vmiABS_OR( end );
		}
	}
	this->compile1( mnx->getChild( 1 ), end );
	e.labelSet();	
}

void CodeGenClass::compileErase( Gnx mnx, LabelClass * contn ) {
	int v = this->tmpvar();
	this->vmiSTART_MARK( v );
	this->compileChildren( mnx, CONTINUE_LABEL );
	this->vmiERASE_MARK( v );
	this->continueFrom( contn );
}

bool CodeGenClass::tryFlatten( Gnx mnx, const char * name, std::vector< Gnx > & vars ) {
	if ( mnx->hasName( name ) ) {
		vars.push_back( mnx );
		return true;
	} else if ( mnx->hasName( GNX_SEQ ) ) {
		MnxChildIterator children( mnx );
		while ( children.hasNext() ) {
			Gnx child = children.next();
			if ( not this->tryFlatten( child, name, vars ) ) return false;
		}
		return true;
	} else {
		return false;
	}
}

/*bool CodeGenClass::tryFlattenVarOrConstant( Gnx mnx, std::vector< Gnx > & vars ) {
	if ( mnx->hasName( VAR ) || mnx->hasName( CONSTANT ) ) {
		vars.push_back( mnx );
		return true;
	} else if ( mnx->hasName( SEQ ) ) {
		MnxChildIterator children( mnx );
		while ( children.hasNext() ) {
			Gnx child = children.next();
			if ( not this->tryFlattenVarOrConstant( child, vars ) ) return false;
		}
		return true;
	} else {
		return false;
	}
}*/

void CodeGenClass::compileBindDst( Gnx lhs ) {
	if ( lhs->hasName( GNX_VAR ) ) {
		VIdent vid( this, lhs );
		this->vmiPOP( vid, false );
	} else if ( lhs->hasName( GNX_SEQ ) ) {
		std::vector< Gnx > vars;
		if ( this->tryFlatten( lhs, GNX_VAR, vars ) ) {
			for ( std::vector< Gnx >::reverse_iterator it = vars.rbegin(); it != vars.rend(); ++it ) {
				VIdent vid( this, *it );
				this->vmiPOP( vid, false );
			}
		} else {
			throw Ginger::Mishap( "BIND not fully implemented [1]" );
		}
	} else {
		throw Ginger::Mishap( "BIND not fully implemented [2]" );
	}
}

void CodeGenClass::compileBind( Gnx lhs, Gnx rhs, LabelClass * contn ) {
	Arity a( lhs->attribute( GNX_PATTERN_ARITY, "0+" ) );
	if ( a.isExact() ) { 
		this->compileN( rhs, a.count(), CONTINUE_LABEL );
	} else {
		throw Mishap( "Pattern matching not fully implemented [3]" );
	}
	this->compileBindDst( lhs );
	this->continueFrom( contn );
}




void CodeGenClass::compileGnx( Gnx mnx, LabelClass * contn ) {
	#ifdef DBG_CODEGEN
		cerr << "appginger/compileGnx" << endl;
		cerr << "  [[";
		mnx->render( cerr );
		cerr << "]]" << endl;
	#endif
	const string & nm = mnx->name();
	const int N = mnx->size();
	if ( nm == GNX_CONSTANT ) {
		this->compileGnxConstant( mnx, contn );
	} else if ( nm == GNX_ID ) {
		VIdent vid( this, mnx );
		this->vmiPUSH( vid, contn );
	} else if ( nm == GNX_IF ) {
		this->compileGnxIf( mnx, contn );
	} else if ( nm == GNX_SWITCH && mnx->size() >= 1 ) {
		this->compileGnxSwitch( mnx, contn );
	} else if ( nm == GNX_OR && mnx->size() == 2 ) {
		this->compileBoolAbsAndOr( true, false, mnx, contn );
	} else if ( nm == GNX_AND && mnx->size() == 2 ) {
		this->compileBoolAbsAndOr( true, true, mnx, contn );
	} else if ( nm == GNX_ABSOR && mnx->size() == 2 ) {
		this->compileBoolAbsAndOr( false, false, mnx, contn );
	} else if ( nm == GNX_ABSAND && mnx->size() == 2 ) {
		this->compileBoolAbsAndOr( false, true, mnx, contn );
	} else if ( nm == GNX_SYSAPP ) {
		this->compileGnxSysApp( mnx, contn );
	} else if ( nm == GNX_APP ) {
		this->compileGnxApp( mnx, contn );
	} else if ( nm == GNX_SEQ ) {
		this->compileChildren( mnx, contn );
	} else if ( nm == GNX_ERASE ) {
		this->compileErase( mnx, contn );
	} else if ( nm == GNX_BIND and N == 2 ) {
		#ifdef DBG_CODEGEN
			cerr << "appginger/compileGnx/BIND" << endl;
			cerr << "  [[";
			mnx->getChild( 0 )->render( cerr );
			cerr << "]]" << endl;
			cerr << "  [[";
			mnx->getChild( 1 )->render( cerr );	
			cerr << "]]" << endl;
		#endif
		Gnx lhs = mnx->getChild( 0 );
		Gnx rhs = mnx->getChild( 1 );
		compileBind( lhs, rhs, contn );
	} else if ( nm == GNX_FOR and N == 1 ) {
		this->compileGnxFor( mnx->getChild( 0 ), contn );
	} else if ( nm == GNX_LIST ) {
		if ( mnx->size() == 0 ) {
			this->vmiPUSHQ( SYS_NIL, contn );
		} else {
			int v = this->tmpvar();
			this->vmiSTART_MARK( v );
			this->compileChildren( mnx, CONTINUE_LABEL );
			this->vmiSET_COUNT_TO_MARK( v );
			this->vmiSYS_CALL( sysNewList );			
			this->continueFrom( contn );
		}
	} else if ( nm == GNX_LIST_APPEND && mnx->size() == 2 ) {
		if ( mnx->getChild( 0 )->name() == GNX_LIST ) {
			int v = this->tmpvar();
			this->vmiSTART_MARK( v );
			this->compileChildren( mnx->getChild( 0 ), CONTINUE_LABEL );
			this->compileGnx( mnx->getChild( 1 ), CONTINUE_LABEL );
			this->vmiSET_COUNT_TO_MARK( v );
			this->vmiSYS_CALL( sysNewListOnto );
		} else {
			int v = this->tmpvar();
			this->vmiSTART_MARK( v );
			this->compileChildren( mnx, CONTINUE_LABEL );
			this->vmiSET_COUNT_TO_MARK( v );
			this->vmiSYS_CALL( sysListAppend );			
		}
		this->continueFrom( contn );
	} else if ( nm == GNX_VECTOR ) {
		int v = this->tmpvar();
		this->vmiSTART_MARK( v );
		this->compileChildren( mnx, CONTINUE_LABEL );
		this->vmiSET_COUNT_TO_MARK( v );
		this->vmiSYS_CALL( sysNewVector );
		this->continueFrom( contn );
	} else if ( nm == GNX_SET and mnx->size() == 2 ) {
		Gnx dst = mnx->getChild( 1 );
		Gnx src = mnx->getChild( 0 );
		if ( dst->hasName( GNX_ID ) ) {
			VIdent vid( this, dst );
			this->compile1( src, CONTINUE_LABEL );
			this->vmiPOP( vid, true );
			this->continueFrom( contn );
		} else if ( dst->hasName( GNX_SEQ ) ) {
			std::vector< Gnx > vars;
			if ( this->tryFlatten( dst, GNX_ID, vars ) ) {
				this->compileN( src, vars.size(), CONTINUE_LABEL );

				for ( std::vector< Gnx >::reverse_iterator it = vars.rbegin(); it != vars.rend(); ++it ) {
					VIdent vid( this, *it );
					this->vmiPOP( vid, true );
				}

				this->continueFrom( contn );
			} else {
				throw Ginger::Mishap( "BIND not fully implemented [1]" );
			}

		} else {
			throw Ginger::Mishap( "SET not fully implemented yet" );
		}
	} else if ( nm == GNX_FN ) {
		this->compileGnxFn( mnx, contn );
	} else if ( nm == GNX_DEREF and mnx->size() == 1 ) {
		this->compileGnxDeref( mnx, contn );
	} else if ( nm == GNX_MAKEREF and mnx->size() == 1 ) {
		this->compileGnxMakeRef( mnx, contn );
	} else if ( nm == GNX_SETCONT and mnx->size() == 2 ) {
		this->compileGnxSetCont( mnx, contn );
	} else if ( 
		nm == GNX_THROW and 
		mnx->size() == 1 and 
		mnx->hasAttribute( GNX_THROW_EVENT ) and 
		mnx->hasAttribute( GNX_THROW_LEVEL )
	) {
		this->compileThrow( mnx, contn );
	} else if ( nm == GNX_ASSERT and mnx->size() == 1 ) {
		if ( mnx->hasAttribute( GNX_ASSERT_N, "1" ) ) {
			//	case-study: Adding New Element Type.
			this->compile1( mnx->getChild( 0 ), contn );
		} else if ( mnx->hasAttribute( GNX_ASSERT_TYPE, "bool" ) ) {
			//	case-study: Adding New Element Type.
			this->compile1( mnx->getChild( 0 ), CONTINUE_LABEL );
			this->vmiSET_SYS_CALL( sysCheckBool, 1 );
			this->continueFrom( contn );
		} else if ( mnx->hasAttribute( GNX_ASSERT_TAILCALL, "true" ) ) {			
			throw Ginger::Mishap( "Return found in non-tailcall position" );
		} else {
			throw SystemError( "Unimplemented assertion" );
		}
	} else if ( nm == GNX_SELF_APP ) {
		this->compileGnxSelfCall( mnx, contn );
	} else if ( nm == GNX_SELF_CONSTANT ) {
		this->vmiSELF_CONSTANT();
	} else if ( nm == GNX_TRY && mnx->size() >= 1 ) {
		this->compileTry( mnx, contn );
	} else if ( nm == GNX_PROBLEM ) {
		throwProblem( mnx );
	} else {
		CompileQuery cq( this );
		if ( cq.isValidQuery( mnx ) ) {
			cq.compileNakedQuery( mnx, contn );
		} else {
			throw SystemError( "Invalid GNX" ).culprit( "Expression", mnx->toString() );
		}
	}	
}

void CodeGenClass::vmiESCAPE() {
	this->emitSPC( vmc_escape );
}

void CodeGenClass::compileThrow( Gnx mnx, LabelClass * contn ) {
	//	TODO: Push the supplied list of arguments as a list.
	#if 1	
		int v = this->tmpvar();
		this->vmiSTART_MARK( v );
		this->compileGnx( mnx->getChild( 0 ), CONTINUE_LABEL );
		this->vmiSET_COUNT_TO_MARK( v );
		this->vmiSYS_CALL( sysNewVector );			
	#else
		this->vmiPUSHQ( SYS_NIL );
	#endif
	//	Push the event name.
	this->vmiPUSHQ_SYMBOL( mnx->attribute( GNX_THROW_EVENT ), CONTINUE_LABEL );
	const std::string level( mnx->attribute( GNX_THROW_LEVEL ) );
	if ( level == "escape" ) {
		this->vmiESCAPE();
	} else {
		this->vmiSET_SYS_CALL( level == "panic" ? sysPanic : sysFailover, 2 );
	}
}

static Gnx getCatchReturn( Gnx try_gnx ) {
	for ( int i = 1; i < try_gnx->size(); i++ ) {
		Gnx clause = try_gnx->getChild( i );
		const std::string nm = clause->name();
		const int N = clause->size();
		if ( nm == GNX_CATCH_RETURN && N == 2 ) {
			return clause;
		}
	}
	return shared< Mnx >();
}

void CodeGenClass::eraseToMarkIfNeeded( const bool needed, const int mark ) {
	if ( needed ) {
		this->vmiERASE_MARK( mark );
	}
}

//	<try> 
//		APPLICATION
//		<catch.return> PATTERN EXPR </catch.return> 	<-- can be any position!
//		<catch.then> PATTERN EXPR </catch.then>* 
//		<catch.else> EXPR </catch.else> 				<-- can be any position!
//	</try>
void CodeGenClass::compileTry( Gnx mnx, LabelClass * contn ) {
	LabelClass done_label( this );
	LabelClass * done = done_label.jumpToJump( contn );

	Gnx appl = mnx->getChild( 0 );
	Gnx retn = getCatchReturn( mnx );
	const bool has_retn = !!retn;
	Arity aretn( has_retn ? retn->getChild( 0 )->attribute( GNX_PATTERN_ARITY, "+0" ) : "+0" );
	
	//	Because we tidy the stack up on escape, we have to make a note of
	//	where we will be tidying back to.
	const int mark = this->tmpvar();
	this->vmiSTART_MARK( mark );
	
	if ( has_retn ) {
		if ( aretn.isExact() ) {
			this->compileN( appl, aretn.count(), CONTINUE_LABEL );
		} else {
			this->vmiSTART_MARK( mark );
			this->compileGnx( appl, CONTINUE_LABEL );
		}
	} else {
		this->compileGnx( appl, CONTINUE_LABEL );
	}

	LabelClass continue_label( this );
	this->vmiBYPASS( &continue_label );

	//	The top two items are the event and escape return values.
	const int event = this->tmpvar();
	const int args = this->tmpvar();
	this->vmiPOP_INNER_SLOT( event );
	this->vmiPOP_INNER_SLOT( args );

	//	If we have no catch-return then we should clean up the stack now.
	this->eraseToMarkIfNeeded( not has_retn, mark  );
	
	bool seen_catch_return = false;
	for ( int i = 1; i < mnx->size(); i++ ) {
		Gnx clause = mnx->getChild( i );
		const std::string nm = clause->name();
		const int N = clause->size();
		if ( nm == GNX_CATCH_THEN && N == 2 && clause->hasAttribute( GNX_CATCH_THEN_EVENT ) ) {
			LabelClass next_label( this );
			this->vmiPUSH_INNER_SLOT( event );
			this->vmiIFNEQTO( refMakeSymbol( clause->attribute( GNX_CATCH_THEN_EVENT ) ), &next_label, CONTINUE_LABEL );
			
			//	If there is a catch-return we have to stack tidy
			this->eraseToMarkIfNeeded( has_retn, mark );

			Gnx pattern = clause->getChild( 0 );	//	Arguments to bind.
			Arity p_arity( pattern->attribute( GNX_PATTERN_ARITY, "0+") ); 
			Gnx expr = clause->getChild( 1 );		//	Expression to execute.
			this->vmiPUSH_INNER_SLOT( args );
			this->vmiCHECK_EXPLODE( p_arity );
			this->compileBind( pattern, expr, done );
			next_label.labelSet();
		} else if ( nm == GNX_CATCH_ELSE && N == 1 ) {
			
			//	If there is a catch-return we have to stack tidy
			this->eraseToMarkIfNeeded( has_retn, mark );

			this->compileGnx( clause->getChild( 0 ), done );
		} else if ( nm == GNX_CATCH_RETURN && N == 2 ) {
			//	Skip. Handled elsewhere.
			if ( seen_catch_return ) {
				throw Mishap( "Too many catch-returns" );
			}
			seen_catch_return = true;
		} else {
			throw SystemError( "Invalid catch clause" ).culprit( "Name", nm );
		}
	}
	
	//	Otherwise the escape was untrapped and it must be escalated.
	this->vmiPUSH_INNER_SLOT( event );
	this->vmiPUSH_INNER_SLOT( args );
	this->vmiSET_SYS_CALL( sysNewVector, 2 );
	this->vmiPUSHQ_STRING( "Escalated escape", CONTINUE_LABEL );
	this->vmiSET_SYS_CALL( sysFailover, 2 );

	continue_label.labelSet();

	if ( has_retn ) {
		//	There was no escape attempted, so we continue with any catch-return
		//	statements.
		Gnx pattern = retn->getChild( 0 );
		Gnx expr = retn->getChild( 1 );

		//	The delivered values are on the stack. So the issue is whether
		//	or not the count agrees with the arity.pattern that is inferred
		//	by the simplifier.
		if ( aretn.isntExact() ) {
			//	If the pattern-arity was exact we have already discharged 
			//	that check by use of compileN. However if it is isnt exact
			//	we have to verify the dynamic count.
			this->vmiCHECK_MARK( mark, aretn );
		}

		//	And now we bind to the pattern.
		this->compileBindDst( pattern );

		//	And then we can execute the catch.return code.
		this->compileGnx( expr, contn );
	} else {
		this->continueFrom( contn );
	}

	done_label.labelSet();
}

void CodeGenClass::compileChildren( Gnx mnx, LabelClass * contn ) {
	int n = mnx->size();
	for ( int i = 0; i < n; i++ ) {
		this->compileGnx( mnx->getChild( i ), CONTINUE_LABEL );
	}	
	this->continueFrom( contn );
}

void CodeGenClass::compile1( Gnx mnx, LabelClass * contn ) {
	Ginger::Arity a( mnx->attribute( GNX_EVAL_ARITY, "+0" ) );
	if ( a.isntExact() ) {
		int n = this->current_slot;
		int v = this->tmpvar();
		this->vmiSTART_MARK( v );
		this->compileGnx( mnx, CONTINUE_LABEL );
		this->vmiCHECK_MARK1( v );
		this->current_slot = n;
		this->continueFrom( contn );
	} else if ( a.count() == 1 ) {
		this->compileGnx( mnx, contn );
	} else {
		throw Ginger::Mishap( "Wrong number of results in single context" ).culprit( "#Results", a.count() );
	}
}

void CodeGenClass::compileN( Gnx mnx, int N, LabelClass * contn ) {
	if ( N == 0 ) {
		this->compile0( mnx, contn );
	} else if ( N == 1 ) {
		this->compile1( mnx, contn );
	} else {
		Ginger::Arity a( mnx->attribute( GNX_EVAL_ARITY, "+0" ) );
		if ( a.isntExact() ) {
			int n = this->current_slot;
			int v = this->tmpvar();
			this->vmiSTART_MARK( v );
			this->compileGnx( mnx, CONTINUE_LABEL );
			this->vmiCHECK_MARK( v, N );
			this->current_slot = n;
			this->continueFrom( contn );
		} else if ( a.count() == N ) {
			this->compileGnx( mnx, contn );
		} else {
			throw Ginger::Mishap( "Wrong number of results in context of known arity" ).culprit( "#Results", a.count() );
		}
	}
}

void CodeGenClass::compileNelse( Gnx mnx, int N, LabelClass * ok, LabelClass * fail ) {
	Ginger::Arity a( mnx->attribute( GNX_EVAL_ARITY, "+0" ) );
	if ( a.isntExact() ) {
		int n = this->current_slot;
		int v = this->tmpvar();
		this->vmiSTART_MARK( v );
		this->compileGnx( mnx, CONTINUE_LABEL );
		this->vmiCHECK_MARK_ELSE( v, N, fail );
		this->current_slot = n;
		this->continueFrom( ok );
	} else if ( a.count() == N ) {
		this->compileGnx( mnx, ok );
	} else {
		this->continueFrom( fail );
	}
}

void CodeGenClass::compile0( Gnx mnx, LabelClass * contn ) {
	Ginger::Arity a( mnx->attribute( GNX_EVAL_ARITY, "+0" ) );
	if ( a.isntExact() ) {
		int n = this->current_slot;
		int v = this->tmpvar();
		this->vmiSTART_MARK( v );
		this->compileGnx( mnx, CONTINUE_LABEL );
		this->vmiCHECK_MARK0( v );
		this->current_slot = n;
		this->continueFrom( contn );
	} else if ( a.count() == 0 ) {
		this->compileGnx( mnx, contn );
	} else {
		throw Ginger::Mishap( "Wrong number of results in zero context" ).culprit( "#Results", a.count() );
	}
}

} // namespace Ginger
