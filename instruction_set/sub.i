/*****************************************************************************\
Definition
    SUB ( X : Num, Y : Num ) -> ( R : Num ), where R = X - Y
    
Code
    <sub />

Summary
    Takes two Nums X & Y off the value stack and replaces them
    with their difference, which should be a Num.
    
Unchecked Precondition
    There must be at least two values on the stack.
    
Checked Preconditions
    Both X & Y are Nums.

Exceptions
    None
    
Postcondition
    * X & Y are replaced by X - Y
    * Execution continues at the next instruction.

Tags
    * Arithmetic operator

\*****************************************************************************/

Ref ry = *( VMVP-- );
Ref rx = *( VMVP );
if ( IsSmall( rx ) ) {
    if ( IsSmall( ry ) ) {
        long y = (long)ry;
        long x = (long)rx;
        long diff = x - y;
        if ( y < 0L ? diff > x : diff <= x ) {
            *VMVP = ToRef( diff );
            RETURN( pc + 1 );
        } 
    } else if ( IsDouble( ry ) ) {
        gngdouble_t x, y;
        y = gngFastDoubleValue( ry );
        x = static_cast< gngdouble_t >( SmallToLong( rx ) );
        *( VMVP ) = vm->heap().copyDouble( x - y );
        RETURN( pc + 1 );
    }
} else if ( IsDouble( rx ) ) {
    gngdouble_t x, y;
    x = gngFastDoubleValue( rx );
    if ( IsSmall( ry ) ) {
        y = static_cast< gngdouble_t >( SmallToLong( ry ) );
        *( VMVP ) = vm->heap().copyDouble( x - y );
        RETURN( pc + 1 );
    } else if ( IsDouble( ry ) ) {
        y = gngFastDoubleValue( ry );
        *( VMVP ) = vm->heap().copyDouble( x - y );
        RETURN( pc + 1 );
    } 
} 
FREEZE;
pc = sysSubHelper( ++pc, vm, ry );
MELT;
RETURN( pc );
