#ifndef MACHINE2_HPP
#define MACHINE2_HPP

#include "machine.hpp"
#include "instruction_set2.hpp"

class Machine2 : public MachineClass {

private:
	InstructionSet2 instruction_set;
	
public:
	virtual void execute( Ref r );
	const InstructionSet & instructionSet() {
		return this->instruction_set;
	}
	
public:
	virtual ~Machine2() {}

};

#endif