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

#include "garbagecollect.hpp"

#include <iostream>
#include <fstream>
using namespace std;

#include "key.hpp"
#include "objlayout.hpp"
#include "scandict.hpp"
#include "machine.hpp"
#include "heapcrawl.hpp"
#include "cagecrawl.hpp"
#include "callstackcrawl.hpp"
#include "fnobjcrawl.hpp"

#define DBG_GC_LOG


/******************************************************************************\

The ToSpaceCopier is the core of the Cheney-style stop-and-copy algorithm. It
maintains a buffer of half-copied objects. There is a direct comparision with
the A* algorithm configured to do a breadth first search.

\******************************************************************************/

//#define DBG_GC

/******************************************************************************\
A callback class for the garbage collector.
\******************************************************************************/

static const bool gc_logging = false;

class GCLogger {
private:
	bool 					scan_call_stack;
	std::ofstream			gclog;

public:
	void atRef( Ref current ) {	
		if ( gc_logging ) {
			gclog << "      Forwarding ref = 0x" << hex << ToULong( current ) << endl;
			gclog.flush();
			gclog << "        Is obj? " << IsObj( current ) << endl;
			gclog.flush();
		}
	}

	void atVariable( std::string var ) {	
		if ( gc_logging ) {
			gclog << "Forwarding " << var << endl;
		}
	}
	
	void startValueStack() {
		if ( gc_logging ) {
			gclog << "Forwarding value stack" << endl;
		}
	}
	
	void endValueStack() {
		if ( gc_logging ) {
		}
	}
	
	void startCallStackAndPC() {
		if ( gc_logging ) {
			gclog << "Forwarding call stack and pc" << endl;
		}
	}
			
	void endCallStackAndPC() {
		if ( gc_logging ) {
		}
	}
	
	void startDictionary() {
		if ( gc_logging ) {
			gclog << "Forward dictionary" << endl;
		}
	}
	
	void endDictionary() {
		if ( gc_logging ) {
		}		
	}
	
	void startGarbageCollection() {
		if ( gc_logging ) {
			gclog << "### " << ( scan_call_stack ? "" : "Quiescent " ) << "GC" << endl;	
		}
	}
	
	void endGarbageCollection() {
		if ( gc_logging ) {
			gclog << "Finished GC (" << ( scan_call_stack ? "" : "Quiescent " ) << ")" << endl;	
		}
	}
	
	void pickedObjectToCopy( Ref * obj ) {
		if ( gc_logging ) {
			gclog << "Picked " << obj << endl;
			gclog.flush();
		}
	}
	
	void startContents( Ref * obj_K ) {
		if ( gc_logging ) {
			gclog << "Forwarding contents for 0x" << std::hex << ToULong( *obj_K ) << endl;
		}
	}

	void endContents() {
		if ( gc_logging ) {
			gclog << "  Finished forwarding contents" << endl;
			gclog.flush();
		}
	}
	
	void startFnObj() {
		if ( gc_logging ) {
			gclog << "  (Function object)" << endl;
		}
	}

	void endFnObj() {
		if ( gc_logging ) {
			gclog << "  (Function object)" << endl;
		}
	}

	void startInstruction( FnObjCrawl & fnobjcrawl ) {
		if ( gc_logging ) {
			gclog << "    At instruction " << fnobjcrawl.getName() << endl;
			gclog << "       position    " << fnobjcrawl.getPosn() << endl;
		}
	}

	void endInstruction( FnObjCrawl & fnobjcrawl ) {
		if ( gc_logging ) {
			gclog << "    Forwarded " << fnobjcrawl.getName() << endl;
		}
	}

	void startVector( Ref * obj_K ) {
		if ( gc_logging ) {
			gclog << "  (Vector)" << endl;
		}
	}
					
	void endVector( Ref * obj_K ) {
		if ( gc_logging ) {
		}		
	}
					
	void startRecord( Ref * obj_K ) {
		if ( gc_logging ) {
			gclog << "  (Record)" << endl;
		}		
	}
					
	void endRecord( Ref * obj_K ) {
		if ( gc_logging ) {
		}		
	}
			
	void atString( Ref * obj_K ) {
		if ( gc_logging ) {
			gclog << "  (String)" << endl;
		}		
	}
	
public:
	GCLogger( const bool scs ) :
		scan_call_stack( scs )
	{
		if ( gc_logging ) {
			gclog.open( "gc.log", ios_base::app | ios_base::out );
		}
	}
	
	~GCLogger() {
		if ( gc_logging ) {
			gclog.close();
		}
	}
		
};


class ToSpaceCopier {
private:
	MachineClass * 			vm;
	HeapClass &				heap;		
	CageClass *				currentToSpaceCage;
	CageClass *				currentQueueCage;
	vector< CageClass * > 	fromSpace;
	vector< CageClass * > 	toSpace;

private:
	
	//	Select a new cage that can accomodate an object of size d.
	CageClass * selectCurrentToSpaceCage( ptrdiff_t d ) {
		for ( 
			vector< CageClass * >::iterator it = this->toSpace.begin();
			it != this->toSpace.end();
			++it
		) {	
			if ( (*it)->checkRoom( d ) ) {
				return *it;
			}
		}
		//	Wrong! Has to be at least d in size!!
		#ifdef DBG_GC
			std::cout << "Could not find a cage big enough to hold " << d << " words." << std::endl;
		#endif
		CageClass * p = this->heap.newCageClass();
		this->toSpace.push_back( p );
		return p;
	}

	Ref * copyTo( Ref * obj_K, Ref * obj_A, Ref * obj_Z1, CageClass * cage ) {
		XfrClass xfr( cage );
		xfr.setOrigin();
		xfr.xfrCopy( obj_A, obj_Z1 );
		Ref * new_obj_A = xfr.makeRefRef();
		Ref * new_obj = new_obj_A + ( obj_K - obj_A );
		return new_obj;
	}

public:
	Ref * copy( Ref * obj ) {
		Ref * obj_A;
		Ref * obj_Z1;
		findObjectLimits( obj, obj_A, obj_Z1 );

		//	Is there room in the currentToSpace?
		ptrdiff_t d = obj_Z1 - obj_A;
		if ( this->currentToSpaceCage->checkRoom( d ) ) {
			return this->copyTo( obj, obj_A, obj_Z1, this->currentToSpaceCage );
		} else {
			//	Should we rotate out the current to-space? It depends
			//	a bit on whether or not it has effectively run out of
			//	space. For the moment we will keep the decision simple
			//	and assert that it needs rotating out immediately.
			
			//	Select a new cage.
			this->currentToSpaceCage = this->selectCurrentToSpaceCage( d );
			return this->copyTo( obj, obj_A, obj_Z1, this->currentToSpaceCage );			
		}
	}
	
	
private:
	CageClass * findCageWithNonEmptyQueue() {
		for ( 
			vector< CageClass * >::iterator it = this->toSpace.begin();
			it != this->toSpace.end();
			++it
		) {	
			if ( (*it)->hasntEmptyQueue() ) {
				this->currentQueueCage = *it;
				return *it;
			}
		}
		return static_cast< CageClass * >( 0 );
	}
	
public:
	Ref * next() {
		if ( this->currentQueueCage->hasEmptyQueue() ) {
			CageClass * c = this->findCageWithNonEmptyQueue();
			if ( not c ) return NULL;
			this->currentQueueCage = c;
		}
		CageCrawl crawl( this->currentQueueCage, this->currentQueueCage->queue_base );
		Ref * obj = crawl.next();
		this->currentQueueCage->queue_base = crawl.currentObjA();
		return obj;
	}
	
public:
	ToSpaceCopier( MachineClass * vm ) :
		vm( vm ),
		heap( vm->heap() )
	{
		//	Prepare heap for start of GC.

		//	Split cages into FromSpace and ToSpace		
		HeapCrawl crawl( this->heap );
		#ifdef DBG_GC
			std::cout << "Checking for empty cages ... " << std::endl;
		#endif
		for (;;) {
			CageClass * cage = crawl.next();
			if ( not cage ) break;
			if ( cage->nboxesInUse() == 0 ) {
				#ifdef DBG_GC
					std::cout << "Cage[" << cage->serialNumber() << "] is free, adding to toSpace." << std::endl;
				#endif
				this->toSpace.push_back( cage );
				cage->resetQueue();
			} else {
				#ifdef DBG_GC
					std::cout << "Cage[" << cage->serialNumber() << "] is in use " << cage->nboxesInUse() << "/" << cage->capacity() << ", adding to fromSpace." << std::endl;
				#endif
				this->fromSpace.push_back( cage );
			}
		}
		
		
		//	Pick a toSpace to become the current to-space cage.
		if ( toSpace.empty() ) {
			//	Worried about what would happen in the event of an exception being raised.
			//	This next line is plainly not quite right. We want to make a better
			//	guess as to the size of cage that is required.
			#ifdef DBG_GC
				std::cout << "No available toSpaces" << std::endl;
			#endif
			this->currentToSpaceCage = heap.newCageClass();
		} else {
			this->currentToSpaceCage = this->toSpace.back();
		}
		
		#ifdef DBG_GC
			std::cout << "#Cages in fromSpace : " << this->fromSpace.size() << std::endl;
			std::cout << "#Cages in toSpace   : " << this->toSpace.size() << std::endl;
		#endif 
		this->currentQueueCage = this->currentToSpaceCage;
	}
	
	~ToSpaceCopier() {
		//	Reset all the fromSpaces to zero.
		for ( 
			vector< CageClass * >::iterator it = this->fromSpace.begin();
			it != this->fromSpace.end();
			++it
		) {	
			(*it)->reset();
		}
		
		//	Set current cage.
		heap.selectCurrent();
	}
	
};




/******************************************************************************\

The Garbage Collector implements a stop-and-copy algorithm.

\******************************************************************************/



class GarbageCollect {
private:
	GCLogger				gclogger;

private:
	MachineClass * 			vm;
	bool 					scan_call_stack;
	ToSpaceCopier 			copier;
	ptrdiff_t				pc_delta;
	
public:	
	void forward( Ref & current ) {
		gclogger.atRef( current );
		if( not IsObj( current ) ) return;
		Ref * obj = RefToPtr4( current );
		if ( IsFwd( *obj ) ) {
			//	Already forwarded.
			//	Pick up the to-space address.
			current = Ptr4ToRef( FwdToPtr4( *obj ) );
		} else {
			Ref * half_copied_obj  = copier.copy( obj );
			current = Ptr4ToRef( half_copied_obj );
			
			//	We stomp on the key with a forwarded value.
			*obj = Ptr4ToFwd( half_copied_obj );
		}
	}

	void forwardValueStack() {
		for ( Ref * p = vm->vp; p != vm->vp_base; p-- ) {
			this->forward( *p );
		}
	}
	
	Ref * forwardPtr4( Ref * ptr ) {
		Ref r = Ptr4ToObj( ptr );
		this->forward( r );
		return ObjToPtr4( r );
	}
	
	
	void forwardCallStackAndPC() {
		this->vm->func_of_program_counter = this->forwardPtr4( this->vm->func_of_program_counter );
		
		CallStackCrawl csc( this->vm );
		for (;;) {
			Ref * sp = csc.next();
			if ( not sp ) break;
			if ( sp[ SP_FUNC ] == 0 ) break;
			
			Ref * func = ToRefRef( sp[ SP_FUNC ] );
			ptrdiff_t d = ToRefRef( sp[ SP_LINK ] ) - func;
			
			Ref * new_func = forwardPtr4( func );
			sp[ SP_FUNC ] = new_func;
			sp[ SP_LINK ] = new_func + d;
			
			unsigned long n = ToULong( sp[ SP_NSLOTS ] );
			for ( unsigned long i = 0; i < n; i++ ) {
				this->forward( LOCAL_OF( sp, i ) );
			}			
		}
	}
	
	void forwardDictionary() {
		ScanDict scanner( this->vm->dict() );
		for (;;) {
			Ref * p = scanner.next();
			if ( not p ) break;
			gclogger.atVariable( scanner.variable() );
			this->forward( *p );
		}
	}
	
	void forwardRoots( const bool scan_call_stack ) {
		gclogger.startValueStack();
		this->forwardValueStack();
		gclogger.endValueStack();
		if ( scan_call_stack ) {
			gclogger.startCallStackAndPC();
			this->forwardCallStackAndPC();
			gclogger.endCallStackAndPC();
		}
		gclogger.startDictionary();
		this->forwardDictionary();
		gclogger.endDictionary();
	}
	
	void forwardContents( Ref * obj_K ) {
		gclogger.startContents( obj_K );
		Ref key = *obj_K;
		if ( IsFnKey( key ) ) {
			gclogger.startFnObj();
			FnObjCrawl fnobjcrawl( vm, obj_K );
			for ( ;; ) {
				Ref * p = fnobjcrawl.next();
				if ( p == NULL ) break;
				gclogger.startInstruction( fnobjcrawl );
				this->forward( *p );
				gclogger.endInstruction( fnobjcrawl );
			}
			gclogger.endFnObj();
		} else if ( IsSimpleKey( key ) ) {
			switch ( KindOfSimpleKey( key ) ) {
				case RECORD_KIND: {
					gclogger.startRecord( obj_K );
					this->forward( obj_K[ 1 ] );
					this->forward( obj_K[ 2 ] );
					gclogger.endRecord( obj_K );
					break;
				}
				case VECTOR_KIND: {
					gclogger.startVector( obj_K );
					long n = sizeAfterKeyOfVector( obj_K );
					Ref * obj_K1 = obj_K + 1;
					for ( long i = 0; i < n; i++ ) {
						this->forward( obj_K1[ i ] );
					}
					gclogger.endVector( obj_K );
					break;
				}
				case STRING_KIND: {
					gclogger.atString( obj_K );
					//	Non-full. Can stop.
					break;
				}				
				default: {
					throw "unimplemented (other)";
				}
			}
		} else if ( IsObj( key ) ) {
			//	Compound keys not implemented yet.
			throw "unimplemented (compound keys)";
		} else {
			throw "unimplemented";
		}
		gclogger.endContents();
	}

	
	void collectGarbage() {
		cerr << "### Garbage collection " << ( this->scan_call_stack ? "" : " (quiescent)" ) << endl;
		this->vm->check_call_stack_integrity();		//	debug
		gclogger.startGarbageCollection();
		this->forwardRoots( scan_call_stack );
		for (;;) {
			Ref * obj = copier.next();
			gclogger.pickedObjectToCopy( obj );
			if ( not obj ) break;
			this->forwardContents( obj );
		}
		gclogger.endGarbageCollection();
		this->vm->check_call_stack_integrity();		//	debug
	}
	
public:
	GarbageCollect( MachineClass * virtual_machine, bool scan_call_stack ) :
		gclogger( scan_call_stack ),
		vm( virtual_machine ),
		copier( virtual_machine )
	{
	}
	
};

Ref * sysGarbageCollect( Ref * pc, MachineClass * vm ) {
	GarbageCollect gc( vm, true );
	vm->program_counter = pc;
	gc.collectGarbage();
	return vm->program_counter;
}

void sysQuiescentGarbageCollect( MachineClass *vm ) {
	GarbageCollect gc( vm, false );
	gc.collectGarbage();
}