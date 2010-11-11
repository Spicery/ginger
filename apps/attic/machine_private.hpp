#ifndef MACHINE_PRIVATE_H
#define MACHINE_PRIVATE_H

#include <memory>

#include "dict.hpp"
#include "plant.hpp"
#include "instruction_set.hpp"

class MachineClass {
public:
	Plant		plant;
	std::auto_ptr<DictClass>		dict;
	Heap		heap;
	int			count;			//	Args count
	Ref			*link;			//	Return address
	Ref			*pc;
	Ref			*sp;
	Ref			*sp_base;
	Ref			*sp_end;
	Ref			*vp;
	Ref			*vp_base;
	Ref			*vp_end;
	
public:
	virtual Ref * setUpPC( Machine VM, Ref r );
	virtual void execute( Machine VM, Ref r ) = 0;
	virtual const InstructionSet & instructionSet() = 0;
	
	virtual void init() {
		//	Do nothing.
	}

};

#endif
