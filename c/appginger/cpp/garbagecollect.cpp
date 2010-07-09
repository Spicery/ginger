#include "garbagecollect.hpp"

#include <iostream>
using namespace std;

#include "key.hpp"
#include "objlayout.hpp"
#include "scancallstack.hpp"
#include "scandict.hpp"



/******************************************************************************\

The ToSpaceCopier is the core of the Cheney-style stop-and-copy algorithm. It
maintains a buffer of half-copied objects. There is a direct comparision with
the A* algorithm configured to do a breadth first search.

\******************************************************************************/


class ToSpaceCopier {
public:
	Ref * copy( Ref * obj_A, Ref * obj_Z1 ) {
		return 0;
	}
	
	Ref * next() {
		return 0;
	}
	
};

/******************************************************************************\

The Garbage Collector implements a stop-and-copy algorithm.

\******************************************************************************/

class GarbageCollect {
private:
	MachineClass * vm;
	ToSpaceCopier copier;

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
	
	void forwardCallStack() {
		ScanCallStack scanner( this->vm );
		for (;;) {
			Ref * p = scanner.next();
			if ( not p ) break;
			this->forward( *p );
		}
	}
	
	void forwardDictionary() {
		ScanDict scanner( this->vm->dict() );
	}
	
	void forwardRoots() {
		this->forwardValueStack();
		this->forwardCallStack();
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
	GarbageCollect( MachineClass * virtual_machine ) :
		vm( virtual_machine )
	{
	}
};

void sysGarbageCollect( MachineClass * vm ) {
	GarbageCollect gc( vm );
	gc.collectGarbage();
}

