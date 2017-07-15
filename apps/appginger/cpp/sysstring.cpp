/******************************************************************************\
	Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
	of the GNU General Public License. This file is part of Ginger.

    Ginger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Ginger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Ginger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

#include "sysstring.hpp"
#include "sysvector.hpp"
#include "key.hpp"
#include "misclayout.hpp"
#include "stringlayout.hpp"
#include "heap.hpp"
#include "sys.hpp"

namespace Ginger {

Ref * sysStringAppend( Ref * pc, class MachineClass * vm ) {

	//	Variables here would be unaffected by a GC.
	unsigned long N;
	unsigned long lhs_n;
	unsigned long rhs_n;

	if ( vm->count != 2 ) throw Ginger::Mishap( "Wrong number of arguments in stringAppend" );

	{
		//	May need to GC so leave on the stack.
		Ref rhs = vm->fastPeek();
		Ref lhs = vm->fastPeek( 1 );
		
		if ( !IsObj( lhs ) || !IsObj( rhs ) ) throw Ginger::Mishap( "Invalid arguments in stringAppend" );
		
		Ref * lhs_K = RefToPtr4( lhs );
		Ref * rhs_K = RefToPtr4( rhs );
		Ref lhs_key = *lhs_K;
		Ref rhs_key = *rhs_K;
		
		if ( lhs_key != rhs_key || !IsSimpleKey( lhs_key ) || KindOfSimpleKey( lhs_key ) != STRING_KIND ) throw Ginger::Mishap( "Invalid arguments in stringAppend" );

		lhs_n = lengthOfStringLayout( lhs_K );
		rhs_n = lengthOfStringLayout( rhs_K );
		//	+ 1 for null byte
		N = ( lhs_n + rhs_n + 1 + sizeof( Ref ) - 1 ) / sizeof( Ref );
	}
	
	XfrClass xfr( vm->heap().preflight( pc, N + 2 ) );

	//	No risk of GC so safe to pop.
	Ref * rhs_K = RefToPtr4( vm->fastPop() );
	Ref * lhs_K = RefToPtr4( vm->fastPop() );

	xfr.xfrRef( LongToSmall( lhs_n + rhs_n ) );
	xfr.setOrigin();
	xfr.xfrRef( sysStringKey );
	xfr.xfrSubstringStep( 0, (char *)(lhs_K + 1), 0, lhs_n - 1 );
	xfr.xfrSubstringStep( lhs_n, (char *)(rhs_K + 1), 0, rhs_n );	//	Capture the null byte
	xfr.xfrSubstringFinish( lhs_n + rhs_n + 1 );
	vm->fastPush( xfr.makeRef() );
	return pc;
}
//SysMap::value_type( "stringAppend", SysInfo( Arity( 2 ), Arity( 1 ), sysStringAppend, "Appends two strings" ) ),
SysInfo infoStringAppend( 
    FullName( "stringAppend" ), 
    Ginger::Arity( 2 ), 
    Ginger::Arity( 1 ), 
    sysStringAppend, 
    "Appends two strings."
);

Ref * sysStringIndex( Ref *pc, class MachineClass * vm ) {
	if ( vm->count != 2 ) throw Ginger::Mishap( "ArgsMismatch" );
	Ref str = vm->fastPop();
	if ( !IsString( str ) ) throw Ginger::Mishap( "TypeError" );
	Ref idx = vm->fastPeek();
	if ( !IsSmall( idx ) ) throw Ginger::Mishap( "TypeError" );
	Ref * str_K = RefToPtr4( str );
	char * data = reinterpret_cast< char * >( str_K + 1 ) - 1;
	
	const long I = SmallToLong( idx );
	const long N = SmallToLong( str_K[ STRING_OFFSET_LENGTH ] );
	
	if ( 1 <= I && I <= N ) {
		vm->fastPeek() = CharToCharacter( data[ I ] );
	} else {
		throw Ginger::Mishap( "OutOfRange" );
	}
	
	return pc;
}
//SysMap::value_type( "stringIndex", SysInfo( Arity( 2 ), Arity( 1 ), sysStringIndex, "Indexes a string" ) ),
SysInfo infoStringIndex( 
    FullName( "stringIndex" ), 
    Ginger::Arity( 2 ), 
    Ginger::Arity( 1 ), 
    sysStringIndex, 
    "Indexes a string."
);

Ref * sysStringExplode( Ref *pc, class MachineClass * vm ) {
	if ( vm->count != 1 ) throw Ginger::Mishap( "Wrong number of arguments for stringExplode" );
	Ref r = vm->fastPop();
	if ( !IsStringKind( r ) ) throw Ginger::Mishap( "Argument mismatch for stringExplode" );
	Ref *obj_K = RefToPtr4( r );
	
	long n = SmallToLong( obj_K[ STRING_LAYOUT_OFFSET_LENGTH ] );
	vm->checkStackRoom( n );
	char * p = reinterpret_cast< char * >( obj_K + 1 );
	for ( long i = 0; i < n; i++ ) {
		vm->fastPush( CharToCharacter( p[ i ] ) );
	}
	
	return pc;
}
//SysMap::value_type( "stringExplode", SysInfo( Arity( 1 ), Arity( 0, true ), sysStringExplode, "Explodes a string into its members" ) ),
SysInfo infoStringExplode( 
    FullName( "stringExplode" ), 
    Ginger::Arity( 1 ), 
    Ginger::Arity( 0, true ), 
    sysStringExplode, 
    "Explodes a string into its constituent characters."
);

Ref * sysStringLength( Ref *pc, class MachineClass * vm ) {
	if ( vm->count != 1 ) throw Ginger::Mishap( "Wrong number of arguments for stringLength" );
	Ref r = vm->fastPop();
	if ( !IsStringKind( r ) ) throw Ginger::Mishap( "Argument mismatch for stringLength" );
	Ref *obj_K = RefToPtr4( r );
	
	vm->fastPeek() = obj_K[ STRING_LAYOUT_OFFSET_LENGTH ];
	return pc;
}
//SysMap::value_type( "stringLength", SysInfo( Arity( 1 ), Arity( 1 ), sysStringLength, "Returns the length of a string" ) ),
SysInfo infoStringLength( 
    FullName( "stringLength" ), 
    Ginger::Arity( 1 ), 
    Ginger::Arity( 1 ), 
    sysStringLength, 
    "Returns the length of a string."
);

Ref * sysNewString( Ref *pc, class MachineClass * vm ) {
	std::string sofar;
	vm->vp -= vm->count;
	for ( int i = 1; i <= vm->count; i++ ) {
		sofar.push_back( CharacterToChar( vm->vp[ i ] ) );
	}
	vm->checkStackRoom( 1 );
	vm->fastPush( vm->heap().copyString( pc, sofar.c_str() ) );
	return pc;
}
//SysMap::value_type( "newString", SysInfo( Arity( 0, true ), Arity( 1 ), sysNewString, "Creates a string from a set of characters" ) ),
SysInfo infoNewString( 
    FullName( "newString" ), 
    Ginger::Arity( 0, true ), 
    Ginger::Arity( 1 ), 
    sysNewString, 
    "Creates a string from a tuple of characters."
);

} // namespace Ginger
