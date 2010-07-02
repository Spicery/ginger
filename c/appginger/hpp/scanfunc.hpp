#ifndef SCAN_HPP
#define SCAN_HPP

#include "instruction_set.hpp"

class ScanFunc {
private:
	Ref * pc;
	Ref * end;
	int offset;
	InstructionSet inset;
	
public:
	Ref * next();
	
public:
	ScanFunc( const InstructionSet &, Ref * p );
};

#endif