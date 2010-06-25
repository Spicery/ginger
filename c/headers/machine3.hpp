#ifndef MACHINE3_HPP
#define MACHINE3_HPP

#include "machine.hpp"
#include "instruction_set3.hpp"

class Machine3 : public MachineClass {

private:
	InstructionSet3 instruction_set;
	
private:
	void core( bool init_mode, Ref * pc );
	
public:
	virtual void execute( Ref r );
	const InstructionSet & instructionSet() {
		return this->instruction_set;
	}

public:
	Machine3();
	virtual ~Machine3() {}
};

#endif