#ifndef SCAN_CALL_STACK_HPP
#define SCAN_CALL_STACK_HPP

#include "objlayout.hpp"
#include "callstacklayout.hpp"
#include "machine.hpp"

class ScanCallStack {
private:
	MachineClass * 		vm;
	Ref *				frame_pointer;
	enum ScanState {
		ScanFunc,
		ScanLocals
	} 					state;
	unsigned int 		local;
	
public:
	Ref * next() {
		if ( vm->sp_base <= this->frame_pointer && this->frame_pointer < vm->sp_end ) {
			if ( this->state == ScanFunc ) {
				Ref * func = ToRefRef( this->frame_pointer[ SP_FUNC ] );
				this->state = ScanLocals;
				return func;
			} else if ( this->state == ScanLocals ) {
				unsigned long nslots = ToULong( this->frame_pointer[ SP_NSLOTS ] );
				if  ( this->local < nslots ) {
					return ToRefRef( this->frame_pointer[ this->local++ ] );
				} else {
					this->state = ScanFunc;
					this->local = 0;
					this->frame_pointer = ToRefRef( this->frame_pointer[ SP_PREV_SP ] );
					return this->next();
				}
			} else {
				throw "Unreachable";
			}
		} else {
			return static_cast< Ref * >( 0 );
		}
	}
	
	
public:
	ScanCallStack( MachineClass * m ) : 
		vm( m ),
		frame_pointer( m->sp ),
		state( ScanFunc ),
		local( 0 )
	{
	}
};

#endif