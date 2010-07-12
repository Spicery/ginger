#ifndef COMMON_HPP
#define COMMON_HPP


typedef unsigned long ulong;
typedef const char *charseq;
class MachineClass;

typedef void * Ref;
typedef void ** RefRef;

typedef Ref * SysCall( Ref * pc, MachineClass * );

#define ToRef( x )  	( ( Ref )( x ) )
#define ToRefRef( x )	( ( RefRef )( x ) )
#define ToULong( x ) 	( ( unsigned long )( x ) )
#define ToLong( x )		( ( long )( x ) )
#define	ToChars( x ) 	( ( char * )( x ) )
#define ToSysCall( x ) 	( (SysCall *)( x ) )

#endif
