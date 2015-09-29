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
            } else if ( cy.isRationalObject() ) {
                RationalExternal qx( cx.getLong() );
                RationalExternal * qy = cy.asRationalObject().getRationalExternal();
                return (
                    ( lt and qx.lt( *qy ) ) or 
                    ( gt and qy->lt( qx ) )
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
            } else if ( cy.isRationalObject() ) {
                dy = cy.asRationalObject().getRationalExternal()->toFloat();
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
                BigIntExternal * by = cy.asBigIntObject().getBigIntExternal();
                return (
                    ( lt and bx->lt( *by ) ) or 
                    ( gt and by->lt( *bx ) ) or
                    ( eq and bx->eq( *by ) )
                );
            } else if ( cy.isRationalObject() ) {
                RationalExternal * qy = cy.asRationalObject().getRationalExternal();
                RationalExternal qx( *bx );
                return (
                    ( lt and qx.lt( *qy ) ) or 
                    ( gt and qy->lt( qx ) ) or
                    ( eq and qx.eq( *qy ) )
                );
            } else { 
                throw Mishap( "CMP" );
            }
        } else if ( cx.isRationalObject() ) {
            RationalExternal * qx = cx.asRationalObject().getRationalExternal();
            if ( cy.isSmall() ) {
                RationalExternal qy( cy.getLong() );
                return (
                    ( lt and qx->lt( qy ) ) or 
                    ( gt and qy.lt( *qx ) )
                );                
            } else if ( cy.isDoubleObject() ) {
                gngdouble_t dx = qx->toFloat();
                gngdouble_t dy = cy.asDoubleObject().getDouble();
                return ( lt and dx < dy ) or ( gt and dx > dy ) or ( eq and dx == dy );
            } else if ( cy.isBigIntObject() ) {
                RationalExternal qy( *cy.asBigIntObject().getBigIntExternal() );
                return (
                    ( lt and qx->lt( qy ) ) or 
                    ( gt and qy.lt( *qx ) ) or
                    ( eq and qx->eq( qy ) )
                );
            } else if ( cy.isRationalObject() ) {
                RationalExternal * qy = cy.asRationalObject().getRationalExternal();
                return (
                    ( lt and qx->lt( *qy ) ) or 
                    ( gt and qy->lt( *qx ) ) or
                    ( eq and qx->eq( *qy ) )
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
Ref * sysModHelper( Ref * pc, class MachineClass * vm, Ref ry ) {
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
                    const long b = ToLong( ry );
                    vm->fastPeek() = ToRef( a % b );
                }
            } else if ( cy.isBigIntObject() ) {
                if ( cy.isZero() ) {
                    throw Mishap( "MOD" );
                } else {
                    BigIntExternal * by = cy.asBigIntObject().getBigIntExternal();
                    if ( 
                        ( cx.isPositive() and by->isPositive() ) or 
                        ( cx.isNegative() and by->isNegative() )
                    ) {
                        vm->fastPeek() = rx;   
                    } else {
                        BigIntExternal r( *by );
                        r.addBy( cx.getLong() );
                        vm->fastPeek() = vm->heap().copyBigIntExternal( pc, r );
                    }
                }
            } else {
                throw Mishap( "MOD" );                
            }
        } else if ( cx.isBigIntObject() ) {
             if ( cy.isSmall() ) {
                if ( cy.getLong() == 0L ) {
                    throw Mishap( "MOD" );
                } else {
                    BigIntExternal modulus( *cx.asBigIntObject().getBigIntExternal() );
                    modulus.truncatedRemainderBy( cy.getLong() );
                    vm->fastPeek() = vm->heap().copyBigIntExternal( pc, modulus );
                }
            } else if ( cy.isBigIntObject() ) {
                BigIntExternal modulus( *cx.asBigIntObject().getBigIntExternal() );
                modulus.truncatedRemainderBy( *cy.asBigIntObject().getBigIntExternal() );
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

//  Ref * sysFlooredRemainderHelper( Ref * pc, class MachineClass * vm, Ref ry ) {
//      Ref rx = vm->fastPeek();
//      Cell cx( rx );
//      Cell cy( ry );
//      try {
//          if ( cx.isSmall() ) {
//              if ( cy.isSmall() ) {
//                  if ( cy.getLong() == 0 ) {
//                      throw Mishap( "MOD" );
//                  } else {
//                      const long a = ToLong( rx );
//                      const bool sa = a >= 0;
//                      const long pa = sa ? a : -a;
//  
//                      const long b = ToLong( ry );
//                      const bool sb = b > 0;
//                      const long pb = sb ? b : -b;
//  
//                      const long m4 = pa % pb;
//  
//                      if ( sa && sb ) {
//                          vm->fastPeek() = ToRef( m4 );
//                      } else if ( sa and not sb ) {
//                          vm->fastPeek() = ToRef( b + m4 );
//                      } else if ( not sa and sb ) {
//                          vm->fastPeek() = ToRef( b - m4 );
//                      } else {
//                          vm->fastPeek() = ToRef( -m4 );
//                      }
//                  }
//              } else if ( cy.isBigIntObject() ) {
//                  if ( cy.isZero() ) {
//                      throw Mishap( "MOD" );
//                  } else {
//                      BigIntExternal * by = cy.asBigIntObject().getBigIntExternal();
//                      if ( 
//                          ( cx.isPositive() and by->isPositive() ) or 
//                          ( cx.isNegative() and by->isNegative() )
//                      ) {
//                          vm->fastPeek() = rx;   
//                      } else {
//                          BigIntExternal r( *by );
//                          r.addBy( cx.getLong() );
//                          vm->fastPeek() = vm->heap().copyBigIntExternal( pc, r );
//                      }
//                  }
//              } else {
//                  throw Mishap( "MOD" );                
//              }
//          } else if ( cx.isBigIntObject() ) {
//               if ( cy.isSmall() ) {
//                  if ( cy.getLong() == 0L ) {
//                      throw Mishap( "MOD" );
//                  } else {
//                      BigIntExternal modulus( *cx.asBigIntObject().getBigIntExternal() );
//                      modulus.flooredRemainderBy( cy.getLong() );
//                      vm->fastPeek() = vm->heap().copyBigIntExternal( pc, modulus );
//                  }
//              } else if ( cy.isBigIntObject() ) {
//                  BigIntExternal modulus( *cx.asBigIntObject().getBigIntExternal() );
//                  modulus.flooredRemainderBy( *cy.asBigIntObject().getBigIntExternal() );
//                  vm->fastPeek() = vm->heap().copyBigIntExternal( pc, modulus );
//              } else {
//                  throw Mishap( "MOD" );
//              }                    
//          } else {
//              throw Mishap( "MOD" );
//          }
//      } catch ( Mishap & e ) {
//           throw Mishap( "Bad arguments for mod operation" ).culprit( "First", cx.toShowString() ).culprit( "Second", cy.toShowString() );        
//      }
//      return pc;
//  }

//  Ref * sysFlooredQuotientHelper( Ref * pc, class MachineClass * vm, Ref ry ) {
//      Ref rx = vm->fastPeek();
//      Cell cx( rx );
//      Cell cy( ry );
//      try {
//          if ( cx.isSmall() ) {
//              if ( cy.isSmall() ) {
//                  if ( cy.getLong() == 0L ) {
//                      throw Mishap( "QUO" );
//                  } else {
//                      const long a = ToLong( rx );
//                      const bool sa = a >= 0;
//                      const long pa = sa ? a : -a;
//  
//                      const long b = ToLong( ry );
//                      const bool sb = b > 0;
//                      const long pb = sb ? b : -b;
//  
//                      vm->fastPeek() = LongToSmall( ( sa == sb ) ? pa / pb : ( -1 - pa / pb ) );
//                  }
//              } else if ( cy.isBigIntObject() ) {
//                  if ( cy.isZero() ) {
//                      throw Mishap( "QUO" );
//                  } else {
//                      BigIntExternal * by = cy.asBigIntObject().getBigIntExternal();
//                      if ( 
//                          ( cx.isPositive() and by->isPositive() ) or 
//                          ( cx.isNegative() and by->isNegative() )
//                      ) {
//                          vm->fastPeek() = LongToSmall( 0 );   
//                      } else {
//                          vm->fastPeek() = LongToSmall( -1 );
//                      }
//                  }
//              } else {
//                  throw Mishap( "QUO" );
//              }
//          } else if ( cx.isBigIntObject() ) {
//               if ( cy.isSmall() ) {
//                  if ( cy.getLong() == 0L ) {
//                      throw Mishap( "QUO" );
//                  } else {
//                      BigIntExternal quotient( *cx.asBigIntObject().getBigIntExternal() );
//                      quotient.flooredQuotientBy( cy.getLong() );
//                      vm->fastPeek() = vm->heap().copyBigIntExternal( pc, quotient );
//                  }
//              } else if ( cy.isBigIntObject() ) {
//                  BigIntExternal quotient( *cx.asBigIntObject().getBigIntExternal() );
//                  quotient.flooredQuotientBy( *cy.asBigIntObject().getBigIntExternal() );
//                  vm->fastPeek() = vm->heap().copyBigIntExternal( pc, quotient );
//              } else {
//                  throw Mishap( "QUO" );
//              }           
//          } else {
//              throw Mishap( "QUO" );
//          }
//      } catch ( Mishap & e ) {
//          throw Mishap( "Bad arguments for quotient operation" ).culprit( "First", cx.toShowString() ).culprit( "Second", cy.toShowString() );        
//      }
//      return pc;
//  }

static gngdouble_t byZero( const bool lt, const bool gt ) {
    if ( gt ) {
        return 1.0/0.0;
    } else if ( lt ) {
        return -1.0/0.0;
    } else {
        return 0.0/0.0;
    }
}

Ref * sysDivHelper( Ref * pc, class MachineClass * vm, Ref ry ) {
    Ref rx = vm->fastPeek();
    Cell cx( rx );
    Cell cy( ry );

    try {
        if ( cx.isSmall() ) {
            if ( cy.isSmall() ) {
                const long lx = cx.getLong();
                const long ly = cy.getLong();
                RationalExternal q( lx );
                if ( ly == 0 ) {
                    vm->fastPeek() = vm->heap().copyDouble( pc, byZero( lx < 0, lx > 0 ) );
                } else {
                    q.divBy( ly );
                    vm->fastPeek() = vm->heap().copyRationalExternal( pc, q );
                }
            } else if ( cy.isDoubleObject() ) {
                const gngdouble_t x = static_cast< gngdouble_t >( cx.getLong() );
                const gngdouble_t y = cy.asDoubleObject().getDouble();
                vm->fastPeek() = vm->heap().copyDouble( pc, x / y );
            } else if ( cy.isBigIntObject() ) {
                const long lx = cx.getLong();
                if ( cy.asBigIntObject().getBigIntExternal()->isZero() ) {
                    vm->fastPeek() = vm->heap().copyDouble( pc, byZero( lx < 0, lx > 0 ) );
                } else {
                    RationalExternal q( lx );
                    q.divBy( *cy.asBigIntObject().getBigIntExternal() );
                    vm->fastPeek() = vm->heap().copyRationalExternal( pc, q );
                }
            } else if ( cy.isRationalObject() ) {
                const long lx = cx.getLong();
                if ( cy.asRationalObject().getRationalExternal()->isZero() ) {
                    vm->fastPeek() = vm->heap().copyDouble( pc, byZero( lx < 0, lx > 0 ) );
                } else {
                    RationalExternal q( lx );
                    q.divBy( *cy.asRationalObject().getRationalExternal() );
                    vm->fastPeek() = vm->heap().copyRationalExternal( pc, q );
                }
            } else {
                throw Mishap( "Bad arguments for / operator" );
            }
        } else if ( cx.isDoubleObject() ) {
            const gngdouble_t x = cx.asDoubleObject().getDouble();
            gngdouble_t y;
            if ( cy.isSmall() ) {
                y = static_cast< gngdouble_t >( cy.getLong() );
            } else if ( cy.isDoubleObject() ) {
                y = cy.asDoubleObject().getDouble();
            } else if ( cy.isBigIntObject() ) {
                y = cy.asBigIntObject().getBigIntExternal()->toFloat();
            } else if ( cy.isRationalObject() ) {
                y = cy.asRationalObject().getRationalExternal()->toFloat();
            } else {
                throw Mishap( "Bad arguments for / operator" );
            }
            vm->fastPeek() = vm->heap().copyDouble( pc, x / y );
       } else if ( cx.isBigIntObject() ) {
            BigIntExternal * bx = cx.asBigIntObject().getBigIntExternal();
            if ( cy.isSmall() ) {
                const long ly = cy.getLong();
                if ( ly == 0 ) {
                    vm->fastPeek() = vm->heap().copyDouble( byZero( bx->isNegative(), bx->isPositive() ) );
                } else {
                    RationalExternal q( *bx );
                    q.divBy( cy.getLong() );
                    vm->fastPeek() = vm->heap().copyRationalExternal( pc, q );
                }
            } else if ( cy.isDoubleObject() ) {
                gngdouble_t dx = bx->toFloat();
                gngdouble_t dy = cy.asDoubleObject().getDouble();
                vm->fastPeek() = vm->heap().copyDouble( pc, dx / dy );
            } else if ( cy.isBigIntObject() ) {
                if ( cy.asBigIntObject().getBigIntExternal()->isZero() ) {
                    vm->fastPeek() = vm->heap().copyDouble( pc, byZero( bx->isNegative(), bx->isPositive() ) );
                } else {
                    RationalExternal q( *bx );
                    q.divBy( *cy.asBigIntObject().getBigIntExternal() );
                    vm->fastPeek() = vm->heap().copyRationalExternal( pc, q );
                }
            } else if ( cy.isRationalObject() ) {
                if ( cy.asRationalObject().getRationalExternal()->isZero() ) {
                    vm->fastPeek() = vm->heap().copyDouble( pc, byZero( bx->isNegative(), bx->isPositive() ) );
                } else {
                    RationalExternal q( *bx );
                    q.divBy( *cy.asRationalObject().getRationalExternal() );
                    vm->fastPeek() = vm->heap().copyRationalExternal( pc, q );
                }
            } else {
                throw Mishap( "Bad arguments for / operator" );
            }
        } else if ( cx.isRationalObject() ) {
            RationalExternal * qx = cx.asRationalObject().getRationalExternal();
            if ( cy.isSmall() ) {
                const long ly = cy.getLong();
                if ( ly == 0 ) {
                    vm->fastPeek() = vm->heap().copyDouble( pc, byZero( qx->isNegative(), qx->isPositive() ) );
                } else {
                    RationalExternal q( *qx );
                    q.divBy( ly );
                    vm->fastPeek() = vm->heap().copyRationalExternal( pc, q );
                }
            } else if ( cy.isDoubleObject() ) {
                gngdouble_t dx = qx->toFloat();
                gngdouble_t dy = cy.asDoubleObject().getDouble();
                vm->fastPeek() = vm->heap().copyDouble( pc, dx / dy );  
            } else if ( cy.isBigIntObject() ) {
                if ( cy.asRationalObject().getRationalExternal()->isZero() ) {
                    vm->fastPeek() = vm->heap().copyDouble( pc, byZero( qx->isNegative(), qx->isPositive() ) );
                } else {
                    RationalExternal q( *qx );
                    q.divBy( *cy.asBigIntObject().getBigIntExternal() );
                    vm->fastPeek() = vm->heap().copyRationalExternal( pc, q );
                }
            } else if ( cy.isRationalObject() ) {
                if ( cy.asRationalObject().getRationalExternal()->isZero() ) {
                    vm->fastPeek() = vm->heap().copyDouble( pc, byZero( qx->isNegative(), qx->isPositive() ) );                    
                } else {
                    RationalExternal q( *qx );
                    q.divBy( *cy.asRationalObject().getRationalExternal() );
                    vm->fastPeek() = vm->heap().copyRationalExternal( pc, q );
                }
            } else {
                throw Mishap( "Bad arguments for / operator" );
            }
        } else {
            throw Mishap( "Bad arguments for / operator" );
        }
        //vm->fastPeek() = vm->heap().copyDouble( pc, x / y );
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
            } else if ( cy.isRationalObject() ) {
                RationalExternal q( *cy.asRationalObject().getRationalExternal() );
                q.mulBy( cx.getLong() );
                vm->fastPeek() = vm->heap().copyRationalExternal( pc, q );
            } else {
                throw Mishap( "MUL" );
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
            } else if ( cy.isRationalObject() ) {
                y = cy.asRationalObject().getRationalExternal()->toFloat();
            } else {
                throw Mishap( "MUL" );
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
            } else if ( cy.isRationalObject() ) {
                RationalExternal q( *cy.asRationalObject().getRationalExternal() );
                q.mulBy( *cx.asBigIntObject().getBigIntExternal() );
                vm->fastPeek() = vm->heap().copyRationalExternal( pc, q );
            } else {
                throw Mishap( "MUL" );
            }
        } else if ( cx.isRationalObject() ) {
            RationalExternal * qx = cx.asRationalObject().getRationalExternal();
            if ( cy.isSmall() ) {
                RationalExternal q( *qx );
                q.mulBy( cy.getLong() );
                vm->fastPeek() = vm->heap().copyRationalExternal( pc, q );
            } else if ( cy.isDoubleObject() ) {
                vm->fastPeek() = vm->heap().copyDouble( pc, qx->toFloat() * cy.asDoubleObject().getDouble() );
            } else if ( cy.isBigIntObject() ) {
                RationalExternal q( *qx );
                q.mulBy( *cy.asBigIntObject().getBigIntExternal() );
                vm->fastPeek() = vm->heap().copyRationalExternal( pc, q ); 
            } else if ( cy.isRationalObject() ) {
                RationalExternal q( *cx.asRationalObject().getRationalExternal() );
                q.mulBy( *cy.asBigIntObject().getBigIntExternal() );
                vm->fastPeek() = vm->heap().copyRationalExternal( pc, q );
            } else {
                throw Mishap( "MUL" );
            }
        } else {
            throw Mishap( "MUL" );
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
            } else if ( cy.isRationalObject() ) {
                RationalExternal q( *cy.asRationalObject().getRationalExternal() );
                q.subBy( cx.getLong() );
                vm->fastPeek() = vm->heap().copyRationalExternal( pc, q );
            } else {
                throw Mishap( "SUB" );
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
            } else if ( cy.isRationalObject() ) {
                y = cy.asRationalObject().getRationalExternal()->toFloat();
            } else {
                throw Mishap( "SUB" );
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
            } else if ( cy.isRationalObject() ) {
                RationalExternal q( *cy.asRationalObject().getRationalExternal() );
                q.subBy( *cx.asBigIntObject().getBigIntExternal() );
                vm->fastPeek() = vm->heap().copyRationalExternal( pc, q );
            } else {
                throw Mishap( "SUB" );
            }
        } else if ( cx.isRationalObject() ) {
            RationalExternal * qx = cx.asRationalObject().getRationalExternal();
            if ( cy.isSmall() ) {
                RationalExternal q( *qx );
                q.subBy( cy.getLong() );
                vm->fastPeek() = vm->heap().copyRationalExternal( pc, q );
            } else if ( cy.isDoubleObject() ) {
                vm->fastPeek() = vm->heap().copyDouble( pc, qx->toFloat() - cy.asDoubleObject().getDouble() );
            } else if ( cy.isBigIntObject() ) {
                RationalExternal q( *qx );
                q.subBy( *cy.asBigIntObject().getBigIntExternal() );
                vm->fastPeek() = vm->heap().copyRationalExternal( pc, q ); 
            } else if ( cy.isRationalObject() ) {
                RationalExternal q( *cx.asRationalObject().getRationalExternal() );
                q.subBy( *cy.asRationalObject().getRationalExternal() );
                vm->fastPeek() = vm->heap().copyRationalExternal( pc, q );
            } else {
                throw Mishap( "SUB" );
            }
        } else {
            throw Mishap( "SUB" );
        }
    } catch ( Mishap & m ) {
        throw Mishap( "Bad arguments for - operator" ).culprit( "First", cx.toShowString() ).culprit( "Second", cy.toShowString() );
    }
    return pc;
}

Ref * sysAddHelper( Ref * pc, class MachineClass * vm, Ref ry ) {
    Ref rx = vm->fastPeek();
    Cell cx( rx );
    Cell cy( ry );
    try {
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
            } else if ( cy.isRationalObject() ) {
                RationalExternal q( *cy.asRationalObject().getRationalExternal() );
                q.addBy( cx.getLong() );
                vm->fastPeek() = vm->heap().copyRationalExternal( pc, q );
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
            } else if ( cy.isRationalObject() ) {
                y = cy.asRationalObject().getRationalExternal()->toFloat();
            } else {
                throw Mishap( "Bad arguments for + operator" );
            }
            //std::cout << "two doubles: " << x << ", " << y << std::endl;
            vm->fastPeek() = vm->heap().copyDouble( pc, x + y );
        } else if ( cx.isBigIntObject() ) {
            BigIntExternal * bx = cx.asBigIntObject().getBigIntExternal();
            if ( cy.isSmall() ) {
                const BigIntExternal sum( bx->add( cy.getLong() ) );
                vm->fastPeek() = vm->heap().copyBigIntExternal( pc, sum );
            } else if ( cy.isDoubleObject() ) {
                vm->fastPeek() = vm->heap().copyDouble( pc, bx->toFloat() + cy.asDoubleObject().getDouble() );
            } else if ( cy.isBigIntObject() ) {
                BigIntExternal * by = cy.asBigIntObject().getBigIntExternal();
                BigIntExternal sum( bx->add( *by ) );
                vm->fastPeek() = vm->heap().copyBigIntExternal( pc, sum );
            } else if ( cy.isRationalObject() ) {
                RationalExternal q( *cy.asRationalObject().getRationalExternal() );
                q.addBy( *cx.asBigIntObject().getBigIntExternal() );
                vm->fastPeek() = vm->heap().copyRationalExternal( pc, q );
            } else {
                throw Mishap( "Bad arguments for + operator" );
            }
        } else if ( cx.isRationalObject() ) {
            RationalExternal * bx = cx.asRationalObject().getRationalExternal();
            if ( cy.isSmall() ) {
                RationalExternal sum( *bx );
                sum.addBy( cy.getLong() );
                vm->fastPeek() = vm->heap().copyRationalExternal( pc, sum );
            } else if ( cy.isDoubleObject() ) {
                vm->fastPeek() = vm->heap().copyDouble( pc, bx->toFloat() + cy.asDoubleObject().getDouble() );
            } else if ( cy.isBigIntObject() ) {
                BigIntExternal * by = cy.asBigIntObject().getBigIntExternal();
                RationalExternal sum( *bx );
                sum.addBy( *by );
                vm->fastPeek() = vm->heap().copyRationalExternal( pc, sum );
            } else if ( cy.isRationalObject() ) {
                RationalExternal sum( *bx );
                sum.addBy( *cy.asRationalObject().getRationalExternal() );
                vm->fastPeek() = vm->heap().copyRationalExternal( pc, sum );
            } else {
                throw Mishap( "Bad arguments for + operator" );
            }
        } else {
            throw Mishap( "Bad arguments for + operator" );
        }
    } catch ( Mishap & m ) {
        throw Mishap( "Bad arguments for + operator" ).culprit( "First", cx.toShowString() ).culprit( "Second", cy.toShowString() );
    }
    return pc;
}

Ref * sysAdd( Ref * pc, class MachineClass * vm ) {
    if ( vm->count != 2 ) throw Ginger::Mishap( "ArgsMismatch" );
    return sysAddHelper( pc, vm, vm->fastPop() );
}


Ref * sysMax( Ref * pc, class MachineClass * vm ) {
    if ( vm->count != 2 ) throw Ginger::Mishap( "ArgsMismatch" );
    Ref rhs = vm->fastPop();
    Ref lhs = vm->fastPeek();
    vm->fastPeek() = sysCompareNumbers( lhs, rhs, false, false, true ) ? lhs : rhs;
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
    Ref rhs = vm->fastPop();
    Ref lhs = vm->fastPeek();
    vm->fastPeek() = sysCompareNumbers( lhs, rhs, true, false, false ) ? lhs : rhs;
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
        Ref sofar = vm->fastPop();
        for ( int i = 1; i < vm->count; i++ ) {
            Ref next = vm->fastPop();
            if ( sysCompareNumbers( sofar, next, true, false, false ) ) {
                sofar = next;
            }
        }
        vm->fastPush( sofar );
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
        Ref sofar = vm->fastPop();
        for ( int i = 1; i < vm->count; i++ ) {
            Ref next = vm->fastPop();
            if ( sysCompareNumbers( sofar, next, true, false, false ) ) {
                sofar = next;
            }
        }
        vm->fastPush( sofar );
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
    Ref rhs = vm->fastPop();
    Ref lhs = vm->fastPeek();
    vm->fastPeek() = sysCompareNumbers( lhs, rhs, true, false, true ) ? SYS_TRUE : SYS_FALSE;
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
    Ref rhs = vm->fastPop();
    Ref lhs = vm->fastPeek();
    vm->fastPeek() = sysCompareNumbers( lhs, rhs, true, true, true ) ? SYS_TRUE : SYS_FALSE;
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
    Ref rhs = vm->fastPop();
    Ref lhs = vm->fastPeek();
    vm->fastPeek() = sysCompareNumbers( lhs, rhs, true, false, false ) ? SYS_FALSE : SYS_TRUE;
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
    Ref rhs = vm->fastPop();
    Ref lhs = vm->fastPeek();
    vm->fastPeek() = sysCompareNumbers( lhs, rhs, true, true, false ) ? SYS_FALSE : SYS_TRUE;
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
    Ref rhs = vm->fastPop();
    Ref lhs = vm->fastPeek();
    vm->fastPeek() = sysCompareNumbers( lhs, rhs, false, false, true ) ? SYS_FALSE : SYS_TRUE;
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
    Ref rhs = vm->fastPop();
    Ref lhs = vm->fastPeek();
    vm->fastPeek() = sysCompareNumbers( lhs, rhs, false, true, true ) ? SYS_FALSE : SYS_TRUE;
    return pc;
}
SysInfo infoNotGtE( 
    SysNames( "!>=" ), 
    Ginger::Arity( 2 ), 
    Ginger::Arity( 1 ), 
    sysNotGtE, 
    "Returns false if the first argument is greater than or equal to the second, else true."
);

gngdouble_t refToDouble( Ref rx ) {
    Cell cx( rx );
    if ( cx.isSmall() ) {
        return TransDouble( static_cast< double >( cx.getLong() ) );
    } else if ( cx.isDoubleObject() ) {
        return cx.asDoubleObject().getDouble();
    } else if ( cx.isBigIntObject() ) {
        return TransDouble( cx.asBigIntObject().getBigIntExternal()->toFloat() );
    } else if ( cx.isRationalObject() ) {
        return TransDouble( cx.asRationalObject().getRationalExternal()->toFloat() );
    } else {
        throw Ginger::Mishap( "Number needed" ).culprit( "Value", cx.toShowString() );
    }
}

Ref * sysSin( Ref * pc, class MachineClass * vm ) {
    if ( vm->count != 1 ) throw Ginger::Mishap( "ArgsMismatch" );
    vm->fastPeek() = vm->heap().copyDouble( pc, refToDouble( vm->fastPeek() ).sin().asDouble() );
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
    vm->fastPeek() = vm->heap().copyDouble( pc, refToDouble( vm->fastPeek() ).cos().asDouble() );
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
    vm->fastPeek() = vm->heap().copyDouble( pc, refToDouble( vm->fastPeek() ).tan().asDouble() );
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
    vm->fastPeek() = vm->heap().copyDouble( pc, refToDouble( vm->fastPeek() ).exp().asDouble() );
    return pc;
}
SysInfo infoExp( 
    SysNames( "exp" ), 
    Ginger::Arity( 1 ), 
    Ginger::Arity( 1 ), 
    sysExp, 
    "Returns the exponential of a real value (e**x)."
);

Ref * sysQuoHelper( Ref * pc, class MachineClass * vm, Ref ry ) {
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
                    const long b = ToLong( ry );
                    vm->fastPeek() = LongToSmall( a / b );
                }
            } else if ( cy.isBigIntObject() ) {
                if ( cy.isZero() ) {
                    throw Mishap( "QUO" );
                } else {
                    BigIntExternal * by = cy.asBigIntObject().getBigIntExternal();
                    if ( 
                        ( cx.isPositive() and by->isPositive() ) or 
                        ( cx.isNegative() and by->isNegative() )
                    ) {
                        vm->fastPeek() = LongToSmall( 0 );   
                    } else {
                        vm->fastPeek() = LongToSmall( -1 );
                    }
                }
            } else {
                throw Mishap( "QUO" );
            }
        } else if ( cx.isBigIntObject() ) {
             if ( cy.isSmall() ) {
                if ( cy.getLong() == 0L ) {
                    throw Mishap( "QUO" );
                } else {
                    BigIntExternal quotient( *cx.asBigIntObject().getBigIntExternal() );
                    quotient.truncatedQuotientBy( cy.getLong() );
                    vm->fastPeek() = vm->heap().copyBigIntExternal( pc, quotient );
                }
            } else if ( cy.isBigIntObject() ) {
                BigIntExternal quotient( *cx.asBigIntObject().getBigIntExternal() );
                quotient.truncatedQuotientBy( *cy.asBigIntObject().getBigIntExternal() );
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



Ref * sysDivMod( Ref * pc, class MachineClass * vm ) {
    if ( vm->count != 2 ) throw Ginger::Mishap( "ArgsMismatch" ).culprit( "Args received", vm->count ).culprit( "Args needed", 2 );
    Ref ry = vm->fastPeek();
    Ref rx = vm->fastPeek( 1 );
    Cell cx( rx );
    Cell cy( ry );
    try {
        if ( cx.isSmall() ) {
            if ( cy.isSmall() ) {
                if ( cy.getLong() == 0 ) {
                    throw Mishap( "MOD" );
                } else {
                    #ifdef PYTHONIC_DIVMOD
                        const long a = ToLong( rx );
                        const bool sa = a >= 0;
                        const long pa = sa ? a : -a;

                        const long b = ToLong( ry );
                        const bool sb = b > 0;
                        const long pb = sb ? b : -b;

                        const long m4 = pa % pb;

                        vm->fastPeek( 1 ) = LongToSmall( ( sa == sb ) ? pa / pb : ( -1 - pa / pb ) );
                        if ( sa && sb ) {
                            vm->fastPeek() = ToRef( m4 );
                        } else if ( sa and not sb ) {
                            vm->fastPeek() = ToRef( b + m4 );
                        } else if ( not sa and sb ) {
                            vm->fastPeek() = ToRef( b - m4 );
                        } else {
                            vm->fastPeek() = ToRef( -m4 );
                        }
                    #else
                        const long a = ToLong( rx );
                        const long b = ToLong( ry );
                        vm->fastPeek( 1 ) = LongToSmall( a / b );
                        vm->fastPeek() = ToRef( a % b );
                    #endif
                }
            } else if ( cy.isBigIntObject() ) {
                if ( cy.isZero() ) {
                    throw Mishap( "QUO" );
                } else {
                    BigIntExternal * by = cy.asBigIntObject().getBigIntExternal();
                    if ( 
                        ( cx.isPositive() and by->isPositive() ) or 
                        ( cx.isNegative() and by->isNegative() )
                    ) {
                        vm->fastPeek( 1 ) = LongToSmall( 0 );   
                        vm->fastPeek() = rx;   
                    } else {
                        vm->fastPeek( 1 ) = LongToSmall( -1 );
                        BigIntExternal r( *by );
                        r.addBy( cx.getLong() );
                        vm->fastPeek() = vm->heap().copyBigIntExternal( pc, r );
                    }
                }
            } else {
                throw Mishap( "MOD" );                
            }
        } else if ( cx.isBigIntObject() ) {
             if ( cy.isSmall() ) {
                if ( cy.getLong() == 0L ) {
                    throw Mishap( "MOD" );
                } else {
                    BigIntExternal modulus( *cx.asBigIntObject().getBigIntExternal() );
                    modulus.truncatedRemainderBy( cy.getLong() );

                    BigIntExternal quotient( *cx.asBigIntObject().getBigIntExternal() );
                    quotient.truncatedQuotientBy( cy.getLong() );

                    vm->fastPeek( 1 ) = vm->heap().copyBigIntExternal( pc, quotient );
                    vm->fastPeek() = vm->heap().copyBigIntExternal( pc, modulus );
                }
            } else if ( cy.isBigIntObject() ) {
                BigIntExternal modulus( *cx.asBigIntObject().getBigIntExternal() );
                modulus.truncatedRemainderBy( *cy.asBigIntObject().getBigIntExternal() );

                BigIntExternal quotient( *cx.asBigIntObject().getBigIntExternal() );
                quotient.truncatedQuotientBy( *cy.asBigIntObject().getBigIntExternal() );

                vm->fastPeek( 1 ) = vm->heap().copyBigIntExternal( pc, quotient );
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
    return pc;    return pc;
}
SysInfo infoDivMod( 
    SysNames( "divmod" ), 
    Ginger::Arity( 2 ), 
    Ginger::Arity( 2 ), 
    sysDivMod, 
    "Returns integer quotient and remainder (floored)."
);

} // namespace Ginger
