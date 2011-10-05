#include "instruction_set1.hpp"

#include "instructions_context.hpp"

#include <cstdio>
#include <cstdlib>

using namespace Ginger;

#define FREEZE
#define MELT
#define VMLINK 	( vm->link )
#define VMSP	( vm->sp )
#define VMVP	( vm->vp )
#define VMCOUNT	( vm->count )
#define VMLINKFUNC 	( vm->func_of_link )
#define VMPCFUNC	( vm->func_of_program_counter )
#define RETURN( e ) 	return( e )

#include "instruction_set1.cpp.auto"
