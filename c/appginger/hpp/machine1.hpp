#ifndef MACHINE1_HPP
#define MACHINE1_HPP

#include "machine.hpp"
#include "callstacklayout.hpp"
#include "instruction_set1.hpp"
#include "appginger.hpp"

class Machine1 : public MachineClass {

private:
	InstructionSet1 instruction_set;
	
public:
	virtual void execute( Ref r );
	virtual const InstructionSet & instructionSet() {
		return this->instruction_set;
	}

public:
	Machine1( AppGinger & g );
	virtual ~Machine1() {}

};

#endif