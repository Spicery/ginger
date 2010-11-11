#ifndef IMPLEMENTATION_H
#define IMPLEMENTATION_H


//#define IMPLEMENTATION1
//#define IMPLEMENTATION2
//#define IMPLEMENTATION3

#include "common.hpp"
#include "machine.hpp"


#ifdef IMPLEMENTATION1
	#define FREEZE
	#define MELT
	#define VMLINK 	( vm->link )
	#define VMSP	( vm->sp )
	#define VMVP	( vm->vp )
	#define VMCOUNT	( vm->count )
	//typedef Ref *SpecialFn( Ref *pc, Machine vm );
	//typedef SpecialFn *Special;
	#define RETURN( e ) 	return( e )
#endif

#ifdef IMPLEMENTATION2
	#define FREEZE
	#define MELT
	#define VMLINK 	( vm->link )
	#define VMSP	( vm->sp )
	#define VMVP	( vm->vp )
	#define VMCOUNT	( vm->count )
    extern Ref *pc;
    extern Machine vm;
	//typedef void SpecialFn( void );
	//typedef SpecialFn *Special;
	#define RETURN( e ) 	{ pc = ( e ); return; }
#endif

#ifdef IMPLEMENTATION3
	#define VMCOUNT	( vm->count )
	#define MELT { VMSP = vm->sp; VMVP = vm->vp; VMLINK = vm->link; }
	#define FREEZE { vm->sp = VMSP; vm->vp = VMVP; vm->link = VMLINK; }
	//typedef void *Special;
	#define RETURN( e ) 	{ pc = ( e ); goto **pc; }
#endif

#define ToSpecial( x )		((Special)(x))

#endif
