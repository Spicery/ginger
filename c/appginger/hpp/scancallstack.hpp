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
		ScanLink,
		ScanLocals
	} 					state;
	unsigned int 		local;
	
public:
	Ref * next() {
		if ( not this->frame_pointer ) {
			return (Ref *)0;
		} else if ( this->state == ScanLink ) {
			//Ref * func = findFunctionKey( this->frame_pointer );
			//	prolly not implementable.
			throw;
			//this->state = ScanLocals;
			//return func;
		} else if ( this->state == ScanLocals ) {
			unsigned long nslots = ToULong( this->frame_pointer[ SP_NSLOTS ] );
			if  ( this->local < nslots ) {
				return ToRefRef( this->frame_pointer[ this->local++ ] );
			} else {
				this->state = ScanLink;
				this->local = 0;
				this->frame_pointer = ToRefRef( this->frame_pointer[ SP_PREV_SP ] );
				return this->next();
			}
		} else {
			throw "Unreachable";
		}
	}
	
	
public:
	ScanCallStack( MachineClass * m ) : 
		vm( m ),
		frame_pointer( m->sp ),
		state( ScanLink ),
		local( 0 )
	{
	}
};

#endif