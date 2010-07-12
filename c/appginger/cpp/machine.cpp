#include <cstdlib>
#include <iostream>

#include "machine1.hpp"
#include "machine2.hpp"
#include "common.hpp"
#include "machine.hpp"
#include "plant.hpp"
#include "heap.hpp"
#include "key.hpp"
#include "sys.hpp"

using namespace std;

//#define DBG_MACHINE


MachineClass::MachineClass( AppGinger & application ) :
	appg( application ),
	plant_aptr( new PlantClass( this ) ),
	dict_aptr( new DictClass() ),
	heap_aptr( new HeapClass( this ) ),
	program_counter( 0 )
{
	this->sp_base = (Ref *)malloc( sizeof( Ref ) * 1024 );
	this->sp_end = this->sp_base + 1024;
	this->vp_base = (Ref *)malloc( sizeof( Ref ) * 1024 );
	this->vp_end = this->vp_base + 1024;
}

bool MachineClass::getShowCode() {
	return this->appg.getShowCode();
}

void MachineClass::resetMachine() {
	this->plant_aptr.reset( new PlantClass( this ) );
}

PlantClass * MachineClass::plant() {
	return this->plant_aptr.get();
}

DictClass * MachineClass::dict() {
	return this->dict_aptr.get();
}

HeapClass & MachineClass::heap() {
	return *this->heap_aptr.get();
}

Ref * MachineClass::setUpPC( Ref r ) {
	static Ref launch[ 1 ] = { this->instructionSet().lookup( vmc_reset ) };
	
	// pointer to the function object.
	this->func_of_program_counter = RefToPtr4( r );
	// +1 to get a pointer to ENTRY instruction.
	this->program_counter = this->func_of_program_counter + 1;


	this->sp = this->sp_base;
	*this->sp = sys_underflow;
	this->vp = this->vp_base;
	*this->vp = sys_underflow;

	
	//	The previous function object should be set to null.
	*( ++this->sp ) = 0;
	
	//	The previous link address should be set to null too.
	*( ++this->sp ) = 0;
	
	//	And the previous stack point is additionally set to null.
	*( ++this->sp ) = 0;

	//	The number of locals variables - obviously null.
	*( ++this->sp ) = 0;
	
	//	We also need to point one on from the number of local variables.
	this->sp += 1;
	
	//	Now store a fake return address.  This will cause this to halt.
	//	That's a little bit nasty but we'll sort that out later.
	this->link = ToRefRef( &launch );

	this->count = 0;
	
	return this->program_counter;
}

void MachineClass::printfn( ostream & out, Ref x ) {
	Ref * obj_K = RefToPtr4( x );
	Ref * obj_K1 = obj_K + 1;
	long A = SmallToLong( obj_K[ -1 ] );
	long N = ToLong( obj_K[ -2 ] );
	long R = ToLong( obj_K[ -3 ] );
	long C = ToULong( obj_K[ -4 ] ) >> TAGGG;
	Ref * obj_Z1 = obj_K + C + 1;
	out << "define: " << A << " args, " << N << " locals, " << R << " results, " << C << " #words used" << endl;
	{
		Ref *pc = obj_K1;
	    while ( pc < obj_Z1 ) {
		    out << "[" <<  ( pc - obj_K ) << "]\t ";
		   	pc = this->instructionShow( out, pc );
	    }
	}
	out << "enddefine" << endl;
}

void MachineClass::printfn( Ref x ) {
	this->printfn( std::clog, x );
}


Ref * MachineClass::instructionShow( ostream & out, Ref *pc ) {
	const InstructionSet & ins = this->instructionSet();
	const char *types = ins.signature( *pc );
	while ( *types != '\0' ) {
		switch ( *types++ ) {
			case 'i': {
				out << ins.name( *pc ) << " ";
				break;
			}
			case 'r': {
				out << (unsigned long)( *pc ) << " ";
				break;
			}
			case 'c': {
				refPrint( out, *pc );               
				//printf( " " );
				//out <<  "XXX ";
				break;
			}
			case 'v': {
				IdentClass *id = (IdentClass *)RefToPtr4( *pc );
				out << id->getNameString() << " ";
				break;
			}
			default:
				throw "unreachable";
		}
		pc += 1;
	}
	out << endl;
	return pc;
}

Ref * MachineClass::instructionShow( Ref *pc ) {
	return this->instructionShow( std::cout, pc );
}

void MachineClass::print_results( float time_taken ) {
	this->print_results( std::cout, time_taken );
}

void MachineClass::print_results( std::ostream & out, float time_taken ) {
	const bool quiet = this->appg.isBatchMode();


	int n = this->vp - this->vp_base;

	if ( !quiet ) {
		out << "There " << ( n == 1 ? "is" : "are" ) << " " << 
		n << " result" << ( n == 1 ? "" : "s" ) << "\t(" << 
		time_taken << "s)" << endl;
	}
	for ( int i = 0; i < n; i++ ) {
		if ( !quiet ) {
			out << ( i + 1 ) << ".\t";
		}
		refPrint( out, this->vp[ 1 + i - n ] );
		out << endl;
	}
	if ( !quiet ) {
		out << endl;
	}
	this->vp = this->vp_base;
}

