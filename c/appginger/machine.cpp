#include "machine1.hpp"
#include "machine2.hpp"
#include "common.hpp"
#include "machine.hpp"
#include "plant.hpp"
#include "heap.hpp"
#include "key.hpp"

#include <cstdio>
#include <cstdlib>

//#define DBG_MACHINE


MachineClass::MachineClass() :
	plant_aptr( new PlantClass( this ) ),
	dict_aptr( new DictClass() ),
	heap_aptr( new HeapClass( this ) )
{
	this->sp_base = (Ref *)malloc( sizeof( Ref ) * 1024 );
	this->sp_end = this->sp_base + 1024;
	this->vp_base = (Ref *)malloc( sizeof( Ref ) * 1024 );
	this->vp_end = this->vp_base + 1024;
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
	Ref *PC = (Ref *)RefToPtr4( r ) + 1; 	// point to ENTRY instruction.
	this->sp = this->sp_base;
	*this->sp = sys_underflow;
	this->vp = this->vp_base;
	*this->vp = sys_underflow;

	//	We need a little bit of stack to get started.  We definitely need
	//	the number of locals variables - obviously 0.
	*( ++this->sp ) = 0;
	//	We also need to point one on from the number of local variables.
	this->sp += 1;
	//	Now store a fake return address.  This will cause this to halt.
	//	That's a little bit nasty but we'll sort that out later.
	////launch[ 0 ] = this->instructionSet().spc_reset;
	this->link = ToRefRef( &launch );

	this->count = 0;
	
	return PC;
}

void MachineClass::printfn( Ref x ) {
	Ref *r = RefToPtr4( x );
	//	Ref K = r[ 0 ];
	long A = SmallToLong( r[ -1 ] );
	long N = ToLong( r[ -2 ] );
	long R = ToLong( r[ -3 ] );
	long L = ToLong( r[ -4 ] );
	printf( "define: %ld args, %ld locals, %ld results\n", A, N, R );
	{
		Ref *pc = r + 1;
	    while ( pc <= r + L ) {
		    printf( "[ %d ]\t ", pc - r );
		   	pc = this->instructionShow( pc );
	    }
	}
	printf( "enddefine\n" );
}

Ref * MachineClass::instructionShow( Ref *pc ) {
	const InstructionSet & ins = this->instructionSet();
	const char *types = ins.signature( *pc );
	while ( *types != '\0' ) {
		switch ( *types++ ) {
			case 'i': {
				printf( "%s ", ins.name( *pc ) );
				break;
			}
			case 'r': {
				printf( "%ld ", (unsigned long)( *pc ) );
				break;
			}
			case 'c': {
				//sys_print( *pc );               
				//printf( " " );
				printf( "XXX " );
				break;
			}
			case 'v': {
				IdentClass *id = (IdentClass *)pc;
				printf( "%s ", id->getNameString().c_str() );
				break;
			}
			default:
				throw "unreachable";
		}
		pc += 1;
	}
	printf( "\n" );
	return pc;
}


