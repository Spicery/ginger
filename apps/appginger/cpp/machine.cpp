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
#include "sysprint.hpp"
#include "mishap.hpp"
#include "vmi.hpp"

using namespace std;

//#define DBG_MACHINE

#define RANDOM_SIZE 100000

MachineClass::MachineClass( AppContext & application ) :
	appg( application ),
	plant_aptr( new PlantClass( this ) ),
	heap_aptr( new HeapClass( this ) ),
	package_mgr_aptr( new PackageManager( this ) ),
	program_counter( 0 ),
	veto_count( 0 )
{
	this->vp_base = new Ref[ RANDOM_SIZE ];
	this->vp_end = this->vp_base + RANDOM_SIZE;
	this->vp = this->vp_base;
	this->sp_base = this->vp_end - 1;
	this->sp_end = this->vp_base - 1;
	this->sp = this->sp_base;
}

MachineClass::~MachineClass() {
	delete[] this->vp_base;
}

void MachineClass::gcLiftAllVetoes() {
	this->veto_count = 0;
}

void MachineClass::gcVeto() {
	this->veto_count += 1;
}

void MachineClass::gcLiftVeto() {
	if ( this->veto_count > 0 ) {
		this->veto_count -= 1;
	} else {
		throw Ginger::SystemError( "Trying to decrement veto-count less than zero!" );
	}
}

bool MachineClass::gcMoveEnabled() {
	return this->veto_count <= 0;
}

void MachineClass::addToQueue( Ref r ) {
	this->queue.push_back( r );
}

void MachineClass::executeQueue() {
	if ( this->queue.size() == 1 ) {
		Ref r = this->queue.front();
		this->queue.clear();
		this->gcLiftAllVetoes();
	    this->execute( r );
	} else {		
		Plant plant = this->plant();
	    vmiFUNCTION( plant, 0, 0 );
	    vmiENTER( plant );
        for ( 
        	vector< Ref >::iterator it = this->queue.begin();
        	it != this->queue.end();
        	++it
        ) {
        	vmiPUSHQ( plant, *it );
        	vmiSET_CALLS( plant, 0 );
        }
        this->queue.clear();
	    vmiRETURN( plant );
	    Ref r = vmiENDFUNCTION( plant );
		this->gcLiftAllVetoes();
		this->execute( r );
	}
}

bool MachineClass::getShowCode() {
	return this->appg.getShowCode();
}

bool MachineClass::isGCTrace() {
	return this->appg.isGCTrace();
}

void MachineClass::resetMachine() {
	this->plant_aptr.reset( new PlantClass( this ) );
	this->package_mgr_aptr->reset();
}

PlantClass * MachineClass::plant() {
	return this->plant_aptr.get();
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
	this->vp = this->vp_base;
	*this->vp = SYS_SYSTEM_ONLY;

	*this->sp = SYS_SYSTEM_ONLY;
	//	And the previous stack point is additionally set to null.
	this->sp[ SP_PREV_SP ] = 0;
	//	The previous link address should be set to null too.
	this->sp[ SP_LINK ] = 0;
	//	The previous function object should be set to null.
	this->sp[ SP_FUNC ] = 0;
		
	//	Now store a fake return address.  This will cause this to halt.
	//	That's a little bit nasty but we'll sort that out later.
	this->link = ToRefRef( &launch );
	this->func_of_link = 0x0;

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
				Valof * valof = (Valof *)( *pc );
				out << "valof@" << valof;
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

void MachineClass::printResults( float time_taken ) {
	this->printResults( std::cout, time_taken );
}

void MachineClass::printResults( std::ostream & out, float time_taken ) {
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

//	Should be bigger than ( SP_OVERHEAD + 1 ) + 1 = 5
#define STACK_BUFFER_OVERHEAD 16

//	Needs to be inlined after we complete the callstack refactoring.
void MachineClass::checkStackRoom( long n ) {
	if ( this->sp < this->vp + n + STACK_BUFFER_OVERHEAD ) throw Ginger::Mishap( "Stack overflow" );
}

Ref MachineClass::sysFastListIterator() {
	
	//	Memoise.
	static Ref iterator = NULL;
	if ( iterator != NULL ) return iterator;
	
	PlantClass * plant = this->plant();
	vmiFUNCTION( plant, 2, 2 );
	vmiINSTRUCTION( plant, vmc_listiterate );
	iterator = vmiENDFUNCTION( plant, false );

	return iterator;
}

Ref MachineClass::sysFastVectorIterator() {
	
	//	Memoise.
	static Ref iterator = NULL;
	if ( iterator != NULL ) return iterator;
	
	PlantClass * plant = this->plant();
	vmiFUNCTION( plant, 2, 2 );
	vmiINSTRUCTION( plant, vmc_vectoriterate );
	iterator = vmiENDFUNCTION( plant, false );

	return iterator;
}

Ref MachineClass::sysFastStringIterator() {
	
	//	Memoise.
	static Ref iterator = NULL;
	if ( iterator != NULL ) return iterator;
	
	PlantClass * plant = this->plant();
	vmiFUNCTION( plant, 2, 2 );
	vmiINSTRUCTION( plant, vmc_stringiterate );
	iterator = vmiENDFUNCTION( plant, false );

	return iterator;
}

Package * MachineClass::getPackage( std::string title ) {
	return this->package_mgr_aptr->getPackage( title );
}

AppContext & MachineClass::getAppContext() {
	return this->appg;
}

// -----------------------------------------------------------------------------

//	The following routines are a bit peculiar as they are really intended
//	to be used with gdb.
static void bite_me() {
	throw;
}


void MachineClass::check_call_stack_integrity() {
	Ref * ptr = this->sp;
	for ( int count = 0; ptr > this->sp_base; count++ ) {
		Ref * func = ToRefRef( ptr[SP_FUNC] );
		Ref * link = ToRefRef( ptr[SP_LINK] );
		Ref * prev = ToRefRef( ptr[SP_PREV_SP] );
		unsigned long nslots = NSLOTS( ptr );
		
		if ( func ) {
			ptrdiff_t d = link - func;
			if (!( 0 <= d && d <= 1000 )) bite_me();
		}
		
		if ( nslots > 100 ) bite_me();
		ptr = prev;		
	}
}


Pressure & MachineClass::getPressure() { 
	return this->pressure; 
}
