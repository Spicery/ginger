#ifndef SYS_HPP
#define SYS_HPP

#include <map>
#include <iostream>

#include "common.hpp"
#include "arity.hpp"


class MachineClass;


extern void refPrint( Ref r );
extern void refPtrPrint( Ref * r );
extern void refPrint( std::ostream & out, Ref r );
extern void refPtrPrint( std::ostream & out, Ref * r );

extern Ref refKey( Ref r );

extern Ref * sysRefPrint( Ref * pc, MachineClass * );
extern Ref * sysRefPrintln( Ref * pc, MachineClass * );
extern Ref * sysNewList( Ref * pc, MachineClass * vm );
extern Ref * sysIsNil( Ref * pc, class MachineClass * vm );

#include "datatypes.hpp.auto"

struct SysInfo { 
	Functor functor; 
	Arity arity; 
	SysCall * syscall; 
	
	SysInfo( Functor f, Arity a, SysCall * s ) :
		functor( f ),
		arity( a ),
		syscall( s )
	{
	}
};
typedef std::map< std::string, SysInfo > SysMap;
extern SysMap sysMap;

#endif
