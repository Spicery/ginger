#include "plant.hpp"

#include "destination.hpp"
#include "term.hpp"
#include "vmi.hpp"
#include "heap.hpp"
#include "machine.hpp"
#include "mishap.hpp"
#include "key.hpp"
#include "arity.hpp"
#include "ident.hpp"

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

Ref PlantClass::detach() {
	//HeapClass & hp = this->vm->heap();
	int L = this->code_data->size();		//	alt
	int M = ( L + REFBITS-1 ) % REFBITS;
	int preflight_size = 5 + L + M;

	XfrClass xfr( *this->vm, preflight_size );

	//	L has to be a "procedure-length" value if heap scanning is
	//	to be preserved - and I would like that for a variety of
	//	reasons.
	//
	xfr.xfrRef( ToRef( L ) );                 	// 	raw L - wrong!
	xfr.xfrRef( ToRef( this->nresults ) );		//	raw R
	xfr.xfrRef( ToRef( this->nlocals ) );		//	raw N
	xfr.xfrRef( ToRef( this->ninputs ) );		//	raw A

	xfr.setOrigin();
	xfr.xfrRef( sysFnKey );

	xfr.xfrVector( *this->code_data );	//	alt

	//	For the moment, fake the mask bits.  Since we have no garbage
	//	collector this shouldn't be a problem!
	for ( int i = 0; i < M; i++ ) {
		xfr.xfrRef( ToRef( 0 ) );
	}

	return xfr.end();
}


static int tmpvar( Plant plant ) {
	int n = plant->slot;
	plant->slot += 1;
	if ( plant->slot > plant->nlocals ) {
		plant->nlocals = plant->slot;
	}
	return n;
}


static void plant_if( bool sense, Plant plant, Term term, DestinationClass & dst ) {
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
				flag1 = 's';
				arg1 = term_named_ident( lhs )->slot;
			} else if ( flhs == fnc_int ) {
				flag1 = 'i';
				arg1 = term_int_cont( lhs );
			} else {
				break;
			}
		    if ( frhs == fnc_id ) {
				flag2 = 's';
				arg2 = term_named_ident( rhs )->slot;
			} else if ( frhs == fnc_int ) {
				flag2 = 'i';
				arg2 = term_int_cont( rhs );
			} else {
				break;
			}
			vmiIFNOT_RELOP( plant, flag1, arg1, op, flag2, arg2, dst );
			return;
	    }
	    default: {}
	}
	plant1( plant, term );
	vmiIF( sense, plant, dst );
}

static void plant_ifnot( Plant plant, Term term, DestinationClass & dst ) {
	plant_if( false, plant, term, dst );
}

static void plant_ifso( Plant plant, Term term, DestinationClass & dst ) {
	plant_if( true, plant, term, dst );
}

void plant_term( Plant plant, Term term ) {
	Functor fnc = term_functor( term );
	/*Role in = term_role( term );
	if ( IsImmediate( role ) ) {
		vmiPUSHQ( plant, term_ref_cont( term ) );
		return;
	}*/
	switch ( fnc ) {
		case fnc_int:
		case fnc_bool:
		case fnc_absent:
		case fnc_char:
			vmiPUSHQ( plant, term_ref_cont( term ) );
			return;
		case fnc_string: {
			vmiPUSHQ(
				plant,
				plant->vm->heap().copyString(
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
			plant1( plant, term_index( term, 0 ) );
			plant1( plant, term_index( term, 1 ) );
			vmiOPERATOR( plant, fnc );
			break;
		}
		case fnc_incr_by:
		case fnc_decr_by: {
			int n = term_int_cont( term_index( term, 1 ) );
			plant1( plant, term_index( term, 0 ) );
			vmiINCR( plant, fnc == fnc_incr_by ? n : -n );
			break;
		}
		case fnc_appspc: {
			plant_term( plant, term_index( term, 1 ) );
			Instruction instr = functor_inline( term_functor( term_index( term, 0 ) ) );
			vmiAPPSPC( plant, instr );
			break;
		}
		case fnc_seq: {
			int n = term_arity( term );
			for ( int i = 0; i < n ; i++ ) {
				plant_term( plant, term_index( term, i ) );
			}
			break;
		}
		case fnc_id: {
			Ident id = term_named_ident( term );
			if ( id != NULL ) {
				int slot = id->slot;
				if ( slot >= 0 || !id->is_local ) {
					vmiPUSHID( plant, id );
				} else {
					mishap( "Ident record not assigned slot (%s)", term_named_string( term ).c_str() );
				}
			} else {
				mishap( "Unlifted identifier %s", term_named_string( term ).c_str() );
			}
			break;
		}
		case fnc_dec: {
			Term var = term_index( term, 0 );
			if ( term_functor( var ) != fnc_var ) throw;
			Ident & ident = term_named_ident( var );
			Term body = term_index( term, 1 );
			plant1( plant, body );
			vmiPOPID( plant, ident );
			break;
		}
		case fnc_assign: {
			Term lhs = term_index( term, 0 );
			if ( term_is_id( lhs ) ) {
				Ident & ident = term_named_ident( lhs );
				Term rhs = term_index( term, 1 );
				plant1( plant, rhs );
				vmiPOPID( plant, ident );
			} else {
				throw ToBeDone().culprit( "Term", functor_name( term_functor( lhs ) ) );
			}
			break;
		}
		case fnc_fn: {
			Term body = term_index( term, 1 );
			vmiFUNCTION( plant, term_fn_nlocals( term ), term_fn_ninputs( term ) );
			vmiENTER( plant );
			plant_term( plant, body );
			vmiRETURN( plant );
			vmiPUSHQ( plant, vmiENDFUNCTION( plant ) );
			break;
		}
		case fnc_app: {
			Term fn = term_index( term, 0 );
			Term args = term_index( term, 1 );
			int aargs = arity_term( args );
			//	plant_count( plant, args );
			if ( term_is_id( fn ) ) {
				if ( aargs == DONTKNOW ) {
					int v = tmpvar( plant );
		        	vmiSTART( plant, v );
					plant_term( plant, args );
					vmiEND_CALL_ID( plant, v, term_named_ident( fn ) );
				} else {
					plant_term( plant, args );
					vmiSET_CALL_ID( plant, aargs, term_named_ident( fn ) );
				}
			} else if ( aargs == DONTKNOW ) {
				int v = tmpvar( plant );
		        vmiSTART( plant, v );
		        plant_term( plant, args );
				plant1( plant, fn );
				vmiEND1_CALLS( plant, v );
			} else {
				plant_term( plant, args );
				plant1( plant, fn );
				vmiSET_CALLS( plant, aargs );
			}
			break;
		}
		case fnc_syscall: {
			Ref sc = term_ref_cont( term );
			vmiSYS_CALL( plant, sc );
			break;
		}
		case fnc_for: {
			// suppress unused argument ... will be used in the future
			// Term bindings = term_index( term, 0 );
			Term tests = term_index( term, 1 );
			Term body = term_index( term, 2 );
			DestinationClass body_label( plant );
			DestinationClass test_label( plant );
			vmiGOTO( plant, test_label );
			body_label.destinationSet();
			plant_term( plant, body );
			test_label.destinationSet();
			{
				int i, len = term_arity( tests );
				for ( i = 0; i < len; i++ ) {
					plant_ifso( plant, term_index( tests, i ), body_label );
				}
			}
			break;
		}
		case fnc_if: {
			int a = term_arity( term );
			if ( a == 2 ) {
				DestinationClass d( plant );
				//	plant1( plant, term_index( term, 0 ) );
				//	vmiIFNOT( plant, d );
				plant_ifnot( plant, term_index( term, 0 ), d );
				plant_term( plant, term_index( term, 1 ) );
				d.destinationSet();
			} else if ( a == 3 ) {
				DestinationClass d( plant );
				DestinationClass e( plant );
				//	plant1( plant, term_index( term, 0 ) );
				//	vmiIFNOT( plant, e );
				plant_ifnot( plant, term_index( term, 0 ), e );
				plant_term( plant, term_index( term, 1 ) );
				vmiGOTO( plant, d );
				e.destinationSet();
				plant_term( plant, term_index( term, 2 ) );
				d.destinationSet();
			} else {
				this_never_happens();
			}
			break;
		}
		case fnc_not: {
			plant1( plant, term_index( term, 0 ) );
			vmiNOT( plant );
		 	break;
		}
		default: {
			mishap( "PLANT_TERM: Not implemented yet, functor '%s'", functor_name( fnc ) );
		}
	}
}

/*  void plant_count( Plant plant, Term term ) {                            */
/*  	int a = arity_term( term );                                         */
/*  	if ( a != DONTKNOW ) {                                              */
/*  		plant_term( plant, term );                                      */
/*  		vmiPUSHQ( plant, IntToRef( a ) );                               */
/*  	} else {                                                            */
/*  		int n = plant->slot;                                            */
/*  		int v = tmpvar( plant );                                        */
/*  		vmiSTART( plant, v );                                           */
/*  		plant_term( plant, term );                                      */
/*  		vmiEND( plant, v );                                             */
/*  		plant->slot = n;                                                */
/*  	}                                                                   */
/*  }                                                                       */

void plant1( Plant plant, Term term ) {
	int a = arity_term( term );
	if ( a == DONTKNOW ) {
		int n = plant->slot;
		int v = tmpvar( plant );
		vmiSTART( plant, v );
		plant_term( plant, term );
		vmiCHECK1( plant, v );
		plant->slot = n;
	} else if ( a == 1 ) {
		plant_term( plant, term );
	} else {
		throw Mishap( "Wrong number of results in single context" ).culprit( "#Results", "" + a );
	}
}

void plant0( Plant plant, Term term ) {
	int a = arity_term( term );
	if ( a == DONTKNOW ) {
		int n = plant->slot;
		int v = tmpvar( plant );
		vmiSTART( plant, v );
		plant_term( plant, term );
		vmiCHECK0( plant, v );
		plant->slot = n;
	} else if ( a == 0 ) {
		plant_term( plant, term );
	} else {
		throw Mishap( "Wrong number of results in zero context" ).culprit( "#Results", "" + a );
	}
}
