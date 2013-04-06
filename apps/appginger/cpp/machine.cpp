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
#include <cassert>

#include <stddef.h>

#include "callstacklayout.hpp"
#include "common.hpp"
#include "machine.hpp"
#include "codegen.hpp"
#include "heap.hpp"
#include "key.hpp"
#include "sys.hpp"
#include "sysprint.hpp"
#include "mishap.hpp"
#include "externalkind.hpp"
#include "functionlayout.hpp"
//#include "vmi.hpp"

using namespace std;

//#define DBG_MACHINE

#define RANDOM_SIZE 100000

MachineClass::MachineClass( AppContext * application ) :
	appg( application ),
	codegen_aptr( new CodeGenClass( this ) ),
	heap_aptr( new HeapClass( this ) ),
	package_mgr_aptr( new PackageManager( this ) ),
	program_counter( 0 ),
	veto_count( 0 )
{
	this->sigint_flag = false;
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
		throw SystemError( "Trying to decrement veto-count less than zero!" );
	}
}

bool MachineClass::gcMoveEnabled() {
	return this->veto_count <= 0;
}

void MachineClass::addToQueue( Ref r ) {
	this->queue.push_back( r );
}

void MachineClass::executeQueue( const bool clear_stack ) {
	if ( this->queue.size() == 1 ) {
		Ref r = this->queue.front();
		this->queue.clear();
		this->gcLiftAllVetoes();
	    this->execute( r, clear_stack );
	} else {		
		CodeGen codegen = this->codegen();
	    codegen->vmiFUNCTION( "Execute Queue", 0, 0 );
	    codegen->vmiENTER();
        for ( 
        	vector< Ref >::iterator it = this->queue.begin();
        	it != this->queue.end();
        	++it
        ) {
        	codegen->vmiPUSHQ( *it );
        	codegen->vmiSET_CALLS( 0 );
        }
        this->queue.clear();
	    codegen->vmiRETURN();
	    Ref r = codegen->vmiENDFUNCTION();
		this->gcLiftAllVetoes();
		this->execute( r, clear_stack );
	}
}

bool MachineClass::getShowCode() {
	return this->appg->getShowCode();
}

bool MachineClass::isGCTrace() {
	return this->appg->isGCTrace();
}

void MachineClass::resetMachine() {
	this->codegen_aptr.reset( new CodeGenClass( this ) );
	this->package_mgr_aptr->reset();
}

CodeGen MachineClass::codegen() {
	return this->codegen_aptr.get();
}


HeapClass & MachineClass::heap() {
	return *this->heap_aptr.get();
}

void MachineClass::clearStack() {
	this->vp = this->vp_base;
	*this->vp = SYS_SYSTEM_ONLY;	
}

void MachineClass::clearCallStack() {
	this->sp = this->sp_base;
	*this->sp = SYS_SYSTEM_ONLY;
	//	And the previous stack point is additionally set to null.
	this->sp[ SP_PREV_SP ] = 0;
	//	The previous link address should be set to null too.
	this->sp[ SP_LINK ] = 0;
	//	The previous function object should be set to null.
	this->sp[ SP_FUNC ] = 0;	
}

Ref * MachineClass::setUpPC( Ref r, const bool clear_stack ) {
	static Ref launch[ 1 ] = { this->instructionSet().lookup( vmc_reset ) };
	
	// pointer to the function object.
	this->func_of_program_counter = RefToPtr4( r );
	// +1 to get a pointer to ENTRY instruction.
	this->program_counter = this->func_of_program_counter + 1;

	if ( clear_stack ) {
		this->clearStack();
	}

	this->clearCallStack();
		
	//	Now store a fake return address.  This will cause this to reset.
	//	i.e. throw. That's a little bit nasty but if needed we will sort
	//	out later.
	this->link = ToRefRef( &launch );
	this->func_of_link = 0x0;

	this->count = 0;
	
	return this->program_counter;
}

/// @todo This is no longer correct. The structure of a function object
/// should be encapsulated anyway.
void MachineClass::printfn( ostream & out, Ref x ) {
	Ref * obj_K = RefToPtr4( x );
	Ref * obj_K1 = obj_K + 1;
	long A = SmallToLong( obj_K[ OFFSET_TO_NUM_INPUTS ] );
	long N = ToLong( obj_K[ OFFSET_TO_NUM_SLOTS ] );
	long R = ToLong( obj_K[ OFFSET_TO_NUM_OUTPUTS ] );
	long C = ToULong( obj_K[ OFFSET_TO_FUNCTION_LENGTH ] ) >> TAGGG;
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
				out << (long)( *pc ) << " ";
				break;
			}
			case 'c': {
				refPrint( out, *pc );               
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
	PrintDetailLevel & pdl = this->appg->printDetailLevel();
	if ( pdl.isntSilent() ) {
		int n = this->vp - this->vp_base;
		if ( pdl.isChatty() ) {

			bool newline_needed = false;

			Ginger::GSON heading( this->getAppContext().userSettings().resultHeading( n, ( n == 0 ? "No results" : "Results..." ) ) );
			if ( heading.isString() ) {
				Ginger::GSONBuilder b;
				b.beginList();
				b.longValue( n );
				b.endList();
				b.newGSON().formatUsing( out, heading.getString() );
				newline_needed = true;
			}

			if ( pdl.isTimed() ) {
				int prev = out.precision();
				out.precision( 2 );
				out << std::fixed;
				out << "\t(" << time_taken << "s)"; 
				out.unsetf( std::ios::floatfield );
				out.precision( prev );
				newline_needed = true;
			}

			if ( newline_needed ) {
				out << endl;
			}
		}

		const std::string bullet( this->getAppContext().userSettings().resultBullet() );

		for ( int i = 0; i < n; i++ ) {
			if ( pdl.isChatty() ) {
				Ginger::GSONBuilder b;
				b.beginList();
				b.longValue( i + 1 );
				b.endList();
				b.newGSON().formatUsing( out, bullet );
				//out << bullet << ( i + 1 ) << ".\t";
			}
			refShow( out, this->vp[ 1 + i - n ] );
			out << endl;
		}
		if ( pdl.isChatty() ) {
			Ginger::GSON footer( this->getAppContext().userSettings().resultFooter( n, ( n == 0 ? "No results" : "Results..." ) ) );
			if ( footer.isString() ) {
				Ginger::GSONBuilder b;
				b.beginList();
				b.longValue( n );
				b.endList();
				b.newGSON().formatUsing( out, footer.getString() );
				out << endl;
			}
		}
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
	
	CodeGen codegen = this->codegen();
	codegen->vmiFUNCTION( 2, 2 );
	codegen->vmiINSTRUCTION( vmc_listiterate );
	iterator = codegen->vmiENDFUNCTION( false );

	return iterator;
}

Ref MachineClass::sysFastVectorIterator() {
	
	//	Memoise.
	static Ref iterator = NULL;
	if ( iterator != NULL ) return iterator;
	
	CodeGen codegen = this->codegen();
	codegen->vmiFUNCTION( 2, 2 );
	codegen->vmiINSTRUCTION( vmc_vectoriterate );
	iterator = codegen->vmiENDFUNCTION( false );

	return iterator;
}

Ref MachineClass::sysFastMixedIterator() {	
	//	Memoise.
	static Ref iterator = NULL;
	if ( iterator != NULL ) return iterator;
	
	CodeGen codegen = this->codegen();
	codegen->vmiFUNCTION( 2, 2 );
	codegen->vmiINSTRUCTION( vmc_mixediterate );
	iterator = codegen->vmiENDFUNCTION( false );

	return iterator;
}

Ref MachineClass::sysFastStringIterator() {
	
	//	Memoise.
	static Ref iterator = NULL;
	if ( iterator != NULL ) return iterator;
	
	CodeGen codegen = this->codegen();
	codegen->vmiFUNCTION( 2, 2 );
	codegen->vmiINSTRUCTION( vmc_stringiterate );
	iterator = codegen->vmiENDFUNCTION( false );

	return iterator;
}

Package * MachineClass::getPackage( std::string title ) {
	return this->package_mgr_aptr->getPackage( title );
}

AppContext & MachineClass::getAppContext() {
	return *this->appg;
}

// -----------------------------------------------------------------------------

//	The following routines are a bit peculiar as they are really intended
//	to be used with gdb.
///	@todo remove!
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

void ExternalTracker::cleanUpAfterGarbageCollection() {
	std::deque< Ginger::External * > for_deletion;
	
	//cerr << "Running the post GC External Object cleanup" << endl;
	//cerr << "    Num external objects before : " << this->external_object_list.size() << endl;
	
	std::list< Ref * >::iterator it = this->external_object_list.begin();
	while (	it != this->external_object_list.end() ) {
		Ref * exobj = *it;
		Ref key = *exobj;
		//cerr << "Key: " << key << ", " << static_cast< Ref >( sysInputStreamKey ) << endl;
		if ( IsFwd( key ) ) {
			Ref * e = FwdToPtr4( key );
			//cerr << "Fwd: " << exobj << " -> " << e << endl;
			assert( IsSimpleKey( *e ) );
			assert( KindOfSimpleKey( *e ) == EXTERNAL_KIND );

			*it++ = e;
		} else {
			for_deletion.push_back( static_cast< Ginger::External * >( exobj[ EXTERNAL_KIND_OFFSET_VALUE ] ) );
			it = this->external_object_list.erase( it );
		}
	}

	//cerr << "    Num external objects after  : " << this->external_object_list.size() << endl;

	while ( not for_deletion.empty() ) {
		delete for_deletion.front();
		for_deletion.pop_front();
	}
}
