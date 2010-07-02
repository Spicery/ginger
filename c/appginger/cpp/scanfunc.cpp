#include <iostream>

#include "scanfunc.hpp"
#include "instruction_set.hpp"
#include "key.hpp"

ScanFunc::ScanFunc( const InstructionSet & ins, Ref * r ) {
	this->inset = ins;
	this->pc = r + 1;
	this->offset = 0;
	Ref * start = r - 4;
	unsigned long L = ToULong( *start ) >> TAGGG;			
	this->end = start + L;
}

Ref * ScanFunc::next() {
	while ( this->pc < this->end ) {
		//std::cerr << "Instruction " << this->inset.name( *this->pc ) << std::endl;
		const char *types = this->inset.signature( *this->pc );
		for (;;) {
			const char ch = types[ this->offset++ ];
			if ( ch == '\0' ) {
				break;			
			} else if ( ch == 'c' || ch == 'v' ) {
				return this->pc  + this->offset - 1;
			}
		}
		this->pc += this->offset - 1;
		this->offset = 0;
	}
	return 0;
}