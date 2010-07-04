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

Ref refKey( Ref r ) {
	unsigned long u = ( unsigned long )r;
	unsigned long tag, tagg, taggg;
	tag = u & TAG_MASK;
	if ( tag == INT_TAG ) return sysSmallKey;
	if ( tag == PTR_TAG ) return *RefToPtr4(r);
	tagg = u & TAGG_MASK;
	if ( tagg == ( 0 | SIM_TAG ) ) return sysAbsentKey;
	if ( tagg == ToULong( sys_false ) || tagg == ToULong( sys_true ) ) return sysBoolKey;
	if ( tagg == FN_TAGG ) return sysFunctionKey;
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

void refPrint( Ref r ) {
	refPrint( std::cout, r );
}

void refPrint( std::ostream & out, Ref r ) {
	Ref k;
#ifdef DBG_SYS	
	out << "About to print '" << (unsigned int) r << "'\n";
#endif
	k = refKey( r );
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
	} else if ( k == sysFunctionKey ) {
		out << "<function>";
	} else if ( k == sysCharKey ) {
		out << CharacterToChar( r );
	} else if ( k == sysPairKey ) {
		Ref sofar = r;
		bool sep = false;
		out << "[";
		while ( refKey( sofar ) == sysPairKey ) {
			if ( sep ) { out << ","; } else { sep = true; }
			refPrint( out, *( RefToPtr4( sofar ) + 1 ) );
			sofar = *( RefToPtr4( sofar ) + 2 );
		}
		out << "]";
	} else if ( k == sysNilKey ) {
		out << "[]";
	} else if ( k == sysVectorKey ) {
		bool sep = false;
		out << "{";
		Ref * p = RefToPtr4( r );
		long len = RefToLong( p[ -1 ] );
		for ( int i = 1; i <= len; i++ ) {
			if ( sep ) { out << ","; } else { sep = true; }
			refPrint( p[ i ] ); 
		}
		out << "}";
	} else {
		out << "?(" << std::hex << ToULong( r ) << ")";
	}
}

void sysGarbageCollect( class MachineClass * vm ) {
	vm->heap().garbageCollect();
}

void sysRefPrint( class MachineClass * vm ) {
	for ( int i = vm->count - 1; i >= 0; i-- ) {
		Ref r = vm->fastSubscr( i );
		refPrint( r );		
	}
	vm->fastDrop( vm->count );
}

void sysRefPrintln( class MachineClass * vm ) {
	sysRefPrint( vm );
	std::cout << std::endl;
}

void sysNewVector( MachineClass * vm ) {
	int n = vm->count;
	XfrClass xfr( vm->heap().preflight( 2 + n ) );
	xfr.xfrRef( IntToSmall( n ) );
	xfr.setOrigin();
	xfr.xfrRef( sysVectorKey );
	xfr.xfrCopy( ++vm->vp -= n, n );
	vm->fastSet( xfr.make() );
}

static inline void recognise( class MachineClass * vm, Ref key ) {
	if ( vm->count == 1 ) {
		Ref r = vm->fastPeek();
		vm->fastPeek() = IsPtr4( r ) && ( *RefToPtr4( r ) == key ) ? sys_true : sys_false;
	} else {
		throw Mishap( "Wrong number of arguments for head" );
	}
}

void sysIsVector( class MachineClass * vm ) {
	recognise( vm, sysVectorKey );
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

void sysNewPair( class MachineClass * vm ) {
	if ( vm->count == 2 ) {
		XfrClass xfr( vm->heap().preflight( 3 ) );
		xfr.setOrigin();
		xfr.xfrRef( sysPairKey );
		Ref t = vm->fastPop();
		Ref h = vm->fastPop();
		xfr.xfrRef( h );
		xfr.xfrRef( t );
		vm->fastPush( xfr.make() );
	} else {
		throw Mishap( "Wrong number of arguments for newPair" );
	}
}

void sysHead( class MachineClass * vm ) {
	if ( vm->count == 1 ) {
		Ref x = vm->fastPeek();
		if ( refKey( x ) == sysPairKey ) {
			vm->fastPeek() = RefToPtr4( x )[ 1 ];
		} else {
			throw Mishap( "Trying to take the head of a non-pair" );
		}
	} else {
		throw Mishap( "Wrong number of arguments for head" );
	}
}

void sysTail( class MachineClass * vm ) {
	if ( vm->count == 1 ) {
		Ref x = vm->fastPeek();
		if ( refKey( x ) == sysPairKey ) {
			vm->fastPeek() = RefToPtr4( x )[ 2 ];
		} else {
			throw Mishap( "Trying to take the head of a non-pair" );
		}
	} else {
		throw Mishap( "Wrong number of arguments for head" );
	}
}

void sysIsPair( class MachineClass * vm ) {
	if ( vm->count == 1 ) {
		vm->fastPeek() = IsPair( vm->fastPeek() ) ? sys_true : sys_false;
	} else {
		throw Mishap( "Wrong number of arguments for head" );
	}
}

void sysIsNil( class MachineClass * vm ) {
	if ( vm->count == 1 ) {
		vm->fastPeek() = IsNil( vm->fastPeek() ) ? sys_true : sys_false;
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
	SysMap::value_type( "refPrint", SysInfo( fnc_syscall, Arity( 1 ), sysRefPrint ) ),
	SysMap::value_type( "refPrintln", SysInfo( fnc_syscall, Arity( 1 ), sysRefPrintln ) ),
	SysMap::value_type( "newList", SysInfo( fnc_syscall, Arity( 0, true ), sysNewList ) ),
	SysMap::value_type( "newPair", SysInfo( fnc_syscall, Arity( 2 ), sysNewPair ) ),
	SysMap::value_type( "head", SysInfo( fnc_syscall, Arity( 1 ), sysHead ) ),
	SysMap::value_type( "tail", SysInfo( fnc_syscall, Arity( 1 ), sysTail ) ),
	SysMap::value_type( "isPair", SysInfo( fnc_syscall, Arity( 1 ), sysIsPair ) ),
	SysMap::value_type( "isNil", SysInfo( fnc_syscall, Arity( 1 ), sysIsNil ) ),
	SysMap::value_type( "newVector", SysInfo( fnc_syscall, Arity( 0, true ), sysNewVector ) ),
	SysMap::value_type( "isVector", SysInfo( fnc_syscall, Arity( 1 ), sysIsVector ) ),
};
const int numElems = sizeof rawData / sizeof rawData[0];
SysMap sysMap( rawData, rawData + numElems );