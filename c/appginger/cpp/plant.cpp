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
			Ident & ident = term_named_ident( var );
			
			Term start_expr = q->child( 1 );
			this->compile1( start_expr );
			vmiPOPID( this, ident );
			
			Term end_expr = q->child( 2 );
			q->end_expr_ident = this->newTmpIdent();
			this->compile1( end_expr );
			vmiPOPID( this, q->end_expr_ident );
			
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
			Ident & ident = term_named_ident( var );
			vmiPUSHID( this, ident );
			vmiAPPSPC( this, vmc__incr );
			vmiPOPID( this, ident );
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
			Ident & ident = term_named_ident( var );
			
			VmiRelOpFactory relop_factory( this );
			relop_factory.setLeft( ident );
			relop_factory.setLTE();
			relop_factory.setRight( q->end_expr_ident );
			relop_factory.ifSo( dst );

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
		case fnc_appspc: {
			//	I do not believe this can ever be executed
			this->compileTerm( term_index( term, 1 ) );
			Instruction instr = functor_inline( term_functor( term_index( term, 0 ) ) );
			vmiAPPSPC( this, instr );
			break;
		}
		case fnc_seq: {
			int n = term_arity( term );
			for ( int i = 0; i < n ; i++ ) {
				this->compileTerm( term_index( term, i ) );
			}
			break;
		}
		case fnc_id: {
			Ident id = term_named_ident( term );
			if ( id != NULL ) {
				int slot = id->slot;
				if ( slot >= 0 || !id->is_local ) {
					vmiPUSHID(  this, id );
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
			this->compile1( body );
			vmiPOPID( this, ident );
			break;
		}
		case fnc_assign: {
			Term lhs = term_index( term, 0 );
			if ( term_is_id( lhs ) ) {
				Ident & ident = term_named_ident( lhs );
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
			int aargs = arity_term( args );
			//	plant_count( plant, args );
			if ( term_is_id( fn ) ) {
				if ( aargs == DONTKNOW ) {
					int v = tmpvar( this );
		        	vmiSTART( this, v );
					this->compileTerm( args );
					vmiEND_CALL_ID( this, v, term_named_ident( fn ) );
				} else {
					this->compileTerm( args );
					vmiSET_CALL_ID( this, aargs, term_named_ident( fn ) );
				}
			} else if ( aargs == DONTKNOW ) {
				int v = tmpvar( this );
		        vmiSTART( this, v );
		        this->compileTerm(  args );
				this->compile1( fn );
				vmiEND1_CALLS( this, v );
			} else {
				this->compileTerm( args );
				this->compile1( fn );
				vmiSET_CALLS( this, aargs );
			}
			break;
		}
		case fnc_syscall: {
			Ref sc = term_ref_cont( term );
			vmiSYS_CALL( this, sc );
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
			int a = term_arity( term );
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
		default: {
			mishap( "PLANT_TERM: Not implemented yet, functor '%s'", functor_name( fnc ) );
		}
	}
}


void PlantClass::compile1( Term term ) {
	int a = arity_term( term );
	if ( a == DONTKNOW ) {
		int n = this->slot;
		int v = tmpvar( this );
		vmiSTART( this, v );
		this->compileTerm( term );
		vmiCHECK1( this, v );
		this->slot = n;
	} else if ( a == 1 ) {
		this->compileTerm( term );
	} else {
		throw Mishap( "Wrong number of results in single context" ).culprit( "#Results", "" + a );
	}
}

void PlantClass::compile0( Term term ) {
	int a = arity_term( term );
	if ( a == DONTKNOW ) {
		int n = this->slot;
		int v = tmpvar( this );
		vmiSTART( this, v );
		this->compileTerm( term );
		vmiCHECK0( this, v );
		this->slot = n;
	} else if ( a == 0 ) {
		this->compileTerm( term );
	} else {
		throw Mishap( "Wrong number of results in zero context" ).culprit( "#Results", "" + a );
	}
}