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

#include "mishap.hpp"
#include "machine.hpp"
#include "sysstack.hpp"
#include "sysprint.hpp"
#include "sys.hpp"

using namespace Ginger;

Ref * sysFastSwap( Ref *pc, class MachineClass * vm ) {
    Ref r = vm->fastPeek();
    vm->fastPeek() = vm->fastPeek( 1 );
    vm->fastPeek( 1 ) = r;
    return pc;
}

Ref * sysSwap( Ref *pc, class MachineClass * vm ) {
    if ( vm->count != 2 ) throw Ginger::Mishap( "Wrong number of arguments" );
    return sysFastSwap( pc, vm );
}

SysInfo infoSwap(
    SysNames( "swap" ), 
    Arity( 2 ), 
    Arity( 2 ), 
    sysSwap, 
    "Swaps two values around"
);

// - eraseAll ------------------------------------------------------------------

Ref * sysEraseAll( Ref * pc, class MachineClass * vm ) {
    vm->fastDrop( vm->count );
    return pc;
}
SysInfo infoEraseAll(
    SysNames( "eraseAll" ),
    Arity( 0, true ), 
    Arity( 0 ), 
    sysEraseAll, 
    "Discards all the arguments it is given"
);

// - eraseLeading ---------------------------------------------------------------

Ref * sysEraseLeading( Ref * pc, class MachineClass * vm ) {
    if ( vm->count < 1 ) {
        throw Ginger::Mishap( "No arguments" );
    }
    Ref count = vm->fastPop();
    if ( not IsSmall( count ) ) {
        throw Ginger::Mishap( "Last argument must be an integer" ).culprit( "Last arg", refToShowString( count ) );
    }
    long n = SmallToLong( count );
    if ( n < 0 ) {
        throw Ginger::Mishap( "Argument must be >= 0" ).culprit( "Erasure count", n );
    }
    long k = vm->count - 1;
    if ( k <= n ) {
        if ( k == n ) {
            vm->fastDrop( n );
        } else {
            throw Ginger::Mishap( "Trying to erase more values than available" ).culprit( "Erasing", n ).culprit( "Available", k );
        }
    } else {
        long k1 = k - 1;
        long num_swaps = k - n;
        for ( int i = 0; i < num_swaps; i++ ) {
            vm->fastPeek( k1 - i ) = vm->fastPeek( k1 - n - i );
        }
        vm->fastDrop( n );
    }
    return pc;
}
SysInfo infoEraseLeading(
    SysNames( "eraseLeading" ),
    Arity( 1, true ),
    Arity( 0, true ),
    sysEraseLeading,
    "Given args (arg1, ..., arg_n, N ) discards the first N arguments it is given"
);

// - eraseAllButLeading ---------------------------------------------------------

Ref * sysEraseAllButLeading( Ref * pc, class MachineClass * vm ) {
    if ( vm->count < 1 ) {
        throw Ginger::Mishap( "Too few arguments" );
    }
    Ref count = vm->fastPop();
    if ( not IsSmall( count ) ) {
        throw Ginger::Mishap( "Last argument must be an integer" ).culprit( "Last arg", refToShowString( count ) );
    }
    long keep = SmallToLong( count );
    if ( keep < 0 ) {
        throw Ginger::Mishap( "Argument must be >= 0" ).culprit( "Leaving count", keep );
    }
    long k = vm->count - 1;
    if ( k < keep ) {
        throw Ginger::Mishap( "Trying to leave more values than available" ).culprit( "Leaving", keep ).culprit( "Available", k );
    } else {
        vm->fastDrop( k - keep );
    }
    return pc;
}
SysInfo infoEraseAllButLeading(
    SysNames( "eraseAllButLeading" ),
    Arity( 1, true ),
    Arity( 0, true ),
    sysEraseAllButLeading,
    "Given args (arg1, ..., arg_n, N ) discards the last (n-N) arguments to return (arg1, ..., arg_N)"
);

// - eraseTrailing ---------------------------------------------------------------

Ref * sysEraseTrailing( Ref * pc, class MachineClass * vm ) {
    if ( vm->count < 1 ) {
        throw Ginger::Mishap( "No arguments" );
    }
    Ref count = vm->fastPop();
    if ( not IsSmall( count ) ) {
        throw Ginger::Mishap( "Last argument must be an integer" ).culprit( "Last arg", refToShowString( count ) );
    }
    long n = SmallToLong( count );
    if ( n < 0 ) {
        throw Ginger::Mishap( "Argument must be >= 0" ).culprit( "Erasure count", n );
    }
    long k = vm->count - 1;
    if ( k < n ) {
        throw Ginger::Mishap( "Trying to erase more values than available" ).culprit( "Erasing", n ).culprit( "Available", k );
    } else {
        vm->fastDrop( n );
    }
    return pc;
}
SysInfo infoEraseTrailing(
    SysNames( "eraseTrailing" ),
    Arity( 1, true ),
    Arity( 0, true ),
    sysEraseTrailing,
    "Given args (arg1, ..., arg_n, N ) discards the last N arguments, excluding N, that it is given"
);

// - eraseAllButTrailing ---------------------------------------------------------------

Ref * sysEraseAllButTrailing( Ref * pc, class MachineClass * vm ) {
    if ( vm->count < 1 ) {
        throw Ginger::Mishap( "Too few arguments" );
    }
    Ref count = vm->fastPop();
    if ( not IsSmall( count ) ) {
        throw Ginger::Mishap( "Last argument must be an integer" ).culprit( "Last arg", refToShowString( count ) );
    }
    long n1 = SmallToLong( count );
    if ( n1 < 0 ) {
        throw Ginger::Mishap( "Argument must be >= 0" ).culprit( "Trailing count", n1 );
    }
    long k = vm->count - 1;
    long n = k - n1;
    if ( k <= n ) {
        if ( k == n ) {
            vm->fastDrop( n );
        } else {
            throw Ginger::Mishap( "Trying to erase more values than available" ).culprit( "Erasing", n ).culprit( "Available", k );
        }
    } else {
        long k1 = k - 1;
        long num_swaps = k - n;
        for ( int i = 0; i < num_swaps; i++ ) {
            vm->fastPeek( k1 - i ) = vm->fastPeek( k1 - n - i );
        }
        vm->fastDrop( n );
    }
    return pc;
}
SysInfo infoEraseAllButTrailing(
    SysNames( "eraseAllButTrailing" ),
    Arity( 1, true ),
    Arity( 0, true ),
    sysEraseAllButTrailing,
    "Given args (arg1, ..., arg_n, N ) discards the first (n-N) arguments returning (arg_(n-N+1), ..., arg_n)"
);

// - eraseFirst ----------------------------------------------------------------

Ref * sysEraseFirst( Ref * pc, class MachineClass * vm ) {
    if ( vm->count < 1 ) {
        throw Ginger::Mishap( "Too few arguments" );
    }
    long k = vm->count - 1;
    if ( k == 1 ) {
        vm->fastDrop( 1 );
    } else {
        long k1 = k - 1;
        for ( int i = 0; i < k1; i++ ) {
            vm->fastPeek( k1 - i ) = vm->fastPeek( k1 - 1 - i );
        }
        vm->fastDrop( 1 );
    }
    return pc;
}
SysInfo infoEraseFirst(
    SysNames( "eraseFirst" ),
    Arity( 1, true ),
    Arity( 0, true ),
    sysEraseFirst,
    "Given args (arg1, ..., arg_n) discards the arg1 and returns (arg2, ..., arg_n)"
);

// - eraseAllButFirst ----------------------------------------------------------

Ref * sysEraseAllButFirst( Ref * pc, class MachineClass * vm ) {
    if ( vm->count < 1 ) {
        throw Ginger::Mishap( "Too few arguments" );
    }
    vm->fastDrop( vm->count - 1 );
    return pc;
}
SysInfo infoEraseAllButFirst(
    SysNames( "eraseAllButFirst" ),
    Arity( 1, true ),
    Arity( 1 ),
    sysEraseAllButFirst,
    "Given args (arg1, ..., arg_n) returns (arg1)"
);

// - eraseLast ----------------------------------------------------------------

Ref * sysEraseLast( Ref * pc, class MachineClass * vm ) {
    if ( vm->count < 1 ) {
        throw Ginger::Mishap( "Too few arguments" );
    }
    vm->fastDrop( 1 );
    return pc;
}
SysInfo infoEraseLast(
    SysNames( "eraseLast" ),
    Arity( 1, true ),
    Arity( 0, true ),
    sysEraseLast,
    "Given args (arg1, ..., arg_n) discards arg_n and returns (arg1, ..., arg_(n-1))"
);

// - eraseAllButLast ----------------------------------------------------------------

Ref * sysEraseAllButLast( Ref * pc, class MachineClass * vm ) {
    if ( vm->count < 1 ) {
        throw Ginger::Mishap( "Too few arguments" );
    }
    Ref r = vm->fastPeek();
    vm->fastDrop( vm->count );
    vm->fastPush( r );              //  Safe because vm->count was >= 1.
    return pc;
}
SysInfo infoEraseAllButLast(
    SysNames( "eraseAllButLast" ),
    Arity( 1, true ),
    Arity( 1 ),
    sysEraseAllButLast,
    "Given args (arg1, ..., arg_n) returns (arg_n)"
);

// - dupAll --------------------------------------------------------------------

Ref * sysDupAll( Ref * pc, class MachineClass * vm ) {
    vm->checkStackRoom( vm->count );
    long n = vm->count - 1;
    for ( int i = 0; i < vm->count; i++ ) {
        vm->fastPush( vm->fastPeek( n ) );
    }
    return pc;
}
SysInfo infoDupAll(
    SysNames( "dupAll" ),
    Arity( 0, true ),
    Arity( 0, true ),
    sysDupAll,
    "Given args (arg1, ..., arg_n) returns (arg1, ..., arg_n, arg1, ..., arg_n)"
);


// - dupFirst ------------------------------------------------------------------

Ref * sysDupFirst( Ref * pc, class MachineClass * vm ) {
    vm->checkStackRoom( vm->count );
    if ( vm->count < 1 ) {
        throw Ginger::Mishap( "Too few arguments" );
    }
    vm->fastPush( vm->fastPeek( vm->count - 1 ) );
    return pc;
}
SysInfo infoDupFirst(
    SysNames( "dupFirst" ),
    Arity( 1, true ),
    Arity( 2, true ),
    sysDupFirst,
    "Given args (arg1, ..., arg_n) returns (arg1, ..., arg_n, arg1), n >= 1"
);

// - dupAllButFirst ------------------------------------------------------------------

Ref * sysDupAllButFirst( Ref * pc, class MachineClass * vm ) {
    vm->checkStackRoom( vm->count );
    if ( vm->count < 1 ) {
        throw Ginger::Mishap( "Too few arguments" );
    }
    long n = vm->count - 1;
    for ( int i = 0; i < n; i++ ) {
        vm->fastPush( vm->fastPeek( n - 1 ) );
    }
    return pc;
}
SysInfo infoDupAllButFirst(
    SysNames( "dupAllButFirst" ),
    Arity( 1, true ),
    Arity( 1, true ),
    sysDupAllButFirst,
    "Given args (arg1, ..., arg_n) returns (arg1, ..., arg_n, arg2, ..., arg_n ), n >= 1"
);

// - dupLeading ----------------------------------------------------------------

Ref * sysDupLeading( Ref * pc, class MachineClass * vm ) {
    vm->checkStackRoom( vm->count );    // slight over-estimate.
    if ( vm->count < 1 ) {
        throw Ginger::Mishap( "Too few arguments" );
    }
    Ref rK = vm->fastPop();
    if ( not IsSmall( rK ) ) {
        throw Ginger::Mishap( "Small integer needed" ).culprit( "Duplication count", refToShowString( rK ) );
    }
    long K = SmallToLong( rK );
    long n = vm->count - 1;
    if ( K < 0 ) {
        throw Ginger::Mishap( "Count must be positive" ).culprit( "Duplication count", K );
    } else if ( K > n ) {
        throw Ginger::Mishap( "Count more than available elements" ).culprit( "Duplication count", K ).culprit( "Available", n );
    }
    long n1 = n - 1;
    for ( int i = 0; i < K; i++ ) {
        vm->fastPush( vm->fastPeek( n1 ) );
    }
    return pc;
}
SysInfo infoDupLeading(
    SysNames( "dupLeading" ),
    Arity( 1, true ),
    Arity( 0, true ),
    sysDupLeading,
    "Given args (arg1, ..., arg_n, K) returns (arg1, ..., arg_n, arg1, ..., arg_K ), K <= n"
);

// - dupAllButLeading ----------------------------------------------------------------

Ref * sysDupAllButLeading( Ref * pc, class MachineClass * vm ) {
    vm->checkStackRoom( vm->count );    // slight over-estimate.
    if ( vm->count < 1 ) {
        throw Ginger::Mishap( "Too few arguments" );
    }
    Ref rK = vm->fastPop();
    if ( not IsSmall( rK ) ) {
        throw Ginger::Mishap( "Small integer needed" ).culprit( "Count", refToShowString( rK ) );
    }
    long K = SmallToLong( rK );
    long n = vm->count - 1;
    if ( K < 0 ) {
        throw Ginger::Mishap( "Count must be positive" ).culprit( "Duplication count", K );
    } else if ( K > n ) {
        throw Ginger::Mishap( "Count more than available elements" ).culprit( "Duplication count", K ).culprit( "Available", n );
    }
    long d = n - K - 1;
    for ( int i = K; i < n; i++ ) {
        vm->fastPush( vm->fastPeek( d ) );
    }
    return pc;
}
SysInfo infoDupAllButLeading(
    SysNames( "dupAllButLeading" ),
    Arity( 1, true ),
    Arity( 0, true ),
    sysDupAllButLeading,
    "Given args (arg1, ..., arg_n, K) returns (arg_1, ..., arg_n, arg(K+1), ..., arg_n ), K <= n"
);

// - dupLast -------------------------------------------------------------------

Ref * sysDupLast( Ref * pc, class MachineClass * vm ) {
    vm->checkStackRoom( vm->count );
    if ( vm->count < 1 ) {
        throw Ginger::Mishap( "Too few arguments" );
    }
    vm->fastPush( vm->fastPeek( 0 ) );
    return pc;
}
SysInfo infoDupLast(
    SysNames( "dupLast" ),
    Arity( 1, true ),
    Arity( 2, true ),
    sysDupLast,
    "Given args (arg1, ..., arg_n) returns (arg1, ..., arg_n, arg_n), n >= 1"
);


// - dupAllButLast -------------------------------------------------------------------

Ref * sysDupAllButLast( Ref * pc, class MachineClass * vm ) {
    vm->checkStackRoom( vm->count );
    if ( vm->count < 1 ) {
        throw Ginger::Mishap( "Too few arguments" );
    }
    long n = vm->count - 1;
    for ( int i = 0; i < n; i++ ) {
        vm->fastPush( vm->fastPeek( n ) );
    }
    return pc;
}
SysInfo infoDupAllButLast(
    SysNames( "dupAllButLast" ),
    Arity( 1, true ),
    Arity( 1, true ),
    sysDupAllButLast,
    "Given args (arg1, ..., arg_n) returns (arg1, ..., arg_n, arg1, ..., arg_(n-1)), n >= 1"
);

// - dupTrailing -------------------------------------------------------------------

Ref * sysDupTrailing( Ref * pc, class MachineClass * vm ) {
    vm->checkStackRoom( vm->count );    // slight over-estimate.
    if ( vm->count < 1 ) {
        throw Ginger::Mishap( "Too few arguments" );
    }
    Ref rK = vm->fastPop();
    if ( not IsSmall( rK ) ) {
        throw Ginger::Mishap( "Small integer needed" ).culprit( "Count", refToShowString( rK ) );
    }
    long K = SmallToLong( rK );
    long n = vm->count - 1;
    if ( K < 0 ) {
        throw Ginger::Mishap( "Count must be positive" ).culprit( "Count", K );
    } else if ( K > n ) {
        throw Ginger::Mishap( "Count more than available elements" ).culprit( "Duplication count", K ).culprit( "Available", n );
    }
    long d = K - 1;
    for ( int i = 0; i < K; i++ ) {
        vm->fastPush( vm->fastPeek( d ) );
    }
    return pc;
}
SysInfo infoDupTrailing(
    SysNames( "dupTrailing" ),
    Arity( 1, true ),
    Arity( 0, true ),
    sysDupTrailing,
    "Given args (arg1, ..., arg_n, K) returns (arg1, ..., arg_n, arg_(n-K+1), ..., arg_n), K <= n"
);


// - dupAllButTrailing -------------------------------------------------------------------

Ref * sysDupAllButTrailing( Ref * pc, class MachineClass * vm ) {
    vm->checkStackRoom( vm->count );    // slight over-estimate.
    if ( vm->count < 1 ) {
        throw Ginger::Mishap( "Too few arguments" );
    }
    Ref rK = vm->fastPop();
    if ( not IsSmall( rK ) ) {
        throw Ginger::Mishap( "Small integer needed" ).culprit( "Count", refToShowString( rK ) );
    }
    long K = SmallToLong( rK );
    long n = vm->count - 1;
    if ( K < 0 ) {
        throw Ginger::Mishap( "Count must be positive" ).culprit( "Count", K );
    } else if ( K > n ) {
        throw Ginger::Mishap( "Count more than available elements" ).culprit( "Duplication count", K ).culprit( "Available", n );
    }
    long d = n - 1;
    for ( int i = K; i < n; i++ ) {
        vm->fastPush( vm->fastPeek( d ) );
    }
    return pc;
}
SysInfo infoDupAllButTrailing(
    SysNames( "dupAllButTrailing" ),
    Arity( 1, true ),
    Arity( 0, true ),
    sysDupAllButTrailing,
    "Given args (arg1, ..., arg_n, K) returns (arg1, ..., arg_n, arg_1, ..., arg_(n-K)), K <= n"
);

// - dupAllButTrailing -------------------------------------------------------------------

Ref * sysIdentfn( Ref * pc, class MachineClass * vm ) {
    return pc;
}
SysInfo infoIdentfn(
    SysNames( "identfn" ),
    Arity( 0, true ),
    Arity( 0, true ),
    sysIdentfn,
    "Does nothing!"
);

