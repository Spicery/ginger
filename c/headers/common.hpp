#ifndef COMMON_HPP
#define COMMON_HPP

typedef unsigned long ulong;
typedef const char *charseq;
class MachineClass;
typedef void SysCall( MachineClass * );

#define CELL_SCHEME 1

#if CELL_SCHEME == 1

typedef void * Ref;
typedef void ** RefRef;

#define ToRef( x )  	( ( Ref )( x ) )
#define ToRefRef( x )	( ( RefRef )( x ) )
#define ToULong( x ) 	( ( unsigned long )( x ) )
#define ToLong( x )		( ( long )( x ) )
#define	ToChars( x ) 	( ( char * )( x ) )
#define ToSysCall( x ) 	( (SysCall *)( x ) )

#elif CELL_SCHEME == 2

"Work in progress"

typedef union {
	unsigned word		raw;
	packed struct Cell {
		unsigned int	data : 30;
		int				
	}					cell;
	struct Cell {
	unsigned word		data;
} Ref;



#endif

#endif
