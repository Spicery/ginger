#include "instruction_set1.hpp"

#include "machine.hpp"
#include "mishap.hpp"
#include "common.hpp"
#include "key.hpp"
#include "sys.hpp"

#include <cstdio>
#include <cstdlib>


#define FREEZE
#define MELT
#define VMLINK 	( vm->link )
#define VMSP	( vm->sp )
#define VMVP	( vm->vp )
#define VMCOUNT	( vm->count )
//typedef Ref *SpecialFn( Ref *pc, Machine vm );
//typedef SpecialFn *Special;
#define RETURN( e ) 	return( e )

#include "instruction_set1.cpp.auto"