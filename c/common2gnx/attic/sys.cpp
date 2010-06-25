#include <map>
#include <string>

#include <stdio.h>

#include "sys.hpp"
#include "key.hpp"
#include "machine.hpp"

//#define DBG_SYS

Ref sys_key( Ref r ) {
	unsigned int u = ( unsigned int )r;
	unsigned int tag, tagg, taggg;
	tag = u & TAG_MASK;
	if ( tag == INT_TAG ) return sysSmallKey;
	if ( tag == PTR_TAG ) return *RefToPtr4(r);
	tagg = u & TAGG_MASK;
	if ( tagg == ( 0 | SIM_TAG ) ) return sysAbsentKey;
	if ( tagg == ToUInt( sys_false ) || tagg == ToUInt( sys_true ) ) return sysBoolKey;
	if ( tagg == FN_TAGG ) return sysFnKey;
	if ( tagg == KEY_TAGG ) return sysKeyKey;
	taggg = u & TAGGG_MASK;
	if ( taggg == CHAR_TAGGG ) return sysCharKey;
	return sys_absent;
}

void sys_print( Ref r ) {
	Ref k;
#ifdef DBG_SYS	
	printf( "About to print '%x'\n", (unsigned int) r );
#endif
	k = sys_key( r );
#ifdef DBG_SYS
	printf( "key = %x\n", ToUInt(k) );
#endif
	if ( k == sysStringKey ) {
		Ref *rr = RefToPtr4( r );
		char *s = ToChars( rr + 1 );
		printf( "%s", s );
	} else if ( k == sysSmallKey ) {
		printf( "%d", SmallToInt ( r ) );
	} else if ( k == sysBoolKey ) {
		printf( "%s", r == sys_false ? "false" : "true" );
	} else if ( k == sysAbsentKey ) {
		printf( "absent" );
	} else if ( k == sysFnKey ) {
		printf( "<function>" );
	} else if ( k == sysCharKey ) {
		printf( "%c", CharacterToChar( r ) );
	} else {
		printf( "?" );
	}
}

void sysGarbageCollect( MachineClass * vm ) {
	vm->heap().garbageCollect();
}

typedef std::map< std::string, SysCall * > SysMap;

const SysMap::value_type rawData[] = {
	SysMap::value_type( "gc", sysGarbageCollect )
};
const int numElems = sizeof rawData / sizeof rawData[0];
SysMap sysMap( rawData, rawData + numElems );