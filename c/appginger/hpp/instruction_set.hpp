#ifndef INSTRUCTION_SET_HPP
#define INSTRUCTION_SET_HPP

#include "common.hpp"
#include "instruction.hpp"

class InstructionSet {
public:
#include "instruction_set.hpp.auto"
public: 
	Ref lookup( Instruction instr ) const;
	const char * name( Ref spc ) const;
	const char * signature( Ref x ) const;
};

void enter_error( int B, int A );
void call_error( Ref r ) ;
void results_error( int d );
void arith_error( int x, int y );
void arith_error( int x );
void normal_exit();

#endif