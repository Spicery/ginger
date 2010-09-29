/******************************************************************************\
	Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
	of the GNU General Public License. This file is part of AppGinger.

    AppGinger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AppGinger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AppGinger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

#include "sysstring.hpp"
#include "sysvector.hpp"
#include "key.hpp"
#include "misclayout.hpp"

Ref * sysStringAppend( Ref * pc, class MachineClass * vm ) {

	//	Variables here would be unaffected by a GC.
	unsigned long N;
	unsigned long lhs_n;
	unsigned long rhs_n;

	if ( vm->count != 2 ) throw Mishap( "Wrong number of arguments in stringAppend" );

	{
		//	May need to GC so leave on the stack.
		Ref rhs = vm->fastPeek();
		Ref lhs = vm->fastPeek( 1 );
		
		if ( !IsObj( lhs ) || !IsObj( rhs ) ) throw Mishap( "Invalid arguments in stringAppend" );
		
		Ref * lhs_K = RefToPtr4( lhs );
		Ref * rhs_K = RefToPtr4( rhs );
		Ref lhs_key = *lhs_K;
		Ref rhs_key = *rhs_K;
		
		if ( lhs_key != rhs_key || !IsSimpleKey( lhs_key ) || KindOfSimpleKey( lhs_key ) != STRING_KIND ) throw Mishap( "Invalid arguments in stringAppend" );

		lhs_n = lengthOfString( lhs_K );
		rhs_n = lengthOfString( rhs_K );
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

Ref * sysStringExplode( Ref *pc, class MachineClass * vm ) {
	if ( vm->count != 1 ) throw Mishap( "Wrong number of arguments for stringExplode" );
	Ref r = vm->fastPop();
	if ( !IsStringKind( r ) ) throw Mishap( "Argument mismatch for stringExplode" );
	Ref *obj_K = RefToPtr4( r );
	
	unsigned long n = sizeAfterKeyOfVector( obj_K );
	vm->checkStackRoom( n );
	char * p = reinterpret_cast< char * >( obj_K + 1 );
	for ( unsigned long i = 0; i < n; i++ ) {
		vm->fastPush( CharToCharacter( p[ i ] ) );
	}
	
	return pc;
}

Ref * sysStringLength( Ref *pc, class MachineClass * vm ) {
	if ( vm->count != 1 ) throw Mishap( "Wrong number of arguments for stringLength" );
	Ref r = vm->fastPop();
	if ( !IsStringKind( r ) ) throw Mishap( "Argument mismatch for stringLength" );
	Ref *obj_K = RefToPtr4( r );
	
	vm->fastPeek() = LongToSmall( sizeAfterKeyOfVector( obj_K ) );
	return pc;
}



