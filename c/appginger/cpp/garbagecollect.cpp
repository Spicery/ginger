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
#include <vector>
#include <set>
using namespace std;

#include "key.hpp"
#include "misclayout.hpp"
#include "classlayout.hpp"
#include "scandict.hpp"
#include "machine.hpp"
#include "heapcrawl.hpp"
#include "cagecrawl.hpp"
#include "callstackcrawl.hpp"
#include "fnobjcrawl.hpp"
#include "package.hpp"
#include "mishap.hpp"
#include "syssymbol.hpp"
#include "functionlayout.hpp"



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
	
	void afterAtRef( Ref current ) {
		if ( gc_logging ) {
			gclog << "        Forwarded to ref = 0x" << hex << ToULong( current ) << endl;
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
	
	void startRegisters() {
		if ( gc_logging ) {
			gclog << "Forward registers" << endl;
		}
	}
	
	void endRegisters() {
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
			gclog << "  Advance contents for " << keyName( *obj_K ) << endl;
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
			
	void startInstance( Ref * obj_K ) {
		if ( gc_logging ) {
			gclog << "  (Instance)" << endl;
		}		
	}
					
	void endInstance( Ref * obj_K ) {
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
	int						copier_changed;

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
	int changed() {
		return this->copier_changed;
	}
	
	void resetChanged() {
		this->copier_changed = 0;
	}

	Ref * copy( Ref * obj ) {
		Ref * obj_A;
		Ref * obj_Z1;
		findObjectLimits( obj, obj_A, obj_Z1 );
		
		this->copier_changed += 1;

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
		
		Ref * obj_K = findObjectKey( this->currentQueueCage->queue_base );
		unsigned long len = lengthAfterObjectKey( obj_K );
		this->currentQueueCage->queue_base = obj_K + len + 1;
				
		return obj_K;
	}
	
public:
	ToSpaceCopier( MachineClass * vm ) :
		vm( vm ),
		heap( vm->heap() ),
		copier_changed( 0 )
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
	
	size_t					prev_num_assoc_chains;
	vector< Ref * >			assoc_chains;
	set< Ref * >			weak_roots;
	
public:
	GarbageCollect( MachineClass * virtual_machine, bool scan_call_stack ) :
		gclogger( scan_call_stack ),
		vm( virtual_machine ),
		copier( virtual_machine ),
		prev_num_assoc_chains( 0 )
	{
	}
	
private:	
	void forward( Ref & current ) {
		gclogger.atRef( current );
		if ( not IsObj( current ) ) {
			if ( IsSymbol( current ) ) {
				gcTouchSymbol( current );
			}
		} else {
			Ref * obj = RefToPtr4( current );
			if ( not IsCoreFunctionKey( *obj ) ) {
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
		}
		gclogger.afterAtRef( current );
	}

	void forwardValueStack() {
		for ( Ref * p = vm->vp; p != vm->vp_base; p-- ) {
			this->forward( *p );
		}
	}
	
	Ref * forwardPtr4( Ref * ptr ) {
		Ref r = Ptr4ToRef( ptr );
		this->forward( r );
		return RefToPtr4( r );
	}
	
	
	void forwardCallStackAndPC() {
	
		//	The PC might be set to a fake value.
		if ( this->vm->func_of_program_counter != NULL ) {
			this->vm->func_of_program_counter = this->forwardPtr4( this->vm->func_of_program_counter );
		}
		
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
			
			unsigned long n = NSLOTS( sp );
			for ( unsigned long i = 0; i < n; i++ ) {
				this->forward( LOCAL_OF( sp, i ) );
			}			
		}
	}
	

	void forwardAllDictionaries() {
		PackageManager * pmgr = this->vm->package_mgr_aptr.get();
		for ( map< string, Package * >::iterator it = pmgr->packages.begin(); it != pmgr->packages.end(); ++it ) {
			Package * p = it->second;
			DictClass & d = p->dict;
			ScanDict scanner( &d );
			for (;;) {
				Ref * p = scanner.next();
				if ( not p ) break;
				gclogger.atVariable( scanner.variable() );
				this->forward( *p );
			}
		}
	}
	
	void forwardRegisters() {
		this->vm->registers.clearUnusedRegisters();
		for ( int i = 0; i < Registers::NREG; i++ ) { 
			this->forward( this->vm->registers.get( i ) );
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
		this->forwardAllDictionaries();
		gclogger.endDictionary();
		gclogger.startRegisters();
		this->forwardRegisters();
		gclogger.endRegisters();
	}
	
	void forwardContents( Ref * obj_K ) {
		gclogger.startContents( obj_K );
		Ref key = *obj_K;
		if ( IsFunctionKey( key ) ) {
			if ( IsCoreFunctionKey( key ) ) {
				throw Unreachable();
			}
			gclogger.startFnObj();
			if ( IsMethodKey( key ) ) {
				//	This is a temporary hack that works around not having
				//	weak pointers.
				obj_K[ METHOD_OFFSET_CACHE ] = sys_absent;
				this->assoc_chains.push_back( &obj_K[ METHOD_OFFSET_DISPATCH_TABLE ] );
			} else {
				FnObjCrawl fnobjcrawl( vm, obj_K );
				for ( ;; ) {
					Ref * p = fnobjcrawl.next();
					if ( p == NULL ) break;
					gclogger.startInstruction( fnobjcrawl );
					this->forward( *p );
					gclogger.endInstruction( fnobjcrawl );
				}
			}
			gclogger.endFnObj();
		} else if ( IsSimpleKey( key ) ) {
			switch ( KindOfSimpleKey( key ) ) {
				case PAIR_KIND:
				case MAP_KIND:
				case RECORD_KIND: {
					gclogger.startRecord( obj_K );
					unsigned long n = sizeAfterKeyOfRecord( obj_K );
					for ( unsigned long i = 1; i <= n; i++ ) {
						this->forward( obj_K[ i ] );
					}
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
		} else if ( IsFwd( key ) || IsObj( key ) ) {
			gclogger.startInstance( obj_K );
			unsigned long n = sizeAfterKeyOfInstance( obj_K );
			for ( unsigned long i = 0; i <= n; i++ ) {
				this->forward( obj_K[ i ] );
			}
			gclogger.endInstance( obj_K );
		} else {
			throw ToBeDone();
		}
		gclogger.endContents();
	}

	//	Scan the dispatch tables that were added to collect up the new
	//	weak roots that might be eligible for garbage collection.
	void scanAddedWeakAssocChainToCollectWeakRoots() {
		for ( size_t i = prev_num_assoc_chains; i < assoc_chains.size(); ++i ) {
			Ref chain = *this->assoc_chains[ i ];
			while ( chain != sys_absent ) {
				Ref * assoc_K = RefToPtr4( chain );
				Ref key = assoc_K[ ASSOC_KEY_OFFSET ];
				Ref & val = assoc_K[ ASSOC_VALUE_OFFSET ];
				
				if ( IsPrimitive( key ) || IsFwdObj( key ) ) {
					//	The key has been traced (or is non-pointer).
					this->forward( val );
				} else {
					//	This is a candidate for GC.
					this->weak_roots.insert( assoc_K );
				}
				
				chain = assoc_K[ ASSOC_NEXT_OFFSET ];
			}			
		}
		this->prev_num_assoc_chains = this->assoc_chains.size();
	}
	
	//	As a result of forwarding any weak roots, have new weak roots been
	//	eliminated as candidates.
	void scanWeakRoots() {
		//	Take a copy 'cos it is not too smart to mix updates and iteration.
		vector< Ref * > w( this->weak_roots.begin(), this->weak_roots.end() );
		
		for ( vector< Ref * >::iterator it = w.begin(); it != w.end(); ++it ) {
			Ref * wroot_K = *it;
			Ref key = wroot_K[ ASSOC_KEY_OFFSET ];
			Ref & val = wroot_K[ ASSOC_VALUE_OFFSET ];
			if ( IsPrimitive( key ) || IsFwdObj( key ) ) {
				//	The key has been traced (or is non-pointer).
				this->forward( val );
			}
		}
	}
	
	//	Now we patch up the assoc chains so that the untraced keys are
	//	snipped out.
	void pruneWeakAssocChains() {
		for ( vector< Ref * >::iterator it = assoc_chains.begin(); it != assoc_chains.end(); ++ it ) {
			Ref * chain = *it;
			while ( *chain != sys_absent ) {
				Ref * assoc_K = RefToPtr4( *chain );

				//	The key has not been traced and the assoc item
				//	is eligible for garbage collection.
				if ( this->weak_roots.find( assoc_K ) != this->weak_roots.end() ) {
					*chain = *RefToPtr4( assoc_K[ ASSOC_NEXT_OFFSET ] );
				} else {
					chain = &assoc_K[ ASSOC_NEXT_OFFSET ];
				}
			}			
		}		
	}
	
	void copyWeakAssocChains() {
		for ( vector< Ref * >::iterator it = assoc_chains.begin(); it != assoc_chains.end(); ++ it ) {
			Ref * chain = *it;
			this->forward( *chain );
		}
		
		//	Then for debugging we want to check that the forwarded 
		this->copier.resetChanged();
		this->copyPhase();
		cerr << "Copied " << this->copier.changed() << " records" << endl;
	}

	void copyPhase() {
		for (;;) {
			Ref * obj = copier.next();
			gclogger.pickedObjectToCopy( obj );
			if ( not obj ) break;
			this->forwardContents( obj );
		}
	}				
	
public:
	void collectGarbage() {
		if ( this->vm->isGCTrace() ) {
			cerr << "### Garbage collection " << ( this->scan_call_stack ? "" : " (quiescent)" ) << endl;	
		}
		this->vm->check_call_stack_integrity();		//	debug
		gclogger.startGarbageCollection();
		preGCSymbolTable( this->vm->isGCTrace() );
		this->forwardRoots( scan_call_stack );
		this->copyPhase();
		
		for (;;) {
			this->copier.resetChanged();
			this->scanAddedWeakAssocChainToCollectWeakRoots();
			this->scanWeakRoots();
			if ( not( this->copier.changed() > 0 ) ) break;
			this->copyPhase();
		}
		
		this->pruneWeakAssocChains();
		this->copyWeakAssocChains();

		postGCSymbolTable( this->vm->isGCTrace() );
		gclogger.endGarbageCollection();
		this->vm->check_call_stack_integrity();		//	debug
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
