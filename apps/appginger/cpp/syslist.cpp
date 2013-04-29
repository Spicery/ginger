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

#include "syslist.hpp"
#include "sysvector.hpp"
#include "key.hpp"
#include "listlayout.hpp"

namespace Ginger {

Ref * sysNewList( Ref * pc, class MachineClass * vm ) {
	Ref sofar = SYS_NIL;
	int n = vm->count;
	//std::cerr << "Count " << n << std::endl;
	XfrClass xfr( vm->heap().preflight( pc, PAIR_SIZE * n ) );
	for ( int i = 0; i < n; i++ ) {
		xfr.setOrigin();
		xfr.xfrRef( sysPairKey );
		//std::cerr << "Peek: " << vm->fastPeek() << std::endl;
		//std::cerr << "Peek: " << vm->fastPeek() << std::endl;
		Ref r = vm->fastPop();
		//std::cerr << "Pop: " << r << std::endl;

		xfr.xfrRef( r );
		xfr.xfrRef( sofar );
		sofar = xfr.makeRef();
		//std::cerr << "Stored " << *( RefToPtr4( sofar ) + 1 ) << std::endl;
	}
	vm->fastPush( sofar );
	return pc;
}

Ref * sysNewListOnto( Ref * pc, class MachineClass * vm ) {
	int n = vm->count - 1;
	if ( n < 0 ) throw "Too few arguments";
	Ref sofar = vm->fastPop();
	//std::cerr << "Count " << n << std::endl;
	XfrClass xfr( vm->heap().preflight( pc, PAIR_SIZE * n ) );
	for ( int i = 0; i < n; i++ ) {
		xfr.setOrigin();
		xfr.xfrRef( sysPairKey );
		//std::cerr << "Peek: " << vm->fastPeek() << std::endl;
		//std::cerr << "Peek: " << vm->fastPeek() << std::endl;
		Ref r = vm->fastPop();
		//std::cerr << "Pop: " << r << std::endl;

		xfr.xfrRef( r );
		xfr.xfrRef( sofar );
		sofar = xfr.makeRef();
		//std::cerr << "Stored " << *( RefToPtr4( sofar ) + 1 ) << std::endl;
	}
	vm->fastPush( sofar );
	return pc;
}



Ref * sysIsNil( Ref * pc, class MachineClass * vm ) {
	if ( vm->count == 1 ) {
		vm->fastPeek() = IsNil( vm->fastPeek() ) ? SYS_TRUE : SYS_FALSE;
		return pc;
	} else {
		throw Ginger::Mishap( "Wrong number of arguments for head" );
	}
}

Ref * sysIsList( Ref * pc, class MachineClass * vm ) {
	if ( vm->count == 1 ) {
		Ref r = vm->fastPeek();
		vm->fastPeek() = IsPair( r ) || IsNil( r ) ? SYS_TRUE : SYS_FALSE;
		return pc;
	} else {
		throw Ginger::Mishap( "Wrong number of arguments for head" );
	}
}



Ref * sysListAppend( Ref * pc, class MachineClass * vm ) {
	//	Variables here are unaffected by a GC.
	ptrdiff_t D;
	
	if ( vm->count != 2 ) throw Ginger::Mishap( "Wrong number of arguments in listAppend" );

	//	This section is carefully to survive relocations of the
	//	call/value stacks. Will NOT survive a GC.
	{
		//	May need to GC so leave on the stack.
		Ref rhs = vm->fastPeek();
		Ref lhs = vm->fastPeek( 1 );
		
		//	Typecheck arguments.
		if ( !isList( lhs ) || !isList( rhs ) ) throw Ginger::Mishap( "Invalid arguments in listAppend" );
		
		ptrdiff_t start = vm->stackLength();
		while ( IsPair( lhs ) ) {
			vm->checkStackRoom( 1 );
			vm->fastPush( fastPairHead( lhs ) );
			lhs = fastPairTail( lhs );
		}
		vm->checkStackRoom( 1 );
		vm->fastPush( rhs );
		D = vm->stackLength() - start;
	}

	vm->count = D;
	pc = sysNewListOnto( pc, vm );

	//	Now fix the value stack.
	Ref r = vm->fastPop();
	vm->fastPop();
	vm->fastPeek() = r;
	return pc;
}

Ref * sysListIndex( Ref *pc, class MachineClass * vm ) {
	if ( vm->count != 2 ) throw Ginger::Mishap( "ArgsMismatch" );
	Ref list = vm->fastPop();
	Ref idx = vm->fastPeek();
	if ( !IsSmall( idx ) ) throw Ginger::Mishap( "TypeError" );
	
	long I = SmallToLong( idx );
	if ( I <= 0 ) throw Ginger::Mishap( "OutOfRange" );
	
	while ( IsPair( list ) && --I > 0 ) {
		list = fastPairTail( list );
	}
	
	if ( IsPair( list ) ) {
		vm->fastPeek() = fastPairHead( list );
	} else if ( IsNil( list ) ) {
		throw Ginger::Mishap( "OutOfRange" );
	} else {
		throw Ginger::Mishap( "TypeError" );
	}
	
	return pc;
}



Ref * sysListExplode( Ref *pc, class MachineClass * vm ) {
	if ( vm->count != 1 ) throw Ginger::Mishap( "Wrong number of arguments for listExplode" );
	Ref r = vm->fastPop();
	if ( !isList( r ) ) throw Ginger::Mishap( "Argument mismatch for listExplode" );

	while ( IsPair( r ) ) {
		vm->checkStackRoom( 1 );
		vm->fastPush( fastPairHead( r ) );
		r = fastPairTail( r ) ;
	}

	return pc;
}

Ref * sysListLength( Ref *pc, class MachineClass * vm ) {
	if ( vm->count != 1 ) throw Ginger::Mishap( "Wrong number of arguments for listLength" );
	Ref r = vm->fastPeek();
	if ( !isList( r ) ) throw Ginger::Mishap( "Argument mismatch for listLength" );

	int count = 0;
	while ( IsPair( r ) ) {
		count += 1;
		r = fastPairTail( r );
	} 
	vm->fastPeek() = LongToSmall( count );
	
	return pc;
}

Ref * sysFastListLength( Ref *pc, class MachineClass * vm ) {
	Ref r = vm->fastPeek();

	int count = 0;
	while ( IsPair( r ) ) {
		count += 1;
		r = fastPairTail( r );
	} 
	vm->fastPeek() = LongToSmall( count );
	
	return pc;
}

} // namespace Ginger