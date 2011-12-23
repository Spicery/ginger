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

#include "plant.hpp"

#include <iostream>
using namespace std;

#include "destination.hpp"
#include "term.hpp"
#include "vmi.hpp"
#include "instruction.hpp"
#include "heap.hpp"
#include "machine.hpp"
#include "mishap.hpp"
#include "key.hpp"
#include "arity.hpp"
#include "ident.hpp"
#include "makesysfn.hpp"
#include "misclayout.hpp"
#include "sys.hpp"
#include "dict.hpp"
#include "syssymbol.hpp"
#include "syscheck.hpp"
#include "sysexception.hpp"

#ifndef NULL
#define NULL 0
#endif

PlantClass::PlantClass( Machine vm ) {
	this->nlocals = 0;
	this->ninputs = 0;
	this->slot = 0;
	this->vm = vm;
}

const InstructionSet & PlantClass::instructionSet() {
	return this->vm->instructionSet();
}

DestinationClass & PlantClass::newDestination() {
	this->destination_vector.push_back( DestinationClass( this ) );
	return this->destination_vector.back();
}


#define REFBITS ( 8 * sizeof( Ref ) )

Ref PlantClass::detach( const bool in_heap, Ref fnkey ) {
	unsigned long L = this->code_data->size();
	unsigned long preflight_size = OFFSET_FROM_FN_LENGTH_TO_KEY + 1 + L /* + M */;
	
	//	The preflighted size must fit into WORDBITS-8 bits.
	if ( ( preflight_size & ~TAGGG_MASK ) != 0 ) {
		throw Ginger::Mishap( "Procedure too large" );
	}
	
	if ( in_heap ) {
		
		// Disabled temporarily.
		//int M = ( L + REFBITS-1 ) % REFBITS;
	
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
	
		/* Disabled for a while
		//	For the moment, fake the mask bits.  Since we have no garbage
		//	collector this shouldn't be a problem!
		for ( int i = 0; i < M; i++ ) {
			xfr.xfrRef( ToRef( 0 ) );
		}
		*/
	
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
		return Ptr4ToRef( func );
	}
}


static int tmpvar( Plant plant ) {
	int n = plant->slot;
	plant->slot += 1;
	if ( plant->slot > plant->nlocals ) {
		plant->nlocals = plant->slot;
	}
	return n;
}

Ident PlantClass::newTmpIdent() {
	return identNewTmp( tmpvar( this ) );
}

void PlantClass::compileIf( bool sense, Term term, DestinationClass & dst ) {
	char flag1, flag2;
	int arg1, arg2;
	char op = '?'; 			//dummy
	Functor fnc = term_functor( term );
	// term_print( term );
	switch ( fnc ) {
	    case fnc_lte:
			if ( op == '?' ) op = 'l';
	    case fnc_lt:
			if ( op == '?' ) op = '<';
	    case fnc_eq:
			if ( op == '?' ) op = '=';
	    case fnc_gt:
			if ( op == '?' ) op = '>';
	    case fnc_gte: {
		    Term lhs = term_index( term, 0 );
			Functor flhs = term_functor( lhs );
		    Term rhs = term_index( term, 1 );
			Functor frhs = term_functor( rhs );
			if ( op == '?' ) op = 'g';
		    if ( flhs == fnc_id ) {
		    	IdTermClass * t = dynamic_cast< IdTermClass * >( lhs.get() );
				flag1 = 's';
				arg1 = t->ident()->getFinalSlot();
			} else if ( flhs == fnc_int ) {
				flag1 = 'i';
				arg1 = term_int_cont( lhs );
			} else {
				break;
			}
		    if ( frhs == fnc_id ) {
		    	IdTermClass * t = dynamic_cast< IdTermClass * >( rhs.get() );
				flag2 = 's';
				arg2 = t->ident()->getFinalSlot();
			} else if ( frhs == fnc_int ) {
				flag2 = 'i';
				arg2 = term_int_cont( rhs );
			} else {
				break;
			}
			
			this->vmiIF_RELOP( sense, flag1, arg1, op, flag2, arg2, dst );
			return;
	    }
	    default: {}
	}
	this->compile1( term );
	this->vmiIF( sense, dst );
}


void PlantClass::compileIfNot( Term term, DestinationClass & dst ) {
	this->compileIf( false, term, dst );
}

void PlantClass::compileIfSo( Term term, DestinationClass & dst ) {
	this->compileIf( true, term, dst );
}

void PlantClass::compileQueryInit( Term query ) {
	switch ( term_functor( query ) ) {
		case fnc_from: {
			FromTermClass * q = dynamic_cast< FromTermClass * >( query.get() );
			Term var = q->child( 0 );
			if ( term_functor( var ) != fnc_var ) throw;
			VarTermClass * v = dynamic_cast< VarTermClass * >( var.get() );
			Ident & ident = v->ident();
			
			Term start_expr = q->child( 1 );
			this->compile1( start_expr );
			this->vmiPOPID( ident );
			
			Term end_expr = q->child( 2 );
			q->end_expr_ident = this->newTmpIdent();
			this->compile1( end_expr );
			this->vmiPOPID( q->end_expr_ident );
			
			break;
		}
		case fnc_in: {
			InTermClass * q = dynamic_cast< InTermClass * >( query.get() );
			
			this->compile1( q->child( 1 ) );
			this->vmiINSTRUCTION( vmc_getiterator );
			q->next_fn = this->newTmpIdent();
			q->context = this->newTmpIdent();
			q->state = this->newTmpIdent();
			this->vmiPOPID( q->next_fn );
			this->vmiPOPID( q->context );
			this->vmiPOPID( q->state );
			
			break;
		}
		default:
			throw;
	}
}

void PlantClass::compileQueryNext( Term query ) {
	switch ( term_functor( query ) ) {
		case fnc_from: {
			FromTermClass * q = dynamic_cast< FromTermClass * >( query.get() );
			Term var = q->child( 0 );
			if ( term_functor( var ) != fnc_var ) throw;
			VarTermClass * v = dynamic_cast< VarTermClass * >( var.get() );
			Ident & ident = v->ident();
			
			//	Obvious candidate for a merged instruction.
			this->vmiPUSHID( ident );
			this->vmiINSTRUCTION( vmc_incr );
			this->vmiPOPID( ident );
			
			break;
		}
		case fnc_in: {
			//	Nothing.
			break;
		}
		default:
			throw;
	}
}

void PlantClass::compileQueryIfSo( Term query, DestinationClass & dst ) {
	switch ( term_functor( query ) ) {
		case fnc_from: {
			FromTermClass * q = dynamic_cast< FromTermClass * >( query.get() );
			Term var = q->child( 0 );
			if ( term_functor( var ) != fnc_var ) throw;
			VarTermClass * v = dynamic_cast< VarTermClass * >( var.get() );
			Ident & ident = v->ident();
			
			VmiRelOpFactory relop_factory( this );
			relop_factory.setLeft( ident );
			relop_factory.setLTE();
			relop_factory.setRight( q->end_expr_ident );
			relop_factory.ifSo( dst );

			break;
		}
		case fnc_in: {
			InTermClass * q = dynamic_cast< InTermClass * >( query.get() );
			Term var = q->child( 0 );
			if ( term_functor( var ) != fnc_var ) throw;
			VarTermClass * v = dynamic_cast< VarTermClass * >( var.get() );
			Ident & loopident = v->ident();

			Ident & state = q->state;
			Ident & context = q->context;
			Ident & next_fn = q->next_fn;

			this->vmiPUSHID( state );	
			this->vmiPUSHID( context );	
			this->vmiSET_CALL_ID( 2, next_fn );
			this->vmiPOPID( state );
			this->vmiPOPID( loopident );
			this->vmiIF_NEQ_ID_CONSTANT( state, SYS_TERMIN, dst );
			
			break;
		}
		default:
			throw;
	}
}

void PlantClass::compileFor( Term query, Term body ) {
	DestinationClass body_label( this );
	DestinationClass test_label( this );
	this->compileQueryInit( query );
	this->vmiGOTO( test_label );
	body_label.destinationSet();
	this->compileTerm( body );
	this->compileQueryNext( query );
	test_label.destinationSet();
	this->compileQueryIfSo( query, body_label );
}


void PlantClass::compileTerm( Term term ) {
	Functor fnc = term_functor( term );
	switch ( fnc ) {
		case fnc_int:
		case fnc_bool:
		case fnc_absent:
		case fnc_char:
		case fnc_list:
		case fnc_vector:
			this->vmiPUSHQ( term_ref_cont( term ) );
			return;
		case fnc_string: {
			this->vmiPUSHQ(
				this->vm->heap().copyString(
					term_string_cont( term )
				)
			);
			break;
		}
		case fnc_symbol: {
			this->vmiPUSHQ( refMakeSymbol( term_symbol_cont( term ) ) );
			break;
		}
        case fnc_eq:
        case fnc_lt:
        case fnc_lte:
        case fnc_gt:
        case fnc_gte:
        case fnc_mul:
        case fnc_div:
        case fnc_sub:
		case fnc_add: {
			this->compile1( term_index( term, 0 ) );
			this->compile1( term_index( term, 1 ) );
			this->vmiOPERATOR( fnc );
			break;
		}
		case fnc_incr_by:
		case fnc_decr_by: {
			int n = term_int_cont( term_index( term, 1 ) );
			this->compile1( term_index( term, 0 ) );
			this->vmiINCR( fnc == fnc_incr_by ? n : -n );
			break;
		}
		case fnc_package:
		case fnc_seq: {
			int n = term_count( term );
			for ( int i = 0; i < n ; i++ ) {
				this->compileTerm( term_index( term, i ) );
			}
			break;
		}
		case fnc_id: {
			IdTermClass * t = dynamic_cast< IdTermClass * >( term.get() );
			Ident id = t->ident();
			if ( id != NULL ) {
				if ( id->isLocal() && id->getFinalSlot() < 0 ) {
					throw Ginger::Mishap( "Ident record not assigned slot" ).culprit( "Identifier",  t->name().c_str() );
				}
				this->vmiPUSHID( id );
			} else {
				throw Ginger::Mishap( "Unlifted identifier" ).culprit( "Identifier",  t->name().c_str() );
			}
			break;
		}
		case fnc_deref: {
			Term id = term_index( term, 0 );
			if ( term_functor( id ) != fnc_id ) throw Ginger::SystemError( "Internal Error - deref" );
			this->compile1( id );
			this->vmiDEREF();
			break;
		}
		case fnc_makeref: {
			this->compile1( term_index( term, 0 ) );
			this->vmiMAKEREF();
			break;
		}
		case fnc_setcont: {
			Term id = term_index( term, 1 );
			if ( term_functor( id ) != fnc_id ) throw Ginger::SystemError( "Internal Error - setcont" );
			this->compile1( term_index( term, 0 ) );
			this->compile1( id );
			this->vmiSETCONT();
			break;
		}
		case fnc_bind: {
			Term var = term_index( term, 0 );
			if ( term_functor( var ) != fnc_var ) throw;
			VarTermClass * v = dynamic_cast< VarTermClass * >( var.get() );
			Ident & ident = v->ident();
			Term body = term_index( term, 1 );
			this->compile1( body );
			this->vmiNEWID( ident );
			this->vmiPOPID( ident );
			break;
		}
		case fnc_assign: {
			Term target = term_index( term, 1 );
			IdTermClass * t = dynamic_cast< IdTermClass * >( target.get() );
			if ( t ) {
				Ident & ident = t->ident();
				Term source = term_index( term, 0 );
				this->compile1( source );
				this->vmiPOPID( ident );
			} else {
				throw Ginger::Mishap( "TO BE DONE" ).culprit( "Term", functor_name( term_functor( target ) ) );
			}
			break;
		}
		case fnc_fn: {
			FnTermClass * ftc = dynamic_cast< FnTermClass * >( term.get() );
			Term body = ftc->child( 1 );
			
			if ( ftc->hasOuters() ) {
				#ifdef DBG_LIFTING
					cerr << "Function " << ftc->name() << " has outers" << endl;
				#endif
				int N = ftc->sizeOuters();
				for ( int i = 0; i < N; i++ ) {
					this->vmiPUSH_INNER_SLOT( ftc->outer( i )->getFinalSlot() );
				}
			}
			
			this->vmiFUNCTION( ftc->nlocals(), ftc->ninputs() );
			this->vmiENTER();
			this->compileTerm( body );
			this->vmiRETURN();
			this->vmiPUSHQ( this->vmiENDFUNCTION() );
			
			if ( ftc->hasOuters() ) {
				this->vmiSET_SYS_CALL( sysPartApply, ftc->sizeOuters() + 1 );
			}
			
			break;
		}
		case fnc_app: {
			Term fn = term_index( term, 0 );
			Term args = term_index( term, 1 );
			Arity aargs( args );
			//	plant_count( plant, args );
			IdTermClass * iterm = dynamic_cast< IdTermClass * >( fn.get() );
			if ( iterm ) {
				if ( aargs.isntExact() ) { 
					int v = tmpvar( this );
		        	this->vmiSTART_MARK( v );
					this->compileTerm( args );
					this->vmiEND_CALL_ID( v, iterm->ident() );
				} else {
					this->compileTerm( args );
					this->vmiSET_CALL_ID( aargs.count(), iterm->ident() );
				}
			} else if ( aargs.isntExact() ) {
				int v = tmpvar( this );
		        this->vmiSTART_MARK( v );
		        this->compileTerm(  args );
				this->compile1( fn );
				this->vmiEND1_CALLS( v );
			} else {
				this->compileTerm( args );
				this->compile1( fn );
				this->vmiSET_CALLS( aargs.count() );
			}
			break;
		}
		case fnc_syscall: {
			//	Note that reinterpret_cast falls foul of the ISO C++ rule that
			//	forbids interconversion between void* and function pointers.
			SysCall * sc = (SysCall *)( term_ref_cont( term ) );
			
			int v = tmpvar( this );
			this->vmiSTART_MARK( v );
			this->compileArgs( term );
			this->vmiSET( v );
			this->vmiSYS_CALL( sc );
			break;
		}
		case fnc_sysfn: {
			Ref r = makeSysFn( this, term_sysfn_cont( term ), SYS_UNDEF );
			if ( r == SYS_UNDEF ) {
				throw Ginger::Mishap( "No such system function" ).culprit( "Function", term_sysfn_cont( term ) );
			}
			this->vmiPUSHQ( r );			
			break;
		}
		case fnc_for: {
			// suppress unused argument ... will be used in the future
			// Term bindings = term_index( term, 0 );
			Term query = term_index( term, 0 );
			Term body = term_index( term, 1 );
			this->compileFor( query, body );
			break;
		}
		case fnc_if: {
			int a = term_count( term );
			if ( a == 2 ) {
				DestinationClass d( this );
				//	plant1( plant, term_index( term, 0 ) );
				//	vmiIFNOT( plant, d );
				this->compileIfNot( term_index( term, 0 ), d );
				this->compileTerm( term_index( term, 1 ) );
				d.destinationSet();
			} else if ( a == 3 ) {
				DestinationClass d( this );
				DestinationClass e( this );
				//	plant1( plant, term_index( term, 0 ) );
				//	vmiIFNOT( plant, e );
				this->compileIfNot( term_index( term, 0 ), e );
				this->compileTerm( term_index( term, 1 ) );
				this->vmiGOTO( d );
				e.destinationSet();
				this->compileTerm( term_index( term, 2 ) );
				d.destinationSet();
			} else {
				throw Ginger::Unreachable( __FILE__, __LINE__ );
			}
			break;
		}
		case fnc_not: {
			this->compile1( term_index( term, 0 ) );
			this->vmiNOT();
		 	break;
		}
		case fnc_throw: {
			this->compile1( term_index( term, 0 ) );
			this->vmiSET_SYS_CALL( sysPanic, 1 );
			break;
		}
		case fnc_assert_single: {
			//	case-study: Adding New Element Type.
			this->compile1( term_index( term, 0 ) );
			break;
		}
		case fnc_assert_bool: {
			//	case-study: Adding New Element Type.
			this->compile1( term_index( term, 0 ) );
			this->vmiSET_SYS_CALL( sysCheckBool, 1 );
			break;
		}
		case fnc_import: {
			//	Skip - is a pure environment effect.
			break;
		}
		default: {
			//cerr << "ABOUT TO THROW" << endl;
			throw Ginger::Mishap( "Syntax not implemented yet" ).culprit( "functor", functor_name( fnc ) );
		}
	}
}

void PlantClass::compileArgs( Term term ) {
	int n = term->count();
	for ( int i = 0; i < n; i++ ) {
		this->compileTerm( term->child( i ) );
	}
}

void PlantClass::compile1( Term term ) {
	Arity a( term );
	if ( a.isntExact() ) {
		int n = this->slot;
		int v = tmpvar( this );
		this->vmiSTART_MARK( v );
		this->compileTerm( term );
		this->vmiCHECK_MARK1( v );
		this->slot = n;
	} else if ( a.count() == 1 ) {
		this->compileTerm( term );
	} else {
		throw Ginger::Mishap( "Wrong number of results in single context" ).culprit( "#Results", "" + a.count() );
	}
}

void PlantClass::compile0( Term term ) {
	Arity a( term );
	if ( a.isntExact() ) {
		int n = this->slot;
		int v = tmpvar( this );
		this->vmiSTART_MARK( v );
		this->compileTerm( term );
		this->vmiCHECK_MARK0( v );
		this->slot = n;
	} else if ( a.count() == 0 ) {
		this->compileTerm( term );
	} else {
		throw Ginger::Mishap( "Wrong number of results in zero context" ).culprit( "#Results", "" + a.count() );
	}
}

void PlantClass::emitSPC( Instruction instr ) {
	const InstructionSet & ins = this->instructionSet();
	Ref instr_ptr = ins.lookup( instr );
	this->plantRef( instr_ptr );
}

void PlantClass::emitRef( Ref ref ) {
	this->plantRef( ref );
}

void PlantClass::emitValof( Valof *v ) {
	this->plantRef( ToRef( v ) );
}


