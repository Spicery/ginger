#include "garbagecollect.hpp"

#include <iostream>
using namespace std;

#include "key.hpp"
#include "objlayout.hpp"
#include "scancallstack.hpp"
#include "scandict.hpp"
#include "machine.hpp"
#include "heapcrawl.hpp"
#include "cagecrawl.hpp"
#include "callstackcrawl.hpp"


/******************************************************************************\

The ToSpaceCopier is the core of the Cheney-style stop-and-copy algorithm. It
maintains a buffer of half-copied objects. There is a direct comparision with
the A* algorithm configured to do a breadth first search.

\******************************************************************************/


class ToSpaceCopier {
private:
	MachineClass * 			vm;
	HeapClass &				heap;		
	CageClass *				currentToSpaceCage;
	CageClass *				currentQueueCage;
	vector< CageClass * > 	fromSpace;
	vector< CageClass * > 	toSpace;

public:

	Ref * copyTo( Ref * obj_A, Ref * obj_Z1, CageClass * cage ) {
		XfrClass xfr( cage );
		xfr.setOrigin();
		xfr.xfrCopy( obj_A, obj_Z1 );
		Ref * new_obj = RefToPtr4( xfr.make() );
		return new_obj;
	}
	
	//	Select a new cage that can accomodate an object of size d.
	CageClass * selectCurrentCage( ptrdiff_t d ) {
		for ( 
			vector< CageClass * >::iterator it = this->toSpace.begin();
			it != this->toSpace.end();
			++it
		) {	
			if ( (*it)->checkRoom( d ) ) {
				return *it;
			}
		}
		CageClass * p = this->heap.newCageClass();
		this->toSpace.push_back( p );
		return p;
	}

	Ref * copy( Ref * obj_A, Ref * obj_Z1 ) {
		//	Is there room in the currentToSpace?
		ptrdiff_t d = obj_Z1 - obj_A;
		if ( this->currentToSpaceCage->checkRoom( d ) ) {
			return this->copyTo( obj_A, obj_Z1, this->currentToSpaceCage );
		} else {
			//	Should we rotate out the current to-space? It depends
			//	a bit on whether or not it has effectively run out of
			//	space. For the moment we will keep the decision simple
			//	and assert that it needs rotating out immediately.
			
			//	Select a new cage.
			this->currentToSpaceCage = this->selectCurrentCage( d );
			return this->copyTo( obj_A, obj_Z1, this->currentToSpaceCage );			
		}
	}
	
	
public:

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
	
	Ref * next() {
		if ( this->currentQueueCage->hasEmptyQueue() ) {
			this->currentQueueCage = this->findCageWithNonEmptyQueue();
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
		std::vector< CageClass * > fromSpace;
		
		HeapCrawl crawl( this->heap );
		for (;;) {
			CageClass * cage = crawl.next();
			if ( not cage ) break;
			if ( cage->nboxesInUse() == 0 ) {
				toSpace.push_back( cage );
				cage->resetQueue();
			} else {
				fromSpace.push_back( cage );
			}
		}
		
		//	Pick a toSpace to become the current to-space cage.
		if ( toSpace.empty() ) {
			//	Worried about what would happen in the event of an exception being raised.
			//	This next line is plainly not quite right. We want to make a better
			//	guess as to the size of cage that is required.
			this->currentToSpaceCage = heap.newCageClass();
		} else {
			this->currentToSpaceCage = this->toSpace.back();
			this->toSpace.pop_back();
		}
		
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
	Ref * & 				pc;
	MachineClass * 			vm;
	ToSpaceCopier 			copier;
	ptrdiff_t				pc_delta;

public:	
	void forward( Ref & current ) {
		if( not IsPtr4( current ) ) return;
		
		Ref * obj = RefToPtr4( current );
		if ( *obj == sys_forwarded ) {
			//	Already forwarded.
			//	Pick up the to-space address.
			current = *( obj + 1 );		
		} else {
			Ref * obj_A;
			Ref * obj_Z1;
			findObjectLimits( obj, obj_A, obj_Z1 );
			Ref * half_copied_obj  = copier.copy( obj_A, obj_Z1 );
			current = Ptr4ToRef( half_copied_obj );
			
			//	We stomp on the key with a special NON-KEY value 
			//	we leave the to-space address in the next pointer.
			*obj =  sys_forwarded;
			*( obj + 1 ) = half_copied_obj;
		}
	}

	void forwardValueStack() {
		for ( Ref * p = vm->vp; p != vm->vp_base; p-- ) {
			this->forward( *p );
		}
	}
	
	void freezeNonKeyPointers() {
		//	Program counter.
		Ref * func = ToRefRef( this->vm->sp[ SP_FUNC ] );
		this->pc_delta = this->vm->program_counter - func;
	
		//	Call stack pointers.
		CallStackCrawl csc( this->vm );
		for (;;) {
			Ref * sp = csc.next();
			if ( not sp ) break;
			Ref * func = ToRefRef( sp[ SP_FUNC ] );
			ptrdiff_t d = ToRefRef( sp[ SP_LINK ] ) - func;
			sp[ SP_LINK ] = ToRef( d );
		}
	}
	
	void meltNonKeyPointers() {
		//	Recover program counter.
		Ref * func = ToRefRef( this->vm->sp[ SP_FUNC ] );	//	Adjusted by the GC.
		this->vm->program_counter = func + this->pc_delta;
	
		//	Call stack pointers.
		CallStackCrawl csc( this->vm );
		for (;;) {
			Ref * sp = csc.next();
			if ( not sp ) break;

			Ref * func = ToRefRef( sp[ SP_FUNC ] );
			ptrdiff_t d = (ptrdiff_t)( sp[ SP_LINK ] );
			sp[ SP_LINK ] = func + d;
		}
	}
	
	void forwardCallStackAndPC() {
		this->freezeNonKeyPointers();
		ScanCallStack scanner( this->vm );
		for (;;) {
			Ref * p = scanner.next();
			if ( not p ) break;
			this->forward( *p );
		}
		this->meltNonKeyPointers();
	}
	
	void forwardDictionary() {
		ScanDict scanner( this->vm->dict() );
		for (;;) {
			Ref * p = scanner.next();
			if ( not p ) break;
			this->forward( *p );
		}
	}
	
	void forwardRoots() {
		this->forwardValueStack();
		this->forwardCallStackAndPC();
		this->forwardDictionary();
	}
	
	void forwardContents( Ref * obj ) {
		Ref key = *obj;
		if ( IsFnKey( key ) ) {
			throw "unimplemented";
		} else if ( IsSimpleKey( key ) ) {
			switch ( KindOfSimpleKey( key ) ) {
				case RECORD_KIND: {
					this->forward( obj[ 1 ] );
					this->forward( obj[ 2 ] );
					break;
				}
				case VECTOR_KIND: {
					throw "unimplemented";
					break;
				}
				case STRING_KIND: {
					//	Non-full. Can stop.
					break;
				}				
				default: {
					throw;
				}
			}
		} else if ( IsPtr4( key ) ) {
			//	Compound keys not implemented yet.
			throw "unimplemented";
		} else {
			throw;
		}
	}

	
	void collectGarbage() {
		this->forwardRoots();
		for (;;) {
			Ref * obj = copier.next();
			if ( not obj ) break;
			this->forwardContents( obj );
		}
	}
	
public:
	GarbageCollect( Ref * & pc, MachineClass * virtual_machine ) :
		pc( pc ),
		vm( virtual_machine ),
		copier( virtual_machine )
	{
	}
};

Ref * sysGarbageCollect( Ref * pc, MachineClass * vm ) {
	GarbageCollect gc( pc, vm );
	vm->program_counter = pc;
	gc.collectGarbage();
	return vm->program_counter;
}