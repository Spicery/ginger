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

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <cassert>

#include <stddef.h>


#include "garbagecollect.hpp"
#include "gctracker.hpp"
#include "key.hpp"
#include "misclayout.hpp"
#include "vectorlayout.hpp"
#include "classlayout.hpp"
#include "scanpkg.hpp"
#include "machine.hpp"
#include "heapcrawl.hpp"
#include "cagecrawl.hpp"
#include "callstackcrawl.hpp"
#include "fnobjcrawl.hpp"
#include "package.hpp"
#include "mishap.hpp"
#include "syssymbol.hpp"
#include "functionlayout.hpp"
#include "reflayout.hpp"
#include "sysmap.hpp"

#ifdef CAGE_FINDER_NEEDED
	#include "cagefinder.hpp"
#endif

namespace Ginger {
using namespace std;

//#define DBG_GC

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
	Ginger::GCTracker *		tracker;

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
		this->vm->getPressure().increasePressure();
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

private:
	bool changed_flag;

	void copy1() {
		this->changed_flag = true;
		if ( this->tracker ) this->tracker->copy1();
	}
	
public:
	bool changed() {
		return this->changed_flag;
	}
	
	void resetChanged() {
		this->changed_flag = false;
		if ( this->tracker ) this->tracker->reset();
	}
	
public:
	Ref * copy( Ref * obj ) {
		Ref * obj_A;
		Ref * obj_Z1;
		findObjectLimits( obj, obj_A, obj_Z1 );
		
		this->copy1();

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
	ToSpaceCopier( MachineClass * vm, Ginger::GCTracker * tracker ) :
		vm( vm ),
		heap( vm->heap() ),
		tracker( tracker )
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
			this->vm->getPressure().increasePressure();
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
	Ginger::GCTracker *		tracker;
	MachineClass * 			vm;
	bool 					scan_call_stack;
	ToSpaceCopier 			copier;
	//ptrdiff_t				pc_delta;	//	Not used.
	
	size_t					prev_num_assoc_chains;
	vector< Ref * >			assoc_chains;
	set< Ref * >			weak_roots;
	set< Ref * >			weak_refs;
	set< Ref * >			rehash_needed;

#ifdef CAGE_FINDER_NEEDED
	Ginger::CageFinder		cage_finder;
#endif
public:
	GarbageCollect( MachineClass * virtual_machine, bool scan_call_stack ) :
		tracker( NULL ),
		vm( virtual_machine ),
		copier( virtual_machine, NULL ),
		prev_num_assoc_chains( 0 )
	{
		#ifdef CAGE_FINDER_NEEDED
			// Commented out until we add in different kind of cages.
			HeapClass & heap = virtual_machine->heap();
			for ( HeapClass::cage_iterator it = heap.begin(); it != heap.end(); ++it ) {
				//	Note that we should only add cages to the cage_finder if we need 
				//	to find the cages! So in reality we would only populate with the
				//	cages that have interesting flags.
				cage_finder.add( *it );
			}
		#endif
	}
	
public:
	void setGCTracker( Ginger::GCTracker * t ) {
		this->tracker = t;
	}
	
private:	
	void forward( Ref & current ) {
		if ( this->tracker ) this->tracker->atRef( current );
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

					#ifdef DBG_CAGE_FINDER
						CageClass * c = this->cage_finder.find( obj );
						if ( c != NULL ) {
							cerr << "Checking cage of object: " << obj << endl;
							cerr << "    cage         : " << c->serialNumber() << endl;
							cerr << "    contained?   : " << c->contains( obj ) << endl;
							cerr << "    after start? : " << c->atOrAfterStart( obj ) << endl;
							cerr << "    before end?  : " << c->beforeEnd( obj ) << endl;
						} else {
							cerr << "FAILED TO FIND CONTAINING CAGE!" << obj << endl;
						}
					#endif

					Ref * half_copied_obj  = copier.copy( obj );
					current = Ptr4ToRef( half_copied_obj );
					
					//	We stomp on the key with a forwarded value.
					*obj = Ptr4ToFwd( half_copied_obj );
				}
			}
		}
		if ( this->tracker ) this->tracker->afterAtRef( current );
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
			ScanPkg scanner( p );
			for (;;) {
				Ref * p = scanner.next();
				if ( not p ) break;
				if ( this->tracker ) this->tracker->atVariable( scanner.variable() );
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
		if ( this->tracker ) this->tracker->startValueStack();
		this->forwardValueStack();
		if ( this->tracker ) this->tracker->endValueStack();
		if ( scan_call_stack ) {
			if ( this->tracker ) this->tracker->startCallStackAndPC();
			this->forwardCallStackAndPC();
			if ( this->tracker ) this->tracker->endCallStackAndPC();
		}
		if ( this->tracker ) this->tracker->startDictionary();
		this->forwardAllDictionaries();
		if ( this->tracker ) this->tracker->endDictionary();
		if ( this->tracker ) this->tracker->startRegisters();
		this->forwardRegisters();
		if ( this->tracker ) this->tracker->endRegisters();
	}
	
	static bool isTargetForwarded( Ref r ) {
		return IsPrimitive( r ) || IsFwdObj( r );
	}
	
	static bool isntTargetForwarded( Ref r ) {
		return not( IsPrimitive( r ) || IsFwdObj( r ) );
	}
	
	bool isUnderPressure() {
		return this->vm->getPressure().isUnderPressure();
	}

	bool isEffectivelyWeakRef( Ref key, Ref * obj_K ) {
		return(
			( key == sysWeakRefKey || ( key == sysSoftRefKey && this->isUnderPressure() ) ) &&
			( !this->isTargetForwarded( obj_K[ REF_OFFSET_CONT ] ) )
		);
	}
	
	void advanceRecord( Ref * obj_K ) {
		unsigned long n = sizeAfterKeyOfRecordLayout( obj_K );
		for ( unsigned long i = 1; i <= n; i++ ) {
			this->forward( obj_K[ i ] );
		}
	}

	void clearCacheMap( Ref * obj_K ) {
		obj_K[ MAP_OFFSET_COUNT ] = LongToSmall( 0 );
		Ref * data_K = RefToPtr4( obj_K[ MAP_OFFSET_DATA ] );
		const long N = SmallToLong( data_K[ VECTOR_OFFSET_LENGTH ] );
		for ( long i = 1; i <= N; i++ ) {
			data_K[ i ] = SYS_ABSENT;
		}
	}

	void forwardContents( Ref * obj_K ) {
		if ( this->tracker ) this->tracker->startContents( obj_K );
		Ref key = *obj_K;
		if ( IsFunctionKey( key ) ) {
			if ( IsCoreFunctionKey( key ) ) {
				throw UnreachableError();
			}
			if ( this->tracker ) this->tracker->startFnObj();
			if ( IsMethodKey( key ) ) {
				//	This is a temporary hack that works around not having
				//	weak pointers.
				obj_K[ METHOD_OFFSET_CACHE ] = SYS_ABSENT;
				this->assoc_chains.push_back( &obj_K[ METHOD_OFFSET_DISPATCH_TABLE ] );
			} else {
				FnObjCrawl fnobjcrawl( vm, obj_K );
				for ( ;; ) {
					Ref * p = fnobjcrawl.next();
					if ( p == NULL ) break;
					if ( this->tracker ) this->tracker->startInstruction( fnobjcrawl );
					this->forward( *p );
					if ( this->tracker ) this->tracker->endInstruction( fnobjcrawl );
				}
			}
			if ( this->tracker ) this->tracker->endFnObj();
		} else if ( IsSimpleKey( key ) ) {
			switch ( LayoutOfSimpleKey( key ) ) {
			//switch ( KindOfSimpleKey( key ) ) {
				case RECORD_LAYOUT: {
					assert( LayoutOfSimpleKey( key ) == RECORD_LAYOUT );
					assert( KindOfSimpleKey( key ) == PAIR_KIND || KindOfSimpleKey( key ) == MAP_KIND || KindOfSimpleKey( key ) == RECORD_KIND );
					if ( this->tracker ) this->tracker->startRecord( obj_K );
					if ( IsLikeRefSimpleKey( key ) ) {
						if ( this->isEffectivelyWeakRef( key, obj_K ) ) {
							this->weak_refs.insert( &obj_K[ REF_OFFSET_CONT ] );
						} else {
							this->forward( obj_K[ REF_OFFSET_CONT ] );
						}
					} else if ( IsMapSimpleKey( key ) ) {
						//std::cout << "KEY " << key << "; EQ  " << MapKeyEq( key ) << std::endl;
						if ( MapKeyEq( key ) ) {
							if ( key == sysCacheEqMapKey && this->isUnderPressure() ) {
								//std::cerr << "MapKeyEq( key ) = " << MapKeyEq( key ) << std::endl;
								this->clearCacheMap( obj_K );
							}
						} else {
							this->rehash_needed.insert( obj_K );
						}
						this->advanceRecord( obj_K );
					} else {
						this->advanceRecord( obj_K );
					}
					if ( this->tracker ) this->tracker->endRecord( obj_K );
					break;
				}
				case VECTOR_LAYOUT: {
					assert( LayoutOfSimpleKey( key ) == VECTOR_LAYOUT );
					assert( KindOfSimpleKey( key ) == VECTOR_KIND );
				
					if ( this->tracker ) this->tracker->startVector( obj_K );
					if ( key == sysHashMapDataKey ) {
						//obj_K[ METHOD_OFFSET_CACHE ] = SYS_ABSENT;
						//this->assoc_chains.push_back( &obj_K[ METHOD_OFFSET_DISPATCH_TABLE ] );
						long n = sizeAfterKeyOfVectorLayout( obj_K );
						Ref * obj_K1 = obj_K + 1;
						for ( long i = 0; i < n; i++ ) {
							Ref * chain = &obj_K1[ i ];
							if ( *chain != SYS_ABSENT ) {
								this->assoc_chains.push_back( chain );
							}
						}						
					} else {
						long n = sizeAfterKeyOfVectorLayout( obj_K );
						Ref * obj_K1 = obj_K + 1;
						for ( long i = 0; i < n; i++ ) {
							this->forward( obj_K1[ i ] );
						}
					}
					if ( this->tracker ) this->tracker->endVector( obj_K );
					break;
				}
				case MIXED_LAYOUT: {
					assert( LayoutOfSimpleKey( key ) == MIXED_LAYOUT );
				
					if ( this->tracker ) this->tracker->startVector( obj_K );
					if ( key == sysHashMapDataKey ) {
						//obj_K[ METHOD_OFFSET_CACHE ] = SYS_ABSENT;
						//this->assoc_chains.push_back( &obj_K[ METHOD_OFFSET_DISPATCH_TABLE ] );
						long n = sizeAfterKeyOfMixedLayout( obj_K );
						Ref * obj_K1 = obj_K + 1;
						for ( long i = 0; i < n; i++ ) {
							Ref * chain = &obj_K1[ i ];
							if ( *chain != SYS_ABSENT ) {
								this->assoc_chains.push_back( chain );
							}
						}						
					} else {
						long n = sizeAfterKeyOfMixedLayout( obj_K );
						Ref * obj_K1 = obj_K + 1;
						for ( long i = 0; i < n; i++ ) {
							this->forward( obj_K1[ i ] );
						}
					}
					if ( this->tracker ) this->tracker->endVector( obj_K );
					break;
				}
				//	This should be STRING_LAYOUT
				case STRING_LAYOUT: {
					assert( LayoutOfSimpleKey( key ) == STRING_LAYOUT );
					assert( KindOfSimpleKey( key ) == STRING_KIND );

					if ( this->tracker ) this->tracker->atString( obj_K );
					//	Non-full. Can stop.
					break;
				}
				case WRECORD_LAYOUT: {
					assert( LayoutOfSimpleKey( key ) == WRECORD_LAYOUT );
					if ( this->tracker ) this->tracker->atWRecord( obj_K );
					break;
				}
				default: {
					throw "unimplemented (other)";
				}
			}
		} else if ( IsFwd( key ) || IsObj( key ) ) {
			if ( this->tracker ) this->tracker->startInstance( obj_K );
			unsigned long n = sizeAfterKeyOfInstance( obj_K );
			for ( unsigned long i = 0; i <= n; i++ ) {
				this->forward( obj_K[ i ] );
			}
			if ( this->tracker ) this->tracker->endInstance( obj_K );
		} else {
			/// @todo Loose end in the garbage collector.
			throw Ginger::Mishap( "ToBeDone" );
		}
		if ( this->tracker ) this->tracker->endContents();
	}
	
	//	This is a form of forwarding in which the original ref is
	//	not stamped on.
	void forwardTarget( Ref x ) {
		this->forward( x );
	}

	//	Scan the dispatch tables that were added to collect up the new
	//	weak roots that might be eligible for garbage collection.
	void scanAddedWeakAssocChainToCollectWeakRoots() {
		for ( size_t i = prev_num_assoc_chains; i < assoc_chains.size(); ++i ) {
			Ref chain = *this->assoc_chains[ i ];
			while ( chain != SYS_ABSENT ) {
				Ref * assoc_K = RefToPtr4( chain );
				Ref lhs = assoc_K[ ASSOC_OFFSET_KEY ];
				
				if ( isTargetForwarded( lhs ) ) {
					//	The key has been traced (or is non-pointer).
					//	We want to forward the assoc object.
					this->forwardTarget( assoc_K[ ASSOC_OFFSET_VALUE ] );
					if ( isntTargetForwarded( assoc_K[ ASSOC_OFFSET_VALUE ] ) ) {
						cerr << "Target[1] not forwarded!" << endl;
						throw UnreachableError();
					}
				} else {
					//	This is a candidate for GC.
					this->weak_roots.insert( assoc_K );
				}
				
				chain = assoc_K[ ASSOC_OFFSET_NEXT ];
			}			
		}
		this->prev_num_assoc_chains = this->assoc_chains.size();
	}
	
	//	As a result of forwarding any weak roots, have new weak roots been
	//	eliminated as candidates.
	//
	//	COMMENT - it might make more sense to clear the weak-root table each
	//	time and rebuild it from here.
	//
	void scanWeakRoots() {
		//	Take a copy 'cos it is not too smart to mix updates and iteration.
		vector< Ref * > w( this->weak_roots.begin(), this->weak_roots.end() );
		
		for ( vector< Ref * >::iterator it = w.begin(); it != w.end(); ++it ) {
			Ref * wroot_K = *it;
			Ref lhs = wroot_K[ ASSOC_OFFSET_KEY ];
			if ( isTargetForwarded( lhs ) ) {
				//	The key has been traced (or is non-pointer).
				this->forwardTarget( wroot_K[ ASSOC_OFFSET_VALUE ] );
				if ( isntTargetForwarded( wroot_K[ ASSOC_OFFSET_VALUE ] ) ) {
					cerr << "Target[2] not forwarded!" << endl;
					throw UnreachableError();
				}
				cout << "Nudge" << endl;
				
				this->weak_roots.erase( this->weak_roots.find( wroot_K ) );
			}
		}
	}
	
	//	Now we patch up the assoc chains so that the untraced keys are
	//	snipped out.
	void pruneWeakAssocChains() {
		for ( vector< Ref * >::iterator it = assoc_chains.begin(); it != assoc_chains.end(); ++ it ) {
			Ref * chain = *it;
			while ( *chain != SYS_ABSENT ) {
			
				/*if ( IsObj( *chain ) ) {
					cerr << "Chain: " << keyName( RefToPtr4( *chain ) ) << endl;
				} else {
					cerr << "Chain is simple" << endl;
				}*/
			
				Ref * assoc_K = RefToPtr4( *chain );
				
				const bool traced1 = (this->weak_roots.find( assoc_K ) == this->weak_roots.end() );
				
				Ref k  = assoc_K[ ASSOC_OFFSET_KEY ];
				const bool traced2 = isTargetForwarded( k );
				
				if ( traced1 != traced2 ) {
					//cerr << "Key invariant violated" << endl;
					throw SystemError( "GC weak assoc invariant [1] violated" );
				}
				
				const bool traced3 = isTargetForwarded( assoc_K[ ASSOC_OFFSET_VALUE ] );
				if ( traced1 && not( traced3 ) ) {
					//cerr << "All very wrong" << endl;
					throw SystemError( "GC weak assoc invariant [2] violated" );
				}
				
				if ( not traced1 ) {
					//	The key has not been traced and the assoc item
					//	is eligible for garbage collection.
					*chain = assoc_K[ ASSOC_OFFSET_NEXT ];
					if ( this->tracker ) this->tracker->prune1();
					//cerr << "Prune" << endl;
				} else {
					chain = &assoc_K[ ASSOC_OFFSET_NEXT ];
					if ( this->tracker ) this->tracker->retain1();
					//cerr << "Skip" << endl;
				}
			}			
		}		
	}
	
	void copyWeakAssocChains() {
		for ( vector< Ref * >::iterator it = assoc_chains.begin(); it != assoc_chains.end(); ++ it ) {
			Ref * chain = *it;
			this->forward( *chain );
		}
	}

	void updateWeakRefs() {
		for ( set< Ref * >::iterator it = this->weak_refs.begin(); it != this->weak_refs.end(); ++it ) {
			Ref * ptr = *it;
			if ( this->isTargetForwarded( *ptr ) ) {
				this->forward( *ptr );
			} else {
				*ptr = SYS_ABSENT;
			}
		}
	}
	
	void copyPhase() {
		for (;;) {
			Ref * obj = copier.next();
			if ( not obj ) break;
			if ( this->tracker ) this->tracker->pickedObjectToCopy( obj );
			this->forwardContents( obj );
		}
	}				
	
	void postGCRehash() {
		for ( set< Ref * >::iterator it = this->rehash_needed.begin(); it != this->rehash_needed.end(); ++it ) {
			gngRehashMapPtr( *it );
		}
		this->rehash_needed.clear();
	}
	
public:
	void collectGarbage() {
		if ( this->vm->isGCTrace() ) {
			cerr << "### Garbage collection " << ( this->scan_call_stack ? "" : " (quiescent)" ) << endl;	
		}
		this->vm->check_call_stack_integrity();		//	debug
		if ( this->tracker ) this->tracker->startGarbageCollection();
		preGCSymbolTable( this->vm->isGCTrace() );
		this->rehash_needed.clear();
		this->forwardRoots( scan_call_stack );
		this->copyPhase();
		
		for (;;) {
			this->copier.resetChanged();
			this->scanAddedWeakAssocChainToCollectWeakRoots();
			this->copyPhase();
			this->scanWeakRoots();
			this->copyPhase();
			if ( not( this->copier.changed() ) ) break;
		}
		
		this->pruneWeakAssocChains();
		this->copyWeakAssocChains();
		
		this->updateWeakRefs();
		
		//	Then for debugging we want to check that there is no further copying.
		this->copier.resetChanged();
		this->copyPhase();
		
		postGCSymbolTable( this->vm->isGCTrace() );
		this->postGCRehash();
		this->vm->postGCManageExternalObjects();

		this->vm->getPressure().decreasePressure();
		if ( this->tracker ) this->tracker->endGarbageCollection();
		this->vm->check_call_stack_integrity();		//	debug
	}	
	
};

Ref * sysGarbageCollect( Ref * pc, MachineClass * vm ) {
	GarbageCollect gc( vm, true );
	vm->program_counter = pc;
	gc.collectGarbage();
	return vm->program_counter;
}

void sysQuiescentGarbageCollect( MachineClass *vm, Ginger::GCTracker * stats ) {
	GarbageCollect gc( vm, false );
	gc.setGCTracker( stats );
	gc.collectGarbage();
}

} // namespace Ginger
