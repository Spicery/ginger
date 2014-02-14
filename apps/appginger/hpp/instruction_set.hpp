#ifndef INSTRUCTION_SET_HPP
#define INSTRUCTION_SET_HPP

#include <string>

#include "cell.hpp"
#include "common.hpp"
#include "instruction.hpp"
#include "listlayout.hpp"
#include "classlayout.hpp"
#include "methodlayout.hpp"
#include "maplayout.hpp"
#include "sysapply.hpp"
#include "bigint.hpp"
#include "externalkind.hpp"
#include "sysmaths.hpp"

namespace Ginger {

class InstructionSet {
public:
#include "instruction_set.hpp.auto"
public: 
	Ref lookup( Instruction instr ) const;
	Instruction findInstruction( const std::string & name  ) const;
	bool isByPassInstruction( Ref * pc ) const;
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

}

#endif
