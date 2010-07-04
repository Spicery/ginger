#include "machine3.hpp"

#include <cstdio>
#include <cstdlib>


//	Now source the auto-generated C++ file. The 
//	header files are provided here (for no very
//	good reason.)
#include "machine.hpp"
#include "key.hpp"
#include "sys.hpp"
#include "mishap.hpp"
#include "appginger.hpp"

Machine3::Machine3( AppGinger & g ) :
	MachineClass( g )
{
	this->core( true, 0 );
}

void Machine3::execute( Ref r ) {
	Ref * PC = this->setUpPC( r );
	this->core( false, PC );
}

#define vm 		this
#define VMCOUNT ( vm->count )
#define MELT { VMSP = vm->sp; VMVP = vm->vp; VMLINK = vm->link; }
#define FREEZE { vm->sp = VMSP; vm->vp = VMVP; vm->link = VMLINK; }
typedef void *Special;
#define RETURN( e )     { pc = ( e ); goto **pc; }

#include "machine3.cpp.auto"