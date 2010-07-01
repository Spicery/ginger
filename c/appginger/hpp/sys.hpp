#ifndef SYS_HPP
#define SYS_HPP

#include <map>
#include <iostream>

#include "common.hpp"
#include "arity.hpp"


class MachineClass;


extern Ref sys_key( Ref r );
extern void sys_print( Ref r );
extern void sys_print( std::ostream & out, Ref r );

extern void sysNewList( MachineClass * vm );
extern void sysHead( MachineClass * vm );
extern void sysGarbageCollect( MachineClass * vm );

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
