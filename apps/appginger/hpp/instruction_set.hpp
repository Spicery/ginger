#ifndef INSTRUCTION_SET_HPP
#define INSTRUCTION_SET_HPP

#include "common.hpp"
#include "instruction.hpp"
#include "listlayout.hpp"
#include "classlayout.hpp"
#include "methodlayout.hpp"
#include "maplayout.hpp"

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
void invoke_error() ;
void results_error( int d );
void arith_error( int x, int y );
void arith_error( int x );
void normal_exit();

#endif
