#ifndef MACHINE1_HPP
#define MACHINE1_HPP

#include "machine.hpp"
#include "instruction_set1.hpp"

class Machine1 : public MachineClass {

private:
	InstructionSet1 instruction_set;
	
public:
	virtual void execute( Ref r );
	virtual const InstructionSet & instructionSet() {
		return this->instruction_set;
	}

public:
	virtual ~Machine1() {}

};

#endif