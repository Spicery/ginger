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

#include <cmath>

#include "cell.hpp"
#include "mishap.hpp"
#include "machine.hpp"
#include "sys.hpp"  

namespace Ginger {
//using namespace std;

static void need_numbers( Cell lhs, Cell rhs ) {
    throw Mishap( "Numbers needed" ).culprit( "LHS", lhs.toShowString() ).culprit( "RHS", rhs.toShowString() );  
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