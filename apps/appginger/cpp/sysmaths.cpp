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

#include <iostream>
#include <cmath>

#include "cell.hpp"
#include "mishap.hpp"
#include "machine.hpp"
#include "sys.hpp"  
#include "sysmaths.hpp"
#include "heap.hpp"
#include "sysdouble.hpp"
#include "numbers.hpp"

#include "overflow.inc"

namespace Ginger {
using namespace std;

static void need_numbers( Cell lhs, Cell rhs ) {
    throw Mishap( "Numbers needed" ).culprit( "LHS", lhs.toShowString() ).culprit( "RHS", rhs.toShowString() );  
}

bool canFitInSmall( const long n ) { 
    return Numbers::MIN_SMALL <= n && n <= Numbers::MAX_SMALL; 
}

/**
 * Compares two references x & y that must be numbers. It returns true if
 *  lt AND x < y OR
 *  eq AND x = y OR
 *  gt AND x > y
 */
bool sysCompareNumbers( Ref rx, Ref ry, const bool lt, const bool eq, const bool gt ) {
    Cell cx( rx );
    Cell cy( ry );
    try {
        if ( cx.isSmall() ) {
            if ( cy.isSmall() ) {
                return (
                    ( lt and ( ToLong( rx ) < ToLong( ry ) ) ) or 
                    ( gt and ( ToLong( rx ) > ToLong( ry ) ) ) or 
                    ( eq and ( ToLong( rx ) == ToLong( ry ) ) )
                );
            } else if ( cy.isDoubleObject() ) {
                gngdouble_t dx = static_cast< gngdouble_t >( cx.getLong() );
                gngdouble_t dy = cy.asDoubleObject().getDouble();
                return ( lt and dx < dy ) or ( gt and dx > dy ) or ( eq and dx == dy );
            } else if ( cy.isBigIntObject() ) {
                BigIntExternal bx( cx.getLong() );
                BigIntExternal * by = cy.asBigIntObject().getBigIntExternal();
                return (
                    ( lt and bx.lt( *by ) ) or 
                    ( gt and by->lt( bx ) )
                );
            } else {
                throw Mishap( "CMP" );
            }
        } else if ( cx.isDoubleObject() ) {
            gngdouble_t dx = cx.asDoubleObject().getDouble();
            gngdouble_t dy;
            if ( cy.isSmall() ) {
                dy = static_cast< gngdouble_t >( cy.getLong() );
            } else if ( cy.isDoubleObject() ) {
                dy = cy.asDoubleObject().getDouble();
            } else if ( cy.isBigIntObject() ) {
                dy = cy.asBigIntObject().getBigIntExternal()->toFloat();
            } else {
                throw Mishap( "CMP" );                
            }
            return ( lt and dx < dy ) or ( gt and dx > dy ) or ( eq and dx == dy );
        } else if ( cx.isBigIntObject() ) {
            BigIntExternal * bx = cx.asBigIntObject().getBigIntExternal();
            if ( cy.isSmall() ) {
                BigIntExternal by( cy.getLong() );
                return ( lt and bx->lt( by ) ) or ( gt and by.lt( *bx ) );
            } else if ( cy.isDoubleObject() ) {
                gngdouble_t dx = cx.asBigIntObject().getBigIntExternal()->toFloat();
                gngdouble_t dy = cy.asDoubleObject().getDouble();
                return ( lt and dx < dy ) or ( gt and dx > dy ) or ( eq and dx == dy );
            } else if ( cy.isBigIntObject() ) {
                BigIntExternal * bx = cx.asBigIntObject().getBigIntExternal();
                BigIntExternal * by = cy.asBigIntObject().getBigIntExternal();
                return (
                    ( lt and bx->lt( *by ) ) or 
                    ( gt and by->lt( *bx ) ) or
                    ( eq and bx->eq( *by ) )
                );                
            } else { 
                throw Mishap( "CMP" );
            }
        } else {
            throw Mishap( "CMP" );
        }
    } catch ( Mishap & e ) {
        throw Mishap( "Invalid values for numerical comparison" ).culprit( "First value", cx.toShowString() ).culprit( "Second value", cy.toShowString() );
    }
}

/**
 * Returns a value in the half-open range [0, Y).
 */
Ref * sysFlooredRemainderHelper( Ref * pc, class MachineClass * vm, Ref ry ) {
    Ref rx = vm->fastPeek();
    Cell cx( rx );
    Cell cy( ry );
    try {
        if ( cx.isSmall() ) {
            if ( cy.isSmall() ) {
                if ( cy.getLong() == 0 ) {
                    throw Mishap( "MOD" );
                } else {
                    const long a = ToLong( rx );
                    const bool sa = a >= 0;
                    const long pa = sa ? a : -a;

                    const long b = ToLong( ry );
                    const bool sb = b > 0;
                    const long pb = sb ? b : -b;

                    vm->fastPeek() = sa ? ToRef( pa % pb ) : ToRef( ToLong( ry ) - ( pa % pb ) );
                }
            } else if ( cy.isBigIntObject() ) {
                vm->fastPeek() = rx;
            } else {
                throw Mishap( "MOD" );                
            }
        } else if ( cx.isBigIntObject() ) {
             if ( cy.isSmall() ) {
                if ( cy.getLong() == 0L ) {
                    throw Mishap( "MOD" );
                } else {
                    BigIntExternal modulus( *cx.asBigIntObject().getBigIntExternal() );
                    modulus.flooredRemainderBy( cy.getLong() );
                    vm->fastPeek() = vm->heap().copyBigIntExternal( pc, modulus );
                }
            } else if ( cy.isBigIntObject() ) {
                BigIntExternal modulus( *cx.asBigIntObject().getBigIntExternal() );
                modulus.flooredRemainderBy( *cy.asBigIntObject().getBigIntExternal() );
                vm->fastPeek() = vm->heap().copyBigIntExternal( pc, modulus );
            } else {
                throw Mishap( "MOD" );
            }                    
        } else {
            throw Mishap( "MOD" );
        }
    } catch ( Mishap & e ) {
         throw Mishap( "Bad arguments for mod operation" ).culprit( "First", cx.toShowString() ).culprit( "Second", cy.toShowString() );        
    }
    return pc;
}

Ref * sysFlooredQuotientHelper( Ref * pc, class MachineClass * vm, Ref ry ) {
    Ref rx = vm->fastPeek();
    Cell cx( rx );
    Cell cy( ry );
    try {
        if ( cx.isSmall() ) {
            if ( cy.isSmall() ) {
                if ( cy.getLong() == 0L ) {
                    throw Mishap( "QUO" );
                } else {
                    const long a = ToLong( rx );
                    const bool sa = a >= 0;
                    const long pa = sa ? a : -a;

                    const long b = ToLong( ry );
                    const bool sb = b > 0;
                    const long pb = sb ? b : -b;

                    vm->fastPeek() = LongToSmall( ( sa == sb ) ? pa / pb : ( -1 - pa / pb ) );
                }
            } else if ( cy.isBigIntObject() ) {
                vm->fastPeek() = LongToSmall( 0 );
            } else {
                throw Mishap( "QUO" );
            }
        } else if ( cx.isBigIntObject() ) {
             if ( cy.isSmall() ) {
                if ( cy.getLong() == 0L ) {
                    throw Mishap( "QUO" );
                } else {
                    BigIntExternal quotient( *cx.asBigIntObject().getBigIntExternal() );
                    quotient.flooredQuotientBy( cy.getLong() );
                    vm->fastPeek() = vm->heap().copyBigIntExternal( pc, quotient );
                }
            } else if ( cy.isBigIntObject() ) {
                BigIntExternal quotient( *cx.asBigIntObject().getBigIntExternal() );
                quotient.flooredQuotientBy( *cy.asBigIntObject().getBigIntExternal() );
                vm->fastPeek() = vm->heap().copyBigIntExternal( pc, quotient );
            } else {
                throw Mishap( "QUO" );
            }           
        } else {
            throw Mishap( "QUO" );
        }
    } catch ( Mishap & e ) {
        throw Mishap( "Bad arguments for quotient operation" ).culprit( "First", cx.toShowString() ).culprit( "Second", cy.toShowString() );        
    }
    return pc;
}

Ref * sysDivHelper( Ref * pc, class MachineClass * vm, Ref ry ) {
    Ref rx = vm->fastPeek();
    Cell cx( rx );
    Cell cy( ry );

    try {
        gngdouble_t x, y;
        if ( cx.isSmall() ) {
            x = static_cast< gngdouble_t >( cx.getLong() );
        } else if ( cx.isDoubleObject() ) {
            x = cx.asDoubleObject().getDouble();
        } else if ( cx.isBigIntObject() ) {
            x = cx.asBigIntObject().getBigIntExternal()->toFloat();
        } else {
            throw Mishap( "Bad arguments for / operator" );
        }
        if ( cy.isSmall() ) {
            y = static_cast< gngdouble_t >( cy.getLong() );
        } else if ( cy.isDoubleObject() ) {
            y = cy.asDoubleObject().getDouble();
        } else if ( cy.isBigIntObject() ) {
            y = cy.asBigIntObject().getBigIntExternal()->toFloat();
        } else {
            throw Mishap( "Bad arguments for / operator" );
        }
        vm->fastPeek() = vm->heap().copyDouble( pc, x / y );
    } catch ( Mishap & e ) {
        throw Mishap( "Bad arguments for / operation" ).culprit( "First", cx.toShowString() ).culprit( "Second", cy.toShowString() );        
    }
    return pc;
}


Ref * sysNegHelper( Ref *pc, class MachineClass * vm ) {
    Ref rx = vm->fastPeek();
    Cell cx( rx );
    if ( cx.isSmall() ) {
        //  The negation of x CANNOT overflow because of the tagging scheme.
        //  N.B. This is subtle.
        vm->fastPeek() = ToRef( -(long)rx );
    } else if ( cx.isDoubleObject() ) {
        vm->fastPeek() = vm->heap().copyDouble( pc, -( cx.asDoubleObject().getDouble() ) );
    } else if ( cx.isBigIntObject() ) {
        BigIntExternal negated( cx.asBigIntObject().getBigIntExternal()->neg() );
        vm->fastPeek() = vm->heap().copyBigIntExternal( pc, negated );
    } else {
        throw Mishap( "Bad argument for negation operation" ).culprit( "Value", cx.toShowString() );
    } 
    return pc;
}

Ref * sysMulHelper( Ref *pc, class MachineClass * vm, Ref ry ) {
    Ref rx = vm->fastPeek();
    Cell cx( rx );
    Cell cy( ry );

    try {
        if ( cx.isSmall() ) {
            if ( cy.isSmall() ) {
                gnglong_t hy = (gnglong_t)ry >> 1;  //  Scale down by factor of 2 & strip off low bit.
                gnglong_t hx = (gnglong_t)rx >> 1;  //  As above.
                if ( SignedOverflow::mulOverflowCheck( hx, hy ) ) {
                    BigIntExternal by( cx.getLong() );
                    BigIntExternal product( by.mul( cy.getLong() ) );
                    vm->fastPeek() = vm->heap().copyBigIntExternal( pc, product );
                } else {
                    vm->fastPeek() = ToRef( hx * hy );
                }
            } else if ( cy.isDoubleObject() ) {
                gngdouble_t x, y;
                y = cy.asDoubleObject().getDouble();
                x = static_cast< gngdouble_t >( cx.getLong() );
                vm->fastPeek() = vm->heap().copyDouble( x * y );
            } else if ( cy.isBigIntObject() ) {
                BigIntExternal * by = cy.asBigIntObject().getBigIntExternal();
                BigIntExternal product( by->mul( cx.getLong() ) );
                vm->fastPeek() = vm->heap().copyBigIntExternal( pc, product );                
            } else {
                throw Mishap( "Bad arguments for * operation" );
            }
        } else if ( cx.isDoubleObject() ) {
            gngdouble_t x, y;
            x = cx.asDoubleObject().getDouble();
            if ( cy.isSmall() ) {
                y = static_cast< gngdouble_t >( cy.getLong() );
            } else if ( cy.isDoubleObject() ) {
                y = cy.asDoubleObject().getDouble();
            } else if ( cy.isBigIntObject() ) {
                y = cy.asBigIntObject().getBigIntExternal()->toFloat();
            } else {
                throw Mishap( "Invalid arguments for *" );
            }
            vm->fastPeek() = vm->heap().copyDouble( x * y );
        } else if ( cx.isBigIntObject() ) {
            BigIntExternal * bx = cx.asBigIntObject().getBigIntExternal();
            if ( cy.isSmall() ) {
                const BigIntExternal product( bx->mul( cy.getLong() ) );
                vm->fastPeek() = vm->heap().copyBigIntExternal( pc, product );
            } else if ( cy.isDoubleObject() ) {
                vm->fastPeek() = vm->heap().copyDouble( pc, bx->toFloat() * cy.asDoubleObject().getDouble() );
            } else if ( cy.isBigIntObject() ) {
                BigIntExternal product( *cy.asBigIntObject().getBigIntExternal() );
                product.mulBy( *bx );
                vm->fastPeek() = vm->heap().copyBigIntExternal( pc, product ); 
            } else {
                throw Mishap( "Bad arguments for - operator" );
            }
        } else {
            throw Mishap( "Bad arguments for * operation" );
        }
    } catch ( Mishap & e ) {
        throw Mishap( "Bad arguments for * operation" ).culprit( "First", cx.toShowString() ).culprit( "Second", cy.toShowString() );
    }
    return pc;
}

Ref * sysSubHelper( Ref *pc, class MachineClass * vm, Ref ry ) {
    Ref rx = vm->fastPeek();
    Cell cx( rx );
    Cell cy( ry );
    try {
        if ( cx.isSmall() ) {
            if ( cy.isSmall() ) {
                const long y = reinterpret_cast< long >( ry );
                const long x = reinterpret_cast< long >( rx );
                const long diff = x - y;
                if ( y < 0L ? diff > x : diff <= x ) {
                    vm->fastPeek() = ToRef( diff );
                } else {
                    BigIntExternal diff( cx.getLong() );
                    diff.subBy( cx.getLong() );
                    vm->fastPeek() = vm->heap().copyBigIntExternal( pc, diff );
                }
            } else if ( IsDouble( ry ) ) {
                gngdouble_t x, y;
                y = cy.asDoubleObject().getDouble();
                x = static_cast< gngdouble_t >( cx.getLong() );
                vm->fastPeek() = vm->heap().copyDouble( pc, x - y );
            } else if ( cy.isBigIntObject() ) {
                BigIntExternal * by = cy.asBigIntObject().getBigIntExternal();
                const BigIntExternal diff( by->sub( cx.getLong() ) );
                vm->fastPeek() = vm->heap().copyBigIntExternal( pc, diff );
            } else {
                throw Mishap( "Bad arguments for - operation" );
            }
        } else if ( cx.isDoubleObject() ) {
            gngdouble_t x, y;
            x = cx.asDoubleObject().getDouble();
            if ( cy.isSmall() ) {
                y = static_cast< gngdouble_t >( cy.getLong() );
            } else if ( cy.isDoubleObject() ) {
                y = cy.asDoubleObject().getDouble();
            } else if ( cy.isBigIntObject() ) {
                y = cy.asBigIntObject().getBigIntExternal()->toFloat();
            } else {
                throw Mishap( "Bad arguments for + operator" );
            }
            //std::cout << "two doubles: " << x << ", " << y << std::endl;
            vm->fastPeek() = vm->heap().copyDouble( pc, x - y );
        } else if ( cx.isBigIntObject() ) {
            BigIntExternal * bx = cx.asBigIntObject().getBigIntExternal();
            if ( cy.isSmall() ) {
                const BigIntExternal diff( bx->sub( cy.getLong() ) );
                vm->fastPeek() = vm->heap().copyBigIntExternal( pc, diff );
            } else if ( cy.isDoubleObject() ) {
                vm->fastPeek() = vm->heap().copyDouble( pc, bx->toFloat() - cy.asDoubleObject().getDouble() );
            } else if ( cy.isBigIntObject() ) {
                BigIntExternal * by = cy.asBigIntObject().getBigIntExternal();
                BigIntExternal diff( bx->sub( *by ) );
                vm->fastPeek() = vm->heap().copyBigIntExternal( pc, diff ); 
            } else {
                throw Mishap( "Bad arguments for - operator" );
            }
        } else {
            throw Mishap( "Bad arguments for - operation" );
        }
    } catch ( Mishap & m ) {
        throw Mishap( "Bad arguments for - operator" ).culprit( "First", cx.toShowString() ).culprit( "Second", cy.toShowString() );
    }
    //cerr << "Returning" << endl;
    return pc;
}

Ref * sysAddHelper( Ref * pc, class MachineClass * vm, Ref ry ) {
    Ref rx = vm->fastPeek();
    Cell cx( rx );
    Cell cy( ry );
    try {
        //cerr << "try" << endl;
        if ( cx.isSmall() ) {
            if ( cy.isSmall() ) {
                const long y = reinterpret_cast< long >( ry );
                const long x = reinterpret_cast< long >( rx );
                const long sum = x + y;
                //std::cout << "two smalls: " << x << ", " << y << std::endl;
                if ( x < 0L ? sum < y : sum >= y ) {
                    //std::cout << "x, y, sum, sum >= y, ( x < 0L ? sum <= y : sum >= y ), sum < 0" << std::endl;
                    //std::cout << x << ", " << y << ", " << sum << ", " << ( sum >= y ) << ", " << ( x < 0L ? sum <= y : sum >= y ) << ", " << ( sum < 0L ) << std::endl;
                    //std::cout << "result was small: " << sum << " n.b. " << ( x < 0L ) << ", " << ( sum >= y ) << std::endl;
                    vm->fastPeek() = ToRef( sum );
                } else {
                    BigIntExternal sum( cx.getLong() );
                    sum.addBy( cx.getLong() );
                    vm->fastPeek() = vm->heap().copyBigIntExternal( pc, sum );
                }
            } else if ( cy.isDoubleObject() ) {
                gngdouble_t x, y;
                y = cy.asDoubleObject().getDouble();
                x = static_cast< gngdouble_t >( cx.getLong() );
                vm->fastPeek() = vm->heap().copyDouble( pc, x + y );
            } else if ( cy.isBigIntObject() ) {
                BigIntExternal * by = cy.asBigIntObject().getBigIntExternal();
                const BigIntExternal sum( by->add( cx.getLong() ) );
                vm->fastPeek() = vm->heap().copyBigIntExternal( pc, sum );
            } else {
                throw Mishap( "Bad arguments for + operator" );
            }
        } else if ( cx.isDoubleObject() ) {
            gngdouble_t x, y;
            x = cx.asDoubleObject().getDouble();
            if ( cy.isSmall() ) {
                y = static_cast< gngdouble_t >( cy.getLong() );
            } else if ( cy.isDoubleObject() ) {
                y = cy.asDoubleObject().getDouble();
            } else if ( cy.isBigIntObject() ) {
                y = cy.asBigIntObject().getBigIntExternal()->toFloat();
            } else {
                throw Mishap( "Bad arguments for + operator" );
            }
            //std::cout << "two doubles: " << x << ", " << y << std::endl;
            vm->fastPeek() = vm->heap().copyDouble( pc, x + y );
        } else if ( cx.isBigIntObject() ) {
            //cerr << "bigint" << endl;
            BigIntExternal * bx = cx.asBigIntObject().getBigIntExternal();
            if ( cy.isSmall() ) {
                const BigIntExternal sum( bx->add( cy.getLong() ) );
                vm->fastPeek() = vm->heap().copyBigIntExternal( pc, sum );
            } else if ( cy.isDoubleObject() ) {
                vm->fastPeek() = vm->heap().copyDouble( pc, bx->toFloat() + cy.asDoubleObject().getDouble() );
            } else if ( cy.isBigIntObject() ) {
                //cerr << "bigint2" << endl;
                BigIntExternal * by = cy.asBigIntObject().getBigIntExternal();
                BigIntExternal sum( bx->add( *by ) );
                //cerr << "bigint2 sum" << endl;
                vm->fastPeek() = vm->heap().copyBigIntExternal( pc, sum );
                //cerr << "OK" << endl;
            } else {
                throw Mishap( "Bad arguments for + operator" );
            }
        } else {
            throw Mishap( "Bad arguments for + operator" );
        }
    } catch ( Mishap & m ) {
        throw Mishap( "Bad arguments for + operator" ).culprit( "First", cx.toShowString() ).culprit( "Second", cy.toShowString() );
    }
    //cerr << "Returning" << endl;
    return pc;
}

Ref * sysAdd( Ref * pc, class MachineClass * vm ) {
    //cerr << "sysAdd" << endl;
    if ( vm->count != 2 ) throw Ginger::Mishap( "ArgsMismatch" );
    return sysAddHelper( pc, vm, vm->fastPop() );
}


Ref * sysMax( Ref * pc, class MachineClass * vm ) {
    if ( vm->count != 2 ) throw Ginger::Mishap( "ArgsMismatch" );
    Cell rhs( vm->fastPop() );
    Cell lhs( vm->fastPeek() );

    if ( lhs.isSmall() ) {
        if ( rhs.isSmall() ) {
            const long llhs = lhs.getLong();
            const long lrhs = rhs.getLong();
            vm->fastPeek() = llhs < lrhs ? rhs.asRef() : lhs.asRef();
        } else if ( rhs.isDoubleObject() ) {
            vm->fastPeek() = rhs.asDoubleObject().getDouble().isThisMax( gngdouble_t( lhs.getLong() ) ) ? rhs.asRef() : lhs.asRef();
        } else {
            need_numbers( lhs, rhs );
        }
    } else if ( lhs.isDoubleObject() ) {
        if ( rhs.isSmall() ) {
            vm->fastPeek() = lhs.asDoubleObject().getDouble().isThisMax( gngdouble_t( rhs.getLong() ) ) ? lhs.asRef() : rhs.asRef();
        } else if ( rhs.isDoubleObject() ) {
            vm->fastPeek() = lhs.asDoubleObject().getDouble().isThisMax( rhs.asDoubleObject().getDouble() ) ? lhs.asRef() : rhs.asRef();
        } else {
            need_numbers( lhs, rhs );
        }
    } else {
        need_numbers( lhs, rhs );
    }

    return pc;
}
SysInfo infoMax( 
    SysNames( "max" ), 
    Ginger::Arity( 2, true ), 
    Ginger::Arity( 1 ), 
    sysMax, 
    "Returns the maximum of two numbers."
);

Ref * sysMin( Ref * pc, class MachineClass * vm ) {
    if ( vm->count != 2 ) throw Ginger::Mishap( "ArgsMismatch" );
    Cell rhs( vm->fastPop() );
    Cell lhs( vm->fastPeek() );

    if ( lhs.isSmall() ) {
        if ( rhs.isSmall() ) {
            const long llhs = lhs.getLong();
            const long lrhs = rhs.getLong();
            vm->fastPeek() = llhs < lrhs ? rhs.asRef() : lhs.asRef();
        } else if ( rhs.isDoubleObject() ) {
            vm->fastPeek() = rhs.asDoubleObject().getDouble().isThisMin( gngdouble_t( lhs.getLong() ) ) ? rhs.asRef() : lhs.asRef();
        } else {
            need_numbers( lhs, rhs );
        }
    } else if ( lhs.isDoubleObject() ) {
        if ( rhs.isSmall() ) {
            vm->fastPeek() = lhs.asDoubleObject().getDouble().isThisMin( gngdouble_t( rhs.getLong() ) ) ? lhs.asRef() : rhs.asRef();
        } else if ( rhs.isDoubleObject() ) {
            vm->fastPeek() = lhs.asDoubleObject().getDouble().isThisMin( rhs.asDoubleObject().getDouble() ) ? lhs.asRef() : rhs.asRef();
        } else {
            need_numbers( lhs, rhs );
        }
    } else {
        need_numbers( lhs, rhs );
    }
    return pc;
}
SysInfo infoMin( 
    SysNames( "min" ), 
    Ginger::Arity( 2, true ), 
    Ginger::Arity( 1 ), 
    sysMin, 
    "Returns the minimum of two numbers."
);

Ref * sysMaxAll( Ref * pc, class MachineClass * vm ) {
    if ( vm->count == 0 ) {
        vm->checkStackRoom( 1 );
        vm->fastPush( vm->heap().copyDouble( 0.0/0.0 ) );
    } else {

        Cell sofar( vm->fastPop() );
        for ( int i = 1; i < vm->count; i++ ) {
            Cell next( vm->fastPop() );

            if ( sofar.isSmall() ) {
                if ( next.isSmall() ) {
                    const long llhs = sofar.getLong();
                    const long lrhs = next.getLong();
                    if ( llhs < lrhs ) {
                        sofar = next;
                    }
                } else if ( next.isDoubleObject() ) {
                    if ( next.asDoubleObject().getDouble().isThisMax( gngdouble_t( sofar.getLong() ) ) ) {
                        sofar = next;
                    }
                } else {
                    need_numbers( sofar, next );
                }
            } else if ( sofar.isDoubleObject() ) {
                if ( next.isSmall() ) {
                    if ( not sofar.asDoubleObject().getDouble().isThisMax( gngdouble_t( next.getLong() ) ) ) {
                        sofar = next;
                    }
                } else if ( next.isDoubleObject() ) {
                    if( not sofar.asDoubleObject().getDouble().isThisMax( next.asDoubleObject().getDouble() ) ) {
                        sofar = next;
                    }
                } else {
                    need_numbers( sofar, next );
                }
            } else {
                need_numbers( sofar, next );
            }

        }
        
        vm->fastPush( sofar.asRef() );
    }
    return pc;
}
SysInfo infoMaxAll( 
    SysNames( "maxAll" ), 
    Ginger::Arity( 0, false ), 
    Ginger::Arity( 1 ), 
    sysMaxAll, 
    "Returns the maximum of zero or more arguments."
);

Ref * sysMinAll( Ref * pc, class MachineClass * vm ) {
    if ( vm->count == 0 ) {
        vm->checkStackRoom( 1 );
        vm->fastPush( vm->heap().copyDouble( 0.0/0.0 ) );
    } else {

        Cell sofar( vm->fastPop() );
        for ( int i = 1; i < vm->count; i++ ) {
            Cell next( vm->fastPop() );

            if ( sofar.isSmall() ) {
                if ( next.isSmall() ) {
                    const long llhs = sofar.getLong();
                    const long lrhs = next.getLong();
                    if ( llhs < lrhs ) {
                        sofar = next;
                    }
                } else if ( next.isDoubleObject() ) {
                    if ( next.asDoubleObject().getDouble().isThisMin( gngdouble_t( sofar.getLong() ) ) ) {
                        sofar = next;
                    }
                } else {
                    need_numbers( sofar, next );
                }
            } else if ( sofar.isDoubleObject() ) {
                if ( next.isSmall() ) {
                    if ( not sofar.asDoubleObject().getDouble().isThisMin( gngdouble_t( next.getLong() ) ) ) {
                        sofar = next;
                    }
                } else if ( next.isDoubleObject() ) {
                    if( not sofar.asDoubleObject().getDouble().isThisMin( next.asDoubleObject().getDouble() ) ) {
                        sofar = next;
                    }
                } else {
                    need_numbers( sofar, next );
                }
            } else {
                need_numbers( sofar, next );
            }

        }
        
        vm->fastPush( sofar.asRef() );
    }
    return pc;
}
SysInfo infoMinAll( 
    SysNames( "minAll" ), 
    Ginger::Arity( 0, false ), 
    Ginger::Arity( 1 ), 
    sysMinAll, 
    "Returns the maximum of zero or more arguments."
);

Ref * sysLtGt( Ref * pc, class MachineClass * vm ) {
    if ( vm->count != 2 ) throw Ginger::Mishap( "ArgsMismatch" );
    Cell rhs( vm->fastPop() );
    Cell lhs( vm->fastPeek() );
   
    if ( lhs.isSmall() ) {
        if ( rhs.isSmall() ) {
            const long llhs = lhs.getLong();
            const long lrhs = rhs.getLong();
            vm->fastPeek() = llhs == lrhs ? SYS_FALSE : SYS_TRUE;
        } else if ( rhs.isDoubleObject() ) {
            vm->fastPeek() = rhs.asDoubleObject().getDouble().isLtGt( gngdouble_t( lhs.getLong() ) ) ? SYS_TRUE : SYS_FALSE;
        } else {
            need_numbers( lhs, rhs );
        }
    } else if ( lhs.isDoubleObject() ) {
        if ( rhs.isSmall() ) {
            vm->fastPeek() = lhs.asDoubleObject().getDouble().isLtGt( gngdouble_t( rhs.getLong() ) ) ? SYS_TRUE : SYS_FALSE;
        } else if ( rhs.isDoubleObject() ) {
            vm->fastPeek() = lhs.asDoubleObject().getDouble().isLtGt( rhs.asDoubleObject().getDouble() ) ? SYS_TRUE : SYS_FALSE;
        } else {
            need_numbers( lhs, rhs );
        }
    } else {
        need_numbers( lhs, rhs );
    }

    return pc;
}
SysInfo infoLtGt( 
    SysNames( "<>" ), 
    Ginger::Arity( 2 ), 
    Ginger::Arity( 1 ), 
    sysLtGt, 
    "Returns true if the two arguments are either less than or greater than one another."
);

Ref * sysNotLtGt( Ref * pc, class MachineClass * vm ) {
    pc = sysLtGt( pc, vm );
    vm->fastPeek() = vm->fastPeek() == SYS_FALSE ? SYS_TRUE : SYS_FALSE;
    return pc;
}
SysInfo infoNotLtGt( 
    SysNames( "!<>" ), 
    Ginger::Arity( 2 ), 
    Ginger::Arity( 1 ), 
    sysNotLtGt, 
    "Returns true if the two arguments are neither less than nor greater than each other."
);



Ref * sysLtEGt( Ref * pc, class MachineClass * vm ) {
    if ( vm->count != 2 ) throw Ginger::Mishap( "ArgsMismatch" );
    Cell rhs( vm->fastPop() );
    Cell lhs( vm->fastPeek() );
   
    if ( lhs.isSmall() ) {
        if ( rhs.isSmall() ) {
            vm->fastPeek() = SYS_TRUE;
        } else if ( rhs.isDoubleObject() ) {
            vm->fastPeek() = rhs.asDoubleObject().getDouble().isLtEGt( gngdouble_t( lhs.getLong() ) ) ? SYS_TRUE : SYS_FALSE;
        } else {
            need_numbers( lhs, rhs );
        }
    } else if ( lhs.isDoubleObject() ) {
        if ( rhs.isSmall() ) {
            vm->fastPeek() = lhs.asDoubleObject().getDouble().isLtEGt( gngdouble_t( rhs.getLong() ) ) ? SYS_TRUE : SYS_FALSE;
        } else if ( rhs.isDoubleObject() ) {
            vm->fastPeek() = lhs.asDoubleObject().getDouble().isLtEGt( rhs.asDoubleObject().getDouble() ) ? SYS_TRUE : SYS_FALSE;
        } else {
            need_numbers( lhs, rhs );
        }
    } else {
        need_numbers( lhs, rhs );
    }

    return pc;
}
SysInfo infoLtEGt( 
    SysNames( "<=>" ), 
    Ginger::Arity( 2 ), 
    Ginger::Arity( 1 ), 
    sysLtEGt, 
    "Returns true if the two arguments are either less than or greater than one another."
);

Ref * sysNotLtEGt( Ref * pc, class MachineClass * vm ) {
    pc = sysLtEGt( pc, vm );
    vm->fastPeek() = vm->fastPeek() == SYS_FALSE ? SYS_TRUE : SYS_FALSE;
    return pc;
}
SysInfo infoNotLtEGt( 
    SysNames( "!<=>" ), 
    Ginger::Arity( 2 ), 
    Ginger::Arity( 1 ), 
    sysNotLtEGt, 
    "Returns true if the two arguments are neither less than, equal to or greate than each other."
);

Ref * sysNotLt( Ref * pc, class MachineClass * vm ) {
    if ( vm->count != 2 ) throw Ginger::Mishap( "ArgsMismatch" );
    Cell rhs( vm->fastPop() );
    Cell lhs( vm->fastPeek() );
   
    if ( lhs.isSmall() ) {
        if ( rhs.isSmall() ) {
            const long llhs = lhs.getLong();
            const long lrhs = rhs.getLong();
            vm->fastPeek() = llhs < lrhs ? SYS_FALSE : SYS_TRUE;
        } else if ( rhs.isDoubleObject() ) {
            vm->fastPeek() = gngdouble_t( lhs.getLong() ) < rhs.asDoubleObject().getDouble() ? SYS_FALSE : SYS_TRUE;
        } else {
            need_numbers( lhs, rhs );
        }
    } else if ( lhs.isDoubleObject() ) {
        if ( rhs.isSmall() ) {
            vm->fastPeek() = lhs.asDoubleObject().getDouble() < gngdouble_t( rhs.getLong() ) ? SYS_FALSE : SYS_TRUE;
        } else if ( rhs.isDoubleObject() ) {
            vm->fastPeek() = lhs.asDoubleObject().getDouble() < rhs.asDoubleObject().getDouble() ? SYS_FALSE : SYS_TRUE;
        } else {
            need_numbers( lhs, rhs );
        }
    } else {
        need_numbers( lhs, rhs );
    }

    return pc;
}
SysInfo infoNotLt( 
    SysNames( "!<" ), 
    Ginger::Arity( 2 ), 
    Ginger::Arity( 1 ), 
    sysNotLt, 
    "Returns false if the first argument is less than the second, else true."
);

Ref * sysNotLtE( Ref * pc, class MachineClass * vm ) {
    if ( vm->count != 2 ) throw Ginger::Mishap( "ArgsMismatch" );
    Cell rhs( vm->fastPop() );
    Cell lhs( vm->fastPeek() );
   
    if ( lhs.isSmall() ) {
        if ( rhs.isSmall() ) {
            const long llhs = lhs.getLong();
            const long lrhs = rhs.getLong();
            vm->fastPeek() = llhs <= lrhs ? SYS_FALSE : SYS_TRUE;
        } else if ( rhs.isDoubleObject() ) {
            vm->fastPeek() = gngdouble_t( lhs.getLong() ) <= rhs.asDoubleObject().getDouble() ? SYS_FALSE : SYS_TRUE;
        } else {
            need_numbers( lhs, rhs );
        }
    } else if ( lhs.isDoubleObject() ) {
        if ( rhs.isSmall() ) {
            vm->fastPeek() = lhs.asDoubleObject().getDouble() <= gngdouble_t( rhs.getLong() ) ? SYS_FALSE : SYS_TRUE;
        } else if ( rhs.isDoubleObject() ) {
            vm->fastPeek() = lhs.asDoubleObject().getDouble() <= rhs.asDoubleObject().getDouble() ? SYS_FALSE : SYS_TRUE;
        } else {
            need_numbers( lhs, rhs );
        }
    } else {
        need_numbers( lhs, rhs );
    }

    return pc;
}
SysInfo infoNotLtE( 
    SysNames( "!<=" ), 
    Ginger::Arity( 2 ), 
    Ginger::Arity( 1 ), 
    sysNotLtE, 
    "Returns false if the first argument is less than or equal to the second, else true."
);

Ref * sysNotGt( Ref * pc, class MachineClass * vm ) {
    if ( vm->count != 2 ) throw Ginger::Mishap( "ArgsMismatch" );
    Cell rhs( vm->fastPop() );
    Cell lhs( vm->fastPeek() );
   
    if ( lhs.isSmall() ) {
        if ( rhs.isSmall() ) {
            const long llhs = lhs.getLong();
            const long lrhs = rhs.getLong();
            vm->fastPeek() = llhs > lrhs ? SYS_FALSE : SYS_TRUE;
        } else if ( rhs.isDoubleObject() ) {
            vm->fastPeek() = gngdouble_t( lhs.getLong() ) > rhs.asDoubleObject().getDouble() ? SYS_FALSE : SYS_TRUE;
        } else {
            need_numbers( lhs, rhs );
        }
    } else if ( lhs.isDoubleObject() ) {
        if ( rhs.isSmall() ) {
            vm->fastPeek() = lhs.asDoubleObject().getDouble() > gngdouble_t( rhs.getLong() ) ? SYS_FALSE : SYS_TRUE;
        } else if ( rhs.isDoubleObject() ) {
            vm->fastPeek() = lhs.asDoubleObject().getDouble() > rhs.asDoubleObject().getDouble() ? SYS_FALSE : SYS_TRUE;
        } else {
            need_numbers( lhs, rhs );
        }
    } else {
        need_numbers( lhs, rhs );
    }

    return pc;
}
SysInfo infoNotGt( 
    SysNames( "!>" ), 
    Ginger::Arity( 2 ), 
    Ginger::Arity( 1 ), 
    sysNotGt, 
    "Returns false if the first argument is greater than the second, else true."
);

Ref * sysNotGtE( Ref * pc, class MachineClass * vm ) {
    if ( vm->count != 2 ) throw Ginger::Mishap( "ArgsMismatch" );
    Cell rhs( vm->fastPop() );
    Cell lhs( vm->fastPeek() );
   
    if ( lhs.isSmall() ) {
        if ( rhs.isSmall() ) {
            const long llhs = lhs.getLong();
            const long lrhs = rhs.getLong();
            vm->fastPeek() = llhs >= lrhs ? SYS_FALSE : SYS_TRUE;
        } else if ( rhs.isDoubleObject() ) {
            vm->fastPeek() = gngdouble_t( lhs.getLong() ) >= rhs.asDoubleObject().getDouble() ? SYS_FALSE : SYS_TRUE;
        } else {
            need_numbers( lhs, rhs );
        }
    } else if ( lhs.isDoubleObject() ) {
        if ( rhs.isSmall() ) {
            vm->fastPeek() = lhs.asDoubleObject().getDouble() >= gngdouble_t( rhs.getLong() ) ? SYS_FALSE : SYS_TRUE;
        } else if ( rhs.isDoubleObject() ) {
            vm->fastPeek() = lhs.asDoubleObject().getDouble() >= rhs.asDoubleObject().getDouble() ? SYS_FALSE : SYS_TRUE;
        } else {
            need_numbers( lhs, rhs );
        }
    } else {
        need_numbers( lhs, rhs );
    }

    return pc;
}
SysInfo infoNotGtE( 
    SysNames( "!>=" ), 
    Ginger::Arity( 2 ), 
    Ginger::Arity( 1 ), 
    sysNotGtE, 
    "Returns false if the first argument is greater than or equal to the second, else true."
);

Ref * sysSin( Ref * pc, class MachineClass * vm ) {
    if ( vm->count != 1 ) throw Ginger::Mishap( "ArgsMismatch" );
    Cell angle( vm->fastPeek() );
    if ( angle.isSmall() ) {
        vm->fastPeek() = vm->heap().copyDouble( pc, TransDouble( static_cast< double >( angle.getLong() ) ).sin().asDouble() );
    } else if ( angle.isDoubleObject() ) {
        vm->fastPeek() = vm->heap().copyDouble( pc, angle.asDoubleObject().getDouble().sin().asDouble() );
    } else {
        throw Ginger::Mishap( "Number needed" ).culprit( "Value", angle.toShowString() );
    }
    return pc;
}
SysInfo infoSin( 
    SysNames( "sin" ), 
    Ginger::Arity( 1 ), 
    Ginger::Arity( 1 ), 
    sysSin, 
    "Returns sine of an angle in radians."
);

Ref * sysCos( Ref * pc, class MachineClass * vm ) {
    if ( vm->count != 1 ) throw Ginger::Mishap( "ArgsMismatch" );
    Cell angle( vm->fastPeek() );
    if ( angle.isSmall() ) {
        vm->fastPeek() = vm->heap().copyDouble( pc, TransDouble( static_cast< double >( angle.getLong() ) ).cos().asDouble() );
    } else if ( angle.isDoubleObject() ) {
        vm->fastPeek() = vm->heap().copyDouble( pc, angle.asDoubleObject().getDouble().cos().asDouble() );
    } else {
        throw Ginger::Mishap( "Number needed" ).culprit( "Value", angle.toShowString() );
    }
    return pc;
}
SysInfo infoCos( 
    SysNames( "cos" ), 
    Ginger::Arity( 1 ), 
    Ginger::Arity( 1 ), 
    sysCos, 
    "Returns cosine of an angle in radians."
);

Ref * sysTan( Ref * pc, class MachineClass * vm ) {
    if ( vm->count != 1 ) throw Ginger::Mishap( "ArgsMismatch" );
    Cell angle( vm->fastPeek() );
    if ( angle.isSmall() ) {
        vm->fastPeek() = vm->heap().copyDouble( pc, TransDouble( static_cast< double >( angle.getLong() ) ).tan().asDouble() );
    } else if ( angle.isDoubleObject() ) {
        vm->fastPeek() = vm->heap().copyDouble( pc, angle.asDoubleObject().getDouble().tan().asDouble() );
    } else {
        throw Ginger::Mishap( "Number needed" ).culprit( "Value", angle.toShowString() );
    }
    return pc;
}
SysInfo infoTan( 
    SysNames( "tan" ), 
    Ginger::Arity( 1 ), 
    Ginger::Arity( 1 ), 
    sysTan, 
    "Returns the tangent of an angle in radians."
);

Ref * sysExp( Ref * pc, class MachineClass * vm ) {
    if ( vm->count != 1 ) throw Ginger::Mishap( "ArgsMismatch" );
    Cell value( vm->fastPeek() );
    if ( value.isSmall() ) {
        vm->fastPeek() = vm->heap().copyDouble( pc, TransDouble( static_cast< double >( value.getLong() ) ).exp().asDouble() );
    } else if ( value.isDoubleObject() ) {
        vm->fastPeek() = vm->heap().copyDouble( pc, value.asDoubleObject().getDouble().exp().asDouble() );
    } else {
        throw Ginger::Mishap( "Number needed" ).culprit( "Value", value.toShowString() );
    }
    return pc;
}
SysInfo infoExp( 
    SysNames( "exp" ), 
    Ginger::Arity( 1 ), 
    Ginger::Arity( 1 ), 
    sysExp, 
    "Returns the exponential of a real value (e**x)."
);

} // namespace Ginger