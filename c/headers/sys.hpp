#ifndef SYS_HPP
#define SYS_HPP

#include <map>

#include "common.hpp"
#include "arity.hpp"


extern Ref sys_key( Ref r );
extern void sys_print( Ref r );

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
