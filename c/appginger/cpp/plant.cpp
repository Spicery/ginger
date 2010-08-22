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

#include "plant.hpp"

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
#include "objlayout.hpp"
#include "sys.hpp"
#include "dict.hpp"

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

Ref PlantClass::detach( const bool in_heap ) {
	unsigned long L = this->code_data->size();
	unsigned long preflight_size = OFFSET_FROM_FN_LENGTH_TO_KEY + 1 + L /* + M */;
	
	//	The preflighted size must fit into WORDBITS-8 bits.
	if ( ( preflight_size & ~TAGGG_MASK ) != 0 ) {
		throw Mishap( "Procedure too large" );
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
		xfr.xfrRef( sysFunctionKey );
	
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
		Ref * func = p;
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
	return ident_new_tmp( tmpvar( this ) );
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
				arg1 = t->ident()->slot;
			} else if ( flhs == fnc_int ) {
				flag1 = 'i';
				arg1 = term_int_cont( lhs );
			} else {
				break;
			}
		    if ( frhs == fnc_id ) {
		    	IdTermClass * t = dynamic_cast< IdTermClass * >( rhs.get() );
				flag2 = 's';
				arg2 = t->ident()->slot;
			} else if ( frhs == fnc_int ) {
				flag2 = 'i';
				arg2 = term_int_cont( rhs );
			} else {
				break;
			}
			
			vmiIF_RELOP( this, sense, flag1, arg1, op, flag2, arg2, dst );
			return;
	    }
	    default: {}
	}
	this->compile1( term );
	vmiIF( sense, this, dst );
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
			vmiPOPID( this, ident );
			
			Term end_expr = q->child( 2 );
			q->end_expr_ident = this->newTmpIdent();
			this->compile1( end_expr );
			vmiPOPID( this, q->end_expr_ident );
			
			break;
		}
		case fnc_in: {
			InTermClass * q = dynamic_cast< InTermClass * >( query.get() );
			
			this->compile1( q->child( 1 ) );
			vmiINSTRUCTION( this, vmc_getiterator );
			q->next_fn = this->newTmpIdent();
			q->context = this->newTmpIdent();
			q->state = this->newTmpIdent();
			vmiPOPID( this, q->next_fn );
			vmiPOPID( this, q->context );
			vmiPOPID( this, q->state );
			
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
			vmiPUSHID( this, ident );
			vmiINSTRUCTION( this, vmc__incr );
			vmiPOPID( this, ident );
			
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

			vmiPUSHID( this, state );	
			vmiPUSHID( this, context );	
			vmiSET_CALL_ID( this, 2, next_fn );
			vmiPOPID( this, state );
			vmiPOPID( this, loopident );
			vmiIF_NEQ_ID_CONSTANT( this, state, sys_termin, dst );
			
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
	vmiGOTO( this, test_label );
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
			vmiPUSHQ( this, term_ref_cont( term ) );
			return;
		case fnc_string: {
			vmiPUSHQ(
				this,
				this->vm->heap().copyString(
					term_string_cont( term )
				)
			);
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
			vmiOPERATOR( this, fnc );
			break;
		}
		case fnc_incr_by:
		case fnc_decr_by: {
			int n = term_int_cont( term_index( term, 1 ) );
			this->compile1( term_index( term, 0 ) );
			vmiINCR( this, fnc == fnc_incr_by ? n : -n );
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
				int slot = id->slot;
				if ( slot >= 0 || !id->isLocal() ) {
					vmiPUSHID( this, id );
				} else {
					throw Mishap( "Ident record not assigned slot" ).culprit( "Identifier",  t->name().c_str() );
				}
			} else {
				throw Mishap( "Unlifted identifier" ).culprit( "Identifier",  t->name().c_str() );
			}
			break;
		}
		case fnc_dec: {
			Term var = term_index( term, 0 );
			if ( term_functor( var ) != fnc_var ) throw;
			VarTermClass * v = dynamic_cast< VarTermClass * >( var.get() );
			Ident & ident = v->ident();
			Term body = term_index( term, 1 );
			this->compile1( body );
			vmiPOPID( this, ident );
			break;
		}
		case fnc_assign: {
			Term lhs = term_index( term, 0 );
			IdTermClass * t = dynamic_cast< IdTermClass * >( lhs.get() );
			if ( t ) {
				Ident & ident = t->ident();
				Term rhs = term_index( term, 1 );
				this->compile1( rhs );
				vmiPOPID( this, ident );
			} else {
				throw ToBeDone().culprit( "Term", functor_name( term_functor( lhs ) ) );
			}
			break;
		}
		case fnc_fn: {
			Term body = term_index( term, 1 );
			vmiFUNCTION( this, term_fn_nlocals( term ), term_fn_ninputs( term ) );
			vmiENTER( this );
			this->compileTerm( body );
			vmiRETURN( this );
			vmiPUSHQ( this, vmiENDFUNCTION( this ) );
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
		        	vmiSTART( this, v );
					this->compileTerm( args );
					vmiEND_CALL_ID( this, v, iterm->ident() );
				} else {
					this->compileTerm( args );
					vmiSET_CALL_ID( this, aargs.count(), iterm->ident() );
				}
			} else if ( aargs.isntExact() ) {
				int v = tmpvar( this );
		        vmiSTART( this, v );
		        this->compileTerm(  args );
				this->compile1( fn );
				vmiEND1_CALLS( this, v );
			} else {
				this->compileTerm( args );
				this->compile1( fn );
				vmiSET_CALLS( this, aargs.count() );
			}
			break;
		}
		case fnc_syscall: {
			SysCall * sc = reinterpret_cast< SysCall * >( term_ref_cont( term ) );
			int v = tmpvar( this );
			vmiSTART( this, v );
			this->compileArgs( term );
			vmiSET( this, v );
			vmiSYS_CALL( this, sc );
			break;
		}
		case fnc_sysfn: {
			Package * p = this->vm->getPackage( STANDARD_LIBRARY );
			Ident id = p->lookup_or_add( term_sysfn_cont( term ), /*fetchFacet( "public" ),*/ fetchFacetSet( "public" ) );	
			if ( id->value_of->valof == sys_absent ) {
				Ref r = makeSysFn( this, term_sysfn_cont( term ), sys_undef );
				if ( r == sys_undef ) {
					throw Mishap( "No such system function" ).culprit( "Function", term_sysfn_cont( term ) );
				}
				id->value_of->valof = r;
			}
			vmiPUSHID( this, id );
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
				vmiGOTO( this, d );
				e.destinationSet();
				this->compileTerm( term_index( term, 2 ) );
				d.destinationSet();
			} else {
				this_never_happens();
			}
			break;
		}
		case fnc_not: {
			this->compile1( term_index( term, 0 ) );
			vmiNOT( this );
		 	break;
		}
		case fnc_import: {
			//	Skip - is a pure environment effect.
			break;
		}
		default: {
			throw Mishap( "PLANT_TERM: Not implemented yet" ).culprit( "functor", functor_name( fnc ) );
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
		vmiSTART( this, v );
		this->compileTerm( term );
		vmiCHECK1( this, v );
		this->slot = n;
	} else if ( a.count() == 1 ) {
		this->compileTerm( term );
	} else {
		throw Mishap( "Wrong number of results in single context" ).culprit( "#Results", "" + a.count() );
	}
}

void PlantClass::compile0( Term term ) {
	Arity a( term );
	if ( a.isntExact() ) {
		int n = this->slot;
		int v = tmpvar( this );
		vmiSTART( this, v );
		this->compileTerm( term );
		vmiCHECK0( this, v );
		this->slot = n;
	} else if ( a.count() == 0 ) {
		this->compileTerm( term );
	} else {
		throw Mishap( "Wrong number of results in zero context" ).culprit( "#Results", "" + a.count() );
	}
}
