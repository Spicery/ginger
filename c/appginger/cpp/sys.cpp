#include <map>
#include <string>
#include <iostream>

#include <stdio.h>

#include "machine.hpp"
#include "sys.hpp"
#include "key.hpp"
#include "cage.hpp"
#include "mishap.hpp"

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
	if ( taggg == MISC_TAGGG ) {
		if ( r == sys_nil ) {
			return sysNilKey; 
		} else {
			throw;
		}
	}
	throw;
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
	} else if ( k == sysPairKey ) {
		Ref sofar = r;
		bool sep = false;
		out << "[";
		while ( sys_key( sofar ) == sysPairKey ) {
			if ( sep ) { out << ","; } else { sep = true; }
			sys_print( out, *( RefToPtr4( sofar ) + 1 ) );
			sofar = *( RefToPtr4( sofar ) + 2 );
		}
		out << "]";
	} else if ( k == sysNilKey ) {
		out << "[]";
	} else {
		out << "?(" << std::hex << ToULong( r ) << ")";
	}
}

void sysGarbageCollect( class MachineClass * vm ) {
	vm->heap().garbageCollect();
}

void sysNewList( class MachineClass * vm ) {
	Ref sofar = sys_nil;
	int n = vm->count;
	//std::cerr << "Count " << n << std::endl;
	XfrClass xfr( vm->heap().preflight( 3 * n ) );
	for ( int i = 0; i < n; i++ ) {
		xfr.setOrigin();
		xfr.xfrRef( sysPairKey );
		//std::cerr << "Peek: " << vm->fastPeek() << std::endl;
		//std::cerr << "Peek: " << vm->fastPeek() << std::endl;
		Ref r = vm->fastPop();
		//std::cerr << "Pop: " << r << std::endl;

		xfr.xfrRef( r );
		xfr.xfrRef( sofar );
		sofar = xfr.make();
		//std::cerr << "Stored " << *( RefToPtr4( sofar ) + 1 ) << std::endl;
	}
	vm->fastPush( sofar );
}

void sysHead( class MachineClass * vm ) {
	if ( vm->count == 1 ) {
		Ref x = vm->fastPeek();
		if ( sys_key( x ) == sysPairKey ) {
			vm->fastPeek() = RefToPtr4( x )[ 1 ];
		} else {
			throw Mishap( "Trying to take the head of a non-pair" );
		}
	} else {
		throw Mishap( "Wrong number of arguments for head" );
	}
}

typedef std::map< std::string, SysInfo > SysMap;

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
	SysMap::value_type( "gc", SysInfo( fnc_syscall, Arity( 0 ), sysGarbageCollect ) ),
	SysMap::value_type( "newList", SysInfo( fnc_syscall, Arity( 0, true ), sysNewList ) ),
	SysMap::value_type( "head", SysInfo( fnc_syscall, Arity( 1 ), sysHead ) )
};
const int numElems = sizeof rawData / sizeof rawData[0];
SysMap sysMap( rawData, rawData + numElems );