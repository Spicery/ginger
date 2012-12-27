/******************************************************************************\
	Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
	of the GNU General Public License. This file is part of AppGinger.

    AppGinger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AppGinger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AppGinger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

#include "debug.hpp"

#include "numbers.hpp"
#include "codegen.hpp"

#include <iostream>
#include <sstream>

#include <stddef.h>
#include <string.h>


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

using namespace std;
using namespace Ginger;


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


void CodeGenClass::emitVAR_REF( Gnx id ) {
	if ( id->hasAttribute( VID_SCOPE, "local" ) ) {
		this->emitRef( ToRef( id->attributeToInt( VID_SLOT ) ) );
	} else {
		this->emitValof( resolveGlobal( id ) );
	}
}

void CodeGenClass::emitVIDENT_REF( const VIdent & id ) {
	if ( id.isLocal() ) {
		this->emitRef( ToRef( id.getSlot() ) );
	} else if ( id.isGlobal() ) {
		this->emitValof( id.getValof() );
	} else if ( id.isConstant() ) {
		this->emitRef( id.getRef() );
	} else {
		throw Ginger::SystemError( "Internal error" );
	}
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
	this->emitRef( ToRef( n ) );
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
	this->emitSPC( vmc_set_syscall );
	this->emitRef( ToRef( A ) );
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
			if ( Numbers::MIN_SMALL <= n && n <= Numbers::MAX_SMALL ) {
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
	Package * def_pkg = this->vm->getPackage( id->attribute( VID_DEF_PKG ) );
	return def_pkg->fetchAbsoluteValof( id->attribute( VID_NAME ) );
}

void CodeGenClass::vmiPOP_INNER_SLOT( int slot ) {
	this->emitSPC( vmc_pop_local );
	this->emitRef( ToRef( slot ) );	
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
		throw Ginger::SystemError( "Internal Error" );
	}
}

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
		throw Ginger::SystemError( "Internal Error" );
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
		throw Ginger::SystemError( "Internal Error" );
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
		this->emitRef( ToRef( slot ) );
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
			this->emitRef( ToRef( slot ) );
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

void CodeGenClass::vmiSET( int A ) {
	this->emitSPC( vmc_set );
	this->emitRef( ToRef( A ) );
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
		this->vmiEND_MARK( var );
		this->vmiPUSH( ident );
		this->emitSPC( vmc_calls );
	} else if ( ident.isGlobal() ) {
		this->emitSPC( vmc_end_call_global );
		this->emitRef( ToRef( var ) );
		this->emitVIDENT_REF( ident );
	} else {
		throw Ginger::Unreachable( __FILE__, __LINE__ );
	}
}

void CodeGenClass::vmiSET_CALL_ID( int in_arity, const VIdent & ident ) {
	if ( ident.isLocal() ) {
		this->vmiSET( in_arity );
		this->vmiPUSH( ident );
		this->emitSPC( vmc_calls );
	} else if ( ident.isGlobal() ) {
		this->emitSPC( vmc_set_call_global );
		this->emitRef( ToRef( in_arity ) );
		this->emitVIDENT_REF( ident );
	} else {
		throw Ginger::Unreachable( __FILE__, __LINE__ );
	}
}

//	Not very efficient :(
void CodeGenClass::vmiSET_CALL_INNER_SLOT( int in_arity, int slot ) {
	this->vmiSET( in_arity );
	this->vmiPUSH_INNER_SLOT( slot );
	this->emitSPC( vmc_calls );
}


void CodeGenClass::vmiEND1_CALLS( int var ) {
	this->emitSPC( vmc_end1_calls );
	this->emitRef( ToRef( var ) );
}

void CodeGenClass::vmiSET_CALLS( int in_arity ) {
	this->emitSPC( vmc_set_calls );
	this->emitRef( ToRef( in_arity ) );
}

void CodeGenClass::vmiSTART_MARK( int v ) {
	this->emitSPC( vmc_start_mark );
	this->emitRef( ToRef( v ) );
}

void CodeGenClass::vmiEND_MARK( int v ) {
	this->emitSPC( vmc_end_mark );
	this->emitRef( ToRef( v ) );
}

void CodeGenClass::vmiERASE_MARK( int var ) {
	this->emitSPC( vmc_erase_mark );
	this->emitRef( ToRef( var ) );
}

void CodeGenClass::vmiCHECK_COUNT( int v ) {
	this->emitSPC( vmc_check_count );
	this->emitRef( ToRef( v ) );
}

void CodeGenClass::vmiCHECK_MARK1( int v ) {
	this->emitSPC( vmc_check_mark1 );
	this->emitRef( ToRef( v ) );
}

//	Do we ever generate this?
void CodeGenClass::vmiCHECK_MARK0( int v ) {
	this->emitSPC( vmc_check_mark0 );
	this->emitRef( ToRef( v ) );
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
	this->emitSPC( vmc_and );
	dst->labelInsert();
}

void CodeGenClass::vmiOR( LabelClass * dst ) {
	this->emitSPC( vmc_or );
	dst->labelInsert();
}

void CodeGenClass::vmiABS_AND( LabelClass * dst ) {
	this->emitSPC( vmc_absand );
	dst->labelInsert();
}

void CodeGenClass::vmiABS_OR( LabelClass * dst ) {
	this->emitSPC( vmc_absor );
	dst->labelInsert();
}

void CodeGenClass::vmiIFNOT( LabelClass * d, LabelClass * contn ) {
	this->vmiIFSO( contn, d );
}

void CodeGenClass::vmiIFSO( LabelClass * dst ) {
	if ( dst->isntReturn() ) {
		this->emitSPC( vmc_ifso );
		dst->labelInsert();
	} else {
		this->emitSPC( vmc_return_ifso );
	}	
}

void CodeGenClass::vmiIFNOT( LabelClass * dst ) {
	if ( dst->isntReturn() ) {
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

void CodeGenClass::vmiIF( bool sense, LabelClass * dst, LabelClass * contn ) {
	if ( sense ) {
		this->vmiIFSO( dst, contn );
	} else {
		this->vmiIFNOT( dst, contn );
	}
}

void CodeGenClass::vmiGOTO( LabelClass * d ) {
	if ( d->isReturn() ) {
		this->emitSPC( vmc_return );
	} else if ( d == CONTINUE_LABEL ) {
		throw Ginger::SystemError( "Trying to GOTO a fake label" );
	} else {
		this->emitSPC( vmc_goto );
		d->labelInsert();
	}
}

void CodeGenClass::vmiDECR( const long d ) {
	this->vmiINCR( -d );
}

void CodeGenClass::vmiTEST( 
	const VIdent & vid0, 
	CMP_OP cmp_op, 
	const VIdent & vid1, 
	LabelClass * dst 
) { 
	if ( vid0.isLocal() and vid1.isLocal() ) {
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
		xfr.xfrRef( ToRef( this->nresults ) );		//	raw R
		xfr.xfrRef( ToRef( this->nlocals ) );		//	raw N
		xfr.xfrRef( ToRef( this->ninputs ) );		//	raw A
		
	
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
		*p++ = ToRef( this->nresults );
		*p++ = ToRef( this->nlocals );
		*p++ = ToRef( this->ninputs );
		Ref * func = p;
		if ( fnkey != sysCoreFunctionKey ) throw Ginger::Unreachable( __FILE__, __LINE__ );
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
	return nm == VAR or nm == ID or nm == CONSTANT;
}

static bool isConstantIntGnx( Gnx gnx ) {
	return gnx->name() == CONSTANT and gnx->hasAttribute( CONSTANT_TYPE, "int" );
}

void CodeGenClass::compileIfTest( bool sense, Gnx mnx, LabelClass * dst, LabelClass * contn ) {
	bool done = false;

	//	This next section should be refactored.
	if ( mnx->name() == SYSAPP ) {
		SysMap::iterator it = sysMap.find( mnx->attribute( SYSAPP_NAME ) );
		if ( it != sysMap.end() ) {
			const SysInfo & info = it->second;
			if ( 
				info.isCmpOp() and mnx->size() == 2 and
				isVIdentable( mnx->child( 0 ) ) and
				isVIdentable( mnx->child( 1 ) )
			) {
				VIdent vid0( this, mnx->child( 0 ) );
				VIdent vid1( this, mnx->child( 1 ) );
				this->compileComparison( sense, vid0, info.cmp_op, vid1, dst, contn );
				done = true; 	//	*** Processed the special case ***
			}
		}
	}

	if ( not done ) {
		if ( mnx->name() == OR && mnx->size() == 2 ) {
			LabelClass e( this );
			this->compileIfTest( sense, mnx->child( 0 ), e.jumpToJump( dst ), CONTINUE_LABEL );
			this->compileIfTest( sense, mnx->child( 1 ), e.jumpToJump( dst ), e.jumpToJump( contn ) );
			e.labelSet();
		} else if ( mnx->name() == AND && mnx->size() == 2 ) {
			LabelClass e( this );
			this->compileIfTest( sense, mnx->child( 0 ), CONTINUE_LABEL, e.jumpToJump( contn ) );
			this->compileIfTest( sense, mnx->child( 1 ), e.jumpToJump( dst ), e.jumpToJump( contn ) );
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
	this->vmiTEST( vid0, cmp_op, vid1, dst );
	this->continueFrom( contn );
}

VIdent & CompileQuery::getLoopVar() {
	if ( this->loop_var == NULL ) {
		throw Ginger::Mishap( "Internal error (getLoopVar): loop_var not set" );
	}
	return *this->loop_var;
}

void CompileQuery::compileQueryInit( Gnx query, LabelClass * contn ) {
	const string & nm = query->name();
	if ( nm == FROM ) {
		
		Gnx var( query->child( 0 ) );
		this->setLoopVar( new VIdent( this->codegen, var ) );
		
		Gnx start_expr( query->child( 1 ) );
		this->codegen->compile1( start_expr, CONTINUE_LABEL );
		this->codegen->vmiPOP( var, false );
		
		if ( query->size() >= 3 ) {
			Gnx end_expr( query->child( 2 ) );
			int tmp_end_expr = this->codegen->tmpvar();
			query->putAttribute( "tmp.end.expr", tmp_end_expr );
			this->codegen->compile1( end_expr, CONTINUE_LABEL );
			this->codegen->vmiPOP_INNER_SLOT( tmp_end_expr );
		}
		
	} else if ( nm == IN ) {

		this->setLoopVar( new VIdent( this->codegen, query->child( 0 ) ) );
	
		this->codegen->compile1( query->child( 1 ), CONTINUE_LABEL );
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
	
	} else {
		throw Ginger::SystemError( "Not implemented general queries" );
	}
	this->codegen->continueFrom( contn );
}

void CompileQuery::compileQueryNext( Gnx query, LabelClass * contn ) {
	const string & nm = query->name();
	if ( nm == FROM ) {

		//	Obvious candidate for a merged instruction.
		this->codegen->vmiPUSH( this->getLoopVar() );
		this->codegen->vmiINSTRUCTION( vmc_incr );
		this->codegen->vmiPOP( this->getLoopVar(), false );

	} else if ( nm == IN ) {
		//	Nothing.
	} else {
		throw Ginger::SystemError( "Not implemented general queries" );
	}
	this->codegen->continueFrom( contn );
}

/**	Compiles a query so that if the query evaluates to true we jump to label 
	'dst' otherwise we continue at label 'contn'.
*/
void CompileQuery::compileQueryIfSo( Gnx query, LabelClass * dst, LabelClass * contn ) {
	const string & nm = query->name();
	if ( nm == FROM ) {
		if ( query->size() >= 3 ) {
			VIdent end_expr( query->attributeToInt( "tmp.end.expr" ) );
			this->codegen->compileComparison( true, this->getLoopVar(), CMP_LTE, end_expr, dst, contn );		
		} else {
			this->codegen->vmiGOTO( dst );
		}
	} else if ( nm == IN ) {
		int tmp_state = query->attributeToInt( "tmp.state" );
		int tmp_context = query->attributeToInt( "tmp.context" );
		int tmp_next_fn = query->attributeToInt( "tmp.next.fn" );

		this->codegen->vmiPUSH_INNER_SLOT( tmp_state );	
		this->codegen->vmiPUSH_INNER_SLOT( tmp_context );	
		this->codegen->vmiSET_CALL_INNER_SLOT( 2, tmp_next_fn );		
		this->codegen->vmiPOP_INNER_SLOT( tmp_state );
		this->codegen->vmiPOP( this->getLoopVar(), false );
		
		VIdent id_tmp_state( tmp_state );
		VIdent termin( SYS_TERMIN );
		this->codegen->compileComparison( id_tmp_state, CMP_NEQ, termin, dst, contn );
	} else {
		throw Ginger::SystemError( "Not implemented general queries" );
	}
}

void CompileQuery::compileFor( Gnx query, Gnx body, LabelClass * contn ) {
	LabelClass body_label( this->codegen );
	LabelClass test_label( this->codegen );
	this->compileQueryInit( query, &test_label );
	body_label.labelSet();
	this->codegen->compileGnx( body, CONTINUE_LABEL );
	this->compileQueryNext( query, CONTINUE_LABEL );
	test_label.labelSet();
	this->compileQueryIfSo( query, &body_label, contn );
}

void CodeGenClass::compileFor( Gnx query, Gnx body, LabelClass * contn ) {
	CompileQuery cq( this );
	cq.compileFor( query, body, contn );
}

void CodeGenClass::compileGnxSwitch( const int offset, const int switch_slot, int tmp_slot, Gnx mnx, LabelClass * contn ) {
	const int a = mnx->size() - offset;
	if ( a == 0 ) {
		this->continueFrom( contn );
	} else if ( a == 1 ) {
		this->compileGnx( mnx->child( offset ), contn );
	} else if ( a == 2 ) {
		LabelClass doneLab( this );
		if ( tmp_slot == -1 ) {
			//	Allocate tmp_slot on first use.
			tmp_slot = this->tmpvar();
		}
		this->compile1( mnx->child( offset ), CONTINUE_LABEL );
		this->vmiPOP_INNER_SLOT( tmp_slot );
		this->vmiTEST( switch_slot, CMP_NEQ, tmp_slot, doneLab.jumpToJump( contn ) );
		this->compileGnx( mnx->child( offset + 1 ), contn );
		doneLab.labelSet();		
	} else {
		LabelClass elseLab( this );
		LabelClass doneLab( this );

		if ( tmp_slot == -1 ) {
			//	Allocate tmp_slot on first use.
			tmp_slot = this->tmpvar();
		}
		this->compile1( mnx->child( offset ), CONTINUE_LABEL );
		this->vmiPOP_INNER_SLOT( tmp_slot );
		this->vmiTEST( switch_slot, CMP_NEQ, tmp_slot, &elseLab );

		this->compileGnx( mnx->child( offset + 1 ), doneLab.jumpToJump( contn ) );

		elseLab.labelSet();
		this->compileGnxSwitch( offset + 2, switch_slot, tmp_slot, mnx, contn );

		doneLab.labelSet();

	}
}

void CodeGenClass::compileGnxSwitch( Gnx mnx, LabelClass * contn ) {
	int switch_slot = this->tmpvar();
	this->compile1( mnx->child( 0 ), CONTINUE_LABEL );
	this->vmiPOP_INNER_SLOT( switch_slot );
	//	N.B. -1 is an invalid tmpvar.
	this->compileGnxSwitch( 1, switch_slot, -1, mnx, contn );
}

void CodeGenClass::compileGnxIf( int offset, Gnx mnx, LabelClass * contn ) {
	const int a = mnx->size() - offset;
	if ( a == 0 ) {
		this->continueFrom( contn );
	} else if ( a == 1 ) {
		this->compileGnx( mnx->child( offset ), contn );
	} else if ( a == 2 ) {
		//	codegen1( codegen, term_index( mnx, 0 ) );
		//	vmiIFNOT( codegen, d );
		LabelClass doneLab( this );
		this->compileIfNot( mnx->child( offset ), doneLab.jumpToJump( contn ), CONTINUE_LABEL );
		this->compileGnx( mnx->child( offset + 1 ), contn );
		doneLab.labelSet();
	} else {
		LabelClass elseLab( this );
		//	codegen1( codegen, term_index( mnx, 0 ) );
		//	vmiIFNOT( codegen, e );
		LabelClass doneLab( this );
		this->compileIfNot( mnx->child( offset ), &elseLab, CONTINUE_LABEL );
		this->compileGnx( mnx->child( offset + 1 ), doneLab.jumpToJump( contn ) );
		elseLab.labelSet();
		this->compileGnxIf( offset + 2, mnx, contn );
		doneLab.labelSet();
	}
}

void CodeGenClass::compileGnxIf( Gnx mnx, LabelClass * contn ) {
	this->compileGnxIf( 0, mnx, contn );
}

void CodeGenClass::compileGnxFn( Gnx mnx, LabelClass * contn ) {
	int args_count = mnx->attributeToInt( FN_ARGS_COUNT );
	int locals_count = mnx->attributeToInt( FN_LOCALS_COUNT );
	this->vmiFUNCTION( mnx->attribute( FN_NAME, EMPTY_FN_NAME ), locals_count, args_count );
	this->vmiENTER();
	LabelClass retn( this, true );
	this->compileGnx( mnx->child( 1 ), &retn );
	retn.labelSet();
	this->vmiRETURN();
	this->vmiPUSHQ( this->vmiENDFUNCTION() );
	this->continueFrom( contn );
}

void CodeGenClass::compileChildrenChecked( Gnx mnx, Arity arity ) {
	Ginger::Arity args_arity( mnx->attribute( ARGS_ARITY, "+0" ) );
	if ( arity == args_arity ) {
		this->compileChildren( mnx, CONTINUE_LABEL );
	} else if ( arity.isExact() ) {
		int v = this->tmpvar();
		this->vmiSTART_MARK( v );
		this->compileChildren( mnx, CONTINUE_LABEL );
		this->vmiSET( v );
		this->vmiCHECK_COUNT( arity.count() );
	} else {	
		throw Ginger::Unreachable( __FILE__, __LINE__ );
	}
}

void CodeGenClass::compileGnxSysApp( Gnx mnx, LabelClass * contn ) {
	const string & nm = mnx->attribute( SYSAPP_NAME );
	SysMap::iterator it = sysMap.find( nm );
	if ( it != sysMap.end() ) {
		const SysInfo & info = it->second;
		if ( info.isSysCall() ) {
			int v = this->tmpvar();
			this->vmiSTART_MARK( v );
			this->compileChildren( mnx, CONTINUE_LABEL );
			this->vmiSET( v );
			this->vmiSYS_CALL( info.syscall );
		} else if ( info.isVMOp() ) {
			//cerr << "VM OP" << endl;
			bool c0 = false;
			bool c1 = false;
			//cerr << "nm == +? " << nm << "? " << ( nm == "+" ) << endl;
			//cerr << "mnx->size() == 2? " << ( mnx->size() == 2 ) << endl;
			//cerr << "isConstantIntGnx( mnx->child( 0 ) )? " << ( isConstantIntGnx( mnx->child( 0 ) ) ) << endl;
			//cerr << "isConstantIntGnx( mnx->child( 1 ) )? " << ( isConstantIntGnx( mnx->child( 1 ) ) ) << endl;
			if ( 
				nm == "+" and mnx->size() == 2 and 
				( ( c0 = isConstantIntGnx( mnx->child( 0 ) ) ) or ( c1 = isConstantIntGnx( mnx->child( 1 ) ) ) )
			) {			
				//	TODO: We should shift the operator into being an incr/decr.
				//	Check which one gets optimised.
				this->compile1( mnx->child( c0 ? 1 : 0 ), CONTINUE_LABEL );
				this->vmiINCR( mnx->child( c0 ? 0 : 1 )->attributeToLong( CONSTANT_VALUE ) );
			} else if ( 
				nm == "-" and mnx->size() == 2 and isConstantIntGnx( mnx->child( 1 ) )
			) {				
				//	TODO: We should shift the operator into being an incr/decr.
				this->compile1( mnx->child( 0 ), CONTINUE_LABEL );
				this->vmiDECR( mnx->child( 1 )->attributeToLong( CONSTANT_VALUE ) );
			} else {
				this->compileChildrenChecked( mnx, info.in_arity );
				this->vmiINSTRUCTION( info.instruction );
			}
		} else if ( info.isCmpOp() ) {
			this->compileChildrenChecked( mnx, info.in_arity );
			this->vmiINSTRUCTION( cmpOpInstruction( info.cmp_op ) );
		}
	} else {
		throw SystemError( "Unknown system call" ).culprit( "Name", mnx->attribute( SYSAPP_NAME ) );
	}
	this->continueFrom( contn );
}

void CodeGenClass::compileGnxApp( Gnx mnx, LabelClass * contn ) {
	Gnx fn( mnx->child( 0 ) );
	Gnx args( mnx->child( 1 ) );
	Ginger::Arity aargs( args->attribute( ARITY, "+0" ) );
	
	if ( fn->name() == ID ) {
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

void CodeGenClass::compileGnxFor( Gnx mnx, LabelClass * contn ) {
	this->compileFor( mnx->child( 0 ), mnx->child( 1 ), contn );
}

void CodeGenClass::compileGnxDeref( Gnx mnx, LabelClass * contn ) {
	VIdent id( this, mnx->child( 0 ) );
	this->vmiPUSH( id );
	this->vmiDEREF();
}

void CodeGenClass::compileGnxMakeRef( Gnx mnx, LabelClass * contn ) {
	this->compile1( mnx->child( 0 ), contn );
	this->vmiMAKEREF();
}

void CodeGenClass::compileGnxSetCont( Gnx mnx, LabelClass * contn ) {
	VIdent id( this, mnx->child( 1 ) );
	this->compile1( mnx->child( 0 ), contn );
	this->vmiPUSH( id );
	this->vmiSETCONT();	
}

void CodeGenClass::compileGnxSelfCall( Gnx mnx, LabelClass * contn ) {
	Ginger::Arity args_arity( mnx->attribute( ARGS_ARITY, "+0" ) );
	if ( args_arity.isExact() ) {
		this->compileChildren( mnx, CONTINUE_LABEL );
		this->vmiSELF_CALL_N( args_arity.count() ); 
	} else {
		int v = this->tmpvar();
		this->vmiSTART_MARK( v );
		this->compileChildren( mnx, CONTINUE_LABEL );
		this->vmiSET( v );
		this->vmiSELF_CALL();
	}
}


Ref CodeGenClass::calcConstant( Gnx mnx ) {
	#ifdef DBG_CODEGEN
		std::cout << "calcConstant: " << mnx->toString() << std::endl;
	#endif
	const string & type = mnx->attribute( CONSTANT_TYPE );
	if ( type == "int" ) {
		return LongToRef( mnx->attributeToLong( CONSTANT_VALUE ) );
	} else if ( type == "bool" ) {
		return mnx->hasAttribute( CONSTANT_VALUE, "false" ) ? SYS_FALSE : SYS_TRUE;
	} else if ( type == "absent" ) {
		return SYS_ABSENT;
	} else if ( type == "undefined" ) {
		return SYS_UNDEFINED;
	} else if ( type == "indeterminate" ) {
		return SYS_INDETERMINATE;
	} else if ( type == "char" ) {
		const string & s = mnx->attribute( CONSTANT_VALUE );
		if ( not s.empty() ) {
			return CharToCharacter( s[ 0 ] );
		} else {
			throw Ginger::SystemError( "Invalid character string" );
		}
	} else if ( type == "string" ) {
		return this->vm->heap().copyString( mnx->attribute( CONSTANT_VALUE ).c_str() );
	} else if ( type == "double" ) {
		gngdouble_t d;
		const std::string& numtext( mnx->attribute( CONSTANT_VALUE ) );
		std::istringstream i( numtext );
		if ( not ( i >> d ) ) {
			throw Ginger::Mishap( "Format of double precision number incorrect" ).culprit( "Number", numtext );
		} else {
			return this->vm->heap().copyDouble( d );
		}
	} else if ( type == "symbol" ) {
		return refMakeSymbol( mnx->attribute( CONSTANT_VALUE ) );
	} else if ( type == SYSFN and mnx->hasAttribute( SYSFN_VALUE ) ) {
		Ref r = makeSysFn( this, mnx->attribute( SYSFN_VALUE ), SYS_UNDEFINED );
		if ( r == SYS_UNDEFINED ) {
			throw Ginger::Mishap( "No such system function" ).culprit( "Function", mnx->attribute( SYSFN_VALUE ) );
		}
		return r;
	} else if ( type == "sysclass" ) {
		/*
		<sysclass value="Absent"/>          ### class for absent
		<sysclass value="Bool"/>            ### class for true & false
		<sysclass value="SmallInt"/>        ### class for 'small' integers
		<sysclass value="Float"/>           ### class for floats
		<sysclass value="String"/>          ### class for strings
		<sysclass value="Char"/>            ### class for characters
		<sysclass value="Nil"/>             ### class for nil
		<sysclass value="Pair"/>            ### class for list pairs
		<sysclass value="Vector"/>          ### class for vectors
		<sysclass value="Class"/>           ### class for classes
		*/
		const std::string& cname( mnx->attribute( CONSTANT_VALUE ) );
		if ( cname == "Absent" ) {
			return sysAbsentKey;
		} else if ( cname == "Bool" ) {
			return sysBoolKey;
		} else if ( cname == "Small" ) {
			return sysSmallKey;
		} else if ( cname == "Double" ) {
			//	TODO: Add Float?
			return sysDoubleKey;
		} else if ( cname == "String" ) {
			return sysStringKey;
		} else if ( cname == "Char" ) {
			return sysCharKey;
		} else if ( cname == "Nil" ) {
			return sysNilKey;
		} else if ( cname == "Pair" ) {
			return sysPairKey;
		} else if ( cname == "Vector" ) {
			return sysVectorKey;
		} else if ( cname == "Class" ) {
			return sysClassKey;
		} else {
			//	TODO: Function keys - not as easy as the documentation makes out.
			//	TODO: Map keys - same deal
			//			<sysclass value="Fn"/>              ### class for function objects
			throw Ginger::SystemError( "Constant not recognised" ).culprit( "Expression", mnx->toString() );
		}
	} else {
		throw Ginger::SystemError( "Constant not recognised" ).culprit( "Expression", mnx->toString() );
	}
}

void CodeGenClass::compileGnxConstant( Gnx mnx, LabelClass * contn ) {
	this->vmiPUSHQ( this->calcConstant( mnx ), contn );
}

static void throwProblem( Gnx mnx ) {
	const string & sev = mnx->attribute( PROBLEM_SEVERITY );
	if ( sev == "rollback" ) {
		Ginger::Mishap mishap( mnx->attribute( PROBLEM_MESSAGE ) );
		MnxChildIterator mnxit( mnx );
		while ( mnxit.hasNext() ) {
			Gnx & culprit = mnxit.next();
			if ( culprit->hasName( CULPRIT ) && culprit->hasAttribute( CULPRIT_NAME ) && culprit->hasAttribute( CULPRIT_VALUE ) ) {
				mishap.culprit( culprit->attribute( CULPRIT_NAME ), culprit->attribute( CULPRIT_VALUE ) );
			}
		}
		throw mishap;
	} else {
		Ginger::SystemError mishap( mnx->attribute( PROBLEM_MESSAGE ) );
		MnxChildIterator mnxit ( mnx );
		while ( mnxit.hasNext() ) {
			Gnx & culprit = mnxit.next();
			if ( culprit->hasName( CULPRIT ) && culprit->hasAttribute( CULPRIT_NAME ) && culprit->hasAttribute( CULPRIT_VALUE ) ) {
				mishap.culprit( culprit->attribute( CULPRIT_NAME ), culprit->attribute( CULPRIT_VALUE ) );
			}
		}
		throw mishap;
	}
}

/**
	Boolean test, bool_vs_abs == true
	Absent test,  bool_vs_abs == false
	AND and_vs_or == true
	OR  and_vs_or == false
 */

void CodeGenClass::compileBoolAbsAndOr( bool bool_vs_abs, bool and_vs_or, Gnx mnx, LabelClass * contn ) {
	LabelClass e( this );
	LabelClass *end = e.jumpToJump( contn );
	this->compile1( mnx->child( 0 ), CONTINUE_LABEL );
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
	this->compile1( mnx->child( 1 ), end );
	e.labelSet();	
}

void CodeGenClass::compileErase( Gnx mnx, LabelClass * contn ) {
	int v = this->tmpvar();
	this->vmiSTART_MARK( v );
	this->compileChildren( mnx, CONTINUE_LABEL );
	this->vmiERASE_MARK( v );
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
	if ( nm == CONSTANT ) {
		this->compileGnxConstant( mnx, contn );
	} else if ( nm == ID ) {
		VIdent vid( this, mnx );
		this->vmiPUSH( vid, contn );
	} else if ( nm == IF ) {
		this->compileGnxIf( mnx, contn );
	} else if ( nm == SWITCH && mnx->size() >= 1 ) {
		this->compileGnxSwitch( mnx, contn );
	} else if ( nm == OR && mnx->size() == 2 ) {
		this->compileBoolAbsAndOr( true, false, mnx, contn );
	} else if ( nm == AND && mnx->size() == 2 ) {
		this->compileBoolAbsAndOr( true, true, mnx, contn );
	} else if ( nm == ABSOR && mnx->size() == 2 ) {
		this->compileBoolAbsAndOr( false, false, mnx, contn );
	} else if ( nm == ABSAND && mnx->size() == 2 ) {
		this->compileBoolAbsAndOr( false, true, mnx, contn );
	} else if ( nm == SYSAPP ) {
		this->compileGnxSysApp( mnx, contn );
	} else if ( nm == APP ) {
		this->compileGnxApp( mnx, contn );
	} else if ( nm == SEQ ) {
		this->compileChildren( mnx, contn );
	} else if ( nm == ERASE ) {
		this->compileErase( mnx, contn );
	} else if ( nm == BIND and mnx->size() == 2 ) {
		#ifdef DBG_CODEGEN
			cerr << "appginger/compileGnx/BIND" << endl;
			cerr << "  [[";
			mnx->child( 0 )->render( cerr );
			cerr << "]]" << endl;
			cerr << "  [[";
			mnx->child( 1 )->render( cerr );	
			cerr << "]]" << endl;
		#endif
		VIdent vid( this, mnx->child( 0 ) );
		#ifdef DBG_CODEGEN
			cerr << "  (VIdent::VIdent done)" << endl;
		#endif
		this->compile1( mnx->child( 1 ), CONTINUE_LABEL );
		this->vmiPOP( vid, false );
		this->continueFrom( contn );

	} else if ( nm == FOR and mnx->size() == 2 ) {
		this->compileGnxFor( mnx, contn );
	} else if ( nm == LIST ) {
		if ( mnx->size() == 0 ) {
			this->vmiPUSHQ( SYS_NIL, contn );
		} else {
			int v = this->tmpvar();
			this->vmiSTART_MARK( v );
			this->compileChildren( mnx, CONTINUE_LABEL );
			this->vmiSET( v );
			this->vmiSYS_CALL( sysNewList );			
			this->continueFrom( contn );
		}
	} else if ( nm == LIST_APPEND && mnx->size() == 2 ) {
		if ( mnx->child( 0 )->name() == LIST ) {
			int v = this->tmpvar();
			this->vmiSTART_MARK( v );
			this->compileChildren( mnx->child( 0 ), CONTINUE_LABEL );
			this->compileGnx( mnx->child( 1 ), CONTINUE_LABEL );
			this->vmiSET( v );
			this->vmiSYS_CALL( sysNewListOnto );
		} else {
			int v = this->tmpvar();
			this->vmiSTART_MARK( v );
			this->compileChildren( mnx, CONTINUE_LABEL );
			this->vmiSET( v );
			this->vmiSYS_CALL( sysListAppend );			
		}
		this->continueFrom( contn );
	} else if ( nm == VECTOR ) {
		int v = this->tmpvar();
		this->vmiSTART_MARK( v );
		this->compileChildren( mnx, CONTINUE_LABEL );
		this->vmiSET( v );
		this->vmiSYS_CALL( sysNewVector );
		this->continueFrom( contn );
	} else if ( nm == SET and mnx->size() == 2 ) {
		VIdent vid( this, mnx->child( 1 ) );
		this->compile1( mnx->child( 0 ), CONTINUE_LABEL );
		this->vmiPOP( vid, true );
		this->continueFrom( contn );
	} else if ( nm == FN ) {
		this->compileGnxFn( mnx, contn );
	} else if ( nm == DEREF and mnx->size() == 1 ) {
		this->compileGnxDeref( mnx, contn );
	} else if ( nm == MAKEREF and mnx->size() == 1 ) {
		this->compileGnxMakeRef( mnx, contn );
	} else if ( nm == SETCONT and mnx->size() == 2 ) {
		this->compileGnxSetCont( mnx, contn );
	} else if ( 
		nm == THROW and 
		mnx->size() == 1 and 
		mnx->hasAttribute( THROW_EVENT ) and 
		mnx->hasAttribute( THROW_LEVEL )
	) {
		this->compileThrow( mnx, contn );
	} else if ( nm == ASSERT and mnx->size() == 1 ) {
		if ( mnx->hasAttribute( ASSERT_N, "1" ) ) {
			//	case-study: Adding New Element Type.
			this->compile1( mnx->child( 0 ), contn );
		} else if ( mnx->hasAttribute( ASSERT_TYPE, "bool" ) ) {
			//	case-study: Adding New Element Type.
			this->compile1( mnx->child( 0 ), CONTINUE_LABEL );
			this->vmiSET_SYS_CALL( sysCheckBool, 1 );
			this->continueFrom( contn );			
		} else {
			throw Ginger::SystemError( "Unimplemented assertion" );
		}
	} else if ( nm == SELF_APP ) {
		this->compileGnxSelfCall( mnx, contn );
	} else if ( nm == SELF_CONSTANT ) {
		this->vmiSELF_CONSTANT();
	} else if ( nm == TRY && mnx->size() >= 1 ) {
		this->compileTry( mnx, contn );
	} else if ( nm == PROBLEM ) {
		throwProblem( mnx );
	} else if ( nm == FROM or nm == IN ) {
		throw Ginger::Mishap( "Naked queries not yet supported" ).culprit( "Expression", mnx->toString() );
	} else {
		throw Ginger::SystemError( "GNX not recognised" ).culprit( "Expression", mnx->toString() );
	}
}

void CodeGenClass::compileThrow( Gnx mnx, LabelClass * contn ) {
	//	Push the event name.
	this->vmiPUSHQ_STRING( mnx->attribute( THROW_EVENT ), CONTINUE_LABEL );
	//	TODO: Push the supplied list of arguments as a list.
	#if 1	
		int v = this->tmpvar();
		this->vmiSTART_MARK( v );
		this->compileGnx( mnx->child( 0 ), CONTINUE_LABEL );
		this->vmiSET( v );
		this->vmiSYS_CALL( sysNewList );			
	#else
		this->vmiPUSHQ( SYS_NIL );
	#endif
	const std::string level( mnx->attribute( THROW_LEVEL ) );
	this->vmiSET_SYS_CALL( level == "panic" ? sysPanic : sysFailover, 2 );
	//	Don't bother with the 		
	//		this->continueFrom( contn );
	//	as we panic!!
}


void CodeGenClass::compileTry( Gnx mnx, LabelClass * contn ) {
	cerr << SYS_MSG_PREFIX << "Warning: exception catching not implemented" << endl;
	this->compileGnx( mnx->child( 0 ), contn );
}

void CodeGenClass::compileChildren( Gnx mnx, LabelClass * contn ) {
	int n = mnx->size();
	for ( int i = 0; i < n; i++ ) {
		this->compileGnx( mnx->child( i ), CONTINUE_LABEL );
	}	
	this->continueFrom( contn );
}

void CodeGenClass::compile1( Gnx mnx, LabelClass * contn ) {
	Ginger::Arity a( mnx->attribute( "arity", "+0" ) );
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
		throw Ginger::Mishap( "Wrong number of results in single context" ).culprit( "#Results", "" + a.count() );
	}
}

void CodeGenClass::compile0( Gnx mnx, LabelClass * contn ) {
	Ginger::Arity a( mnx->attribute( "arity", "+0" ) );
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
		throw Ginger::Mishap( "Wrong number of results in zero context" ).culprit( "#Results", "" + a.count() );
	}
}

