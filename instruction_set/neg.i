/*****************************************************************************\
Definition
    * NEGATE ( X : Num ) -> ( -X )

Summary
    Takes a number x off the value stack and replaces it
    with its negation, which should be number.
    
Unchecked Precondition
    There must be at least one value on the stack.
    
Checked Preconditions
    X is a number
    
Postcondition
    X is replaced by -X

\*****************************************************************************/

Ref rx = *( VMVP );
if ( IsSmall( rx ) ) {
    const long x = (long)rx;
    const long negx = -x;
    if ( x != negx ) {
        *VMVP = ToRef( negx );
    } else {
        *( VMVP ) = (
            vm->heap().copyDouble( 
                - static_cast< gngdouble_t >( x >> TAG )
            )
        );
    }
    RETURN( pc + 1 );
} else if ( IsDouble( rx ) ) {
    gngdouble_t x;
    x = gngFastDoubleValue( rx );
    *( VMVP ) = vm->heap().copyDouble( pc, -x );
    RETURN( pc + 1 );
} 
FREEZE;
pc = sysNegHelper( ++pc, vm );
MELT;
RETURN( pc );
