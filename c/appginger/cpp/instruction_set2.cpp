#include "instruction_set2.hpp"

#include "machine.hpp"
#include "callstacklayout.hpp"
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
extern Ref *pc;
extern Machine vm;
#define RETURN( e ) 	{ pc = ( e ); return; }
#include "instruction_set2.cpp.auto"