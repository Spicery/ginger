#include <map>
#include <string>
#include <iostream>

#include <stdio.h>

#include "sys.hpp"
#include "key.hpp"
#include "machine.hpp"

//#define DBG_SYS

Ref sys_key( Ref r ) {
	unsigned long u = ( unsigned long )r;
	unsigned long tag, tagg, taggg;
	tag = u & TAG_MASK;
	if ( tag == INT_TAG ) return sysSmallKey;
	if ( tag == PTR_TAG ) return *RefToPtr4(r);
	tagg = u & TAGG_MASK;
	if ( tagg == ( 0 | SIM_TAG ) ) return sysAbsentKey;
	if ( tagg == ToULong( sys_false ) || tagg == ToULong( sys_true ) ) return sysBoolKey;
	if ( tagg == FN_TAGG ) return sysFnKey;
	if ( tagg == KEY_TAGG ) return sysKeyKey;
	taggg = u & TAGGG_MASK;
	if ( taggg == CHAR_TAGGG ) return sysCharKey;
	return sys_absent;
}

void sys_print( Ref r ) {
	sys_print( std::cout, r );
}

void sys_print( std::ostream & out, Ref r ) {
	Ref k;
#ifdef DBG_SYS	
	out << "About to print '" << (unsigned int) r << "'\n";
#endif
	k = sys_key( r );
#ifdef DBG_SYS
	out << "key = " << ToULong(k) << "\n";
#endif
	if ( k == sysStringKey ) {
		Ref *rr = RefToPtr4( r );
		char *s = ToChars( rr + 1 );
		out << s;
	} else if ( k == sysSmallKey ) {
		out << SmallToLong( r );
	} else if ( k == sysBoolKey ) {
		out << ( r == sys_false ? "false" : "true" );
	} else if ( k == sysAbsentKey ) {
		out << "absent";
	} else if ( k == sysFnKey ) {
		out << "<function>";
	} else if ( k == sysCharKey ) {
		out << CharacterToChar( r );
	} else {
		out << "?";
	}
}

void sysGarbageCollect( MachineClass * vm ) {
	vm->heap().garbageCollect();
}

typedef std::map< std::string, SysInfo > SysMap;

/*
	struct { Functor fnc; int arity; } flavour = (
		name == "+" ? struct { fnc_add, 2 } :
		name == "*" ? { fnc_mul, 2 } :
		name == "-" ? { fnc_sub, 2 } :
		name == "/" ? { fnc_div, 2 } :
		name == "**" ? { fnc_pow, 2 } :
		name == "<" ? { fnc_lt, 2 } :
		name == "<=" ? { fnc_lte, 2 } :
		name == ">" ? { fnc_gt, 2 } :
		name == ">=" ? { fnc_gte, 2 } :
		name == "==" ? { fnc_eq, 2 } :
		name == "not" ? { fnc_not :
		fnc_unknown
	);
*/
const SysMap::value_type rawData[] = {
	SysMap::value_type( "+", SysInfo( fnc_add, Arity( 2 ), 0 ) ),
	SysMap::value_type( "-", SysInfo( fnc_sub, Arity( 2 ), 0 ) ),
	SysMap::value_type( "*", SysInfo( fnc_mul, Arity( 2 ), 0 ) ),
	SysMap::value_type( "/", SysInfo( fnc_div, Arity( 2 ), 0 ) ),
	SysMap::value_type( "**", SysInfo( fnc_pow, Arity( 2 ), 0 ) ),
	SysMap::value_type( "<", SysInfo( fnc_lt, Arity( 2 ), 0 ) ),
	SysMap::value_type( "<=", SysInfo( fnc_lte, Arity( 2 ), 0 ) ),
	SysMap::value_type( "==", SysInfo( fnc_eq, Arity( 2 ), 0 ) ),
	SysMap::value_type( ">", SysInfo( fnc_gt, Arity( 2 ), 0 ) ),
	SysMap::value_type( ">=", SysInfo( fnc_gte, Arity( 2 ), 0 ) ),	
	SysMap::value_type( "gc", SysInfo( fnc_syscall, Arity( 0 ), sysGarbageCollect ) )
};
const int numElems = sizeof rawData / sizeof rawData[0];
SysMap sysMap( rawData, rawData + numElems );