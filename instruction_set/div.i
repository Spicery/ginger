/*****************************************************************************\
Definition
    * DIV ( A, B ) -> ( R ), where R = A / B
    * VPC += 1

Summary
    Computes A divided by B in floating point arithmetic.
    Q: If the result is an exact integer, should it convert it back down?

Comment
    This is NOT the same as the high level function "div" but it is 
    the same as "/". For integer division see the QUO instruction.
    
Unchecked Precondition
    * There are two values on the top of the stack.
    
Exceptions (Checked Preconditions)
    * A & B are not numbers.
    
Result (Postcondition)      
    * The two items A & B are removed from the stack and replaced by R.
    * R is floating point.
    * R is equal to A / B with A & B converted to floating point.
    * Execution continues at the next instruction.

Tags
    * Arithmetic operator

\*****************************************************************************/

Ref ry = *( VMVP-- );
Ref rx = *( VMVP );
if ( IsSmall( rx ) ) {
    if ( IsDouble( ry ) ) {
        gngdouble_t x, y;
        y = gngFastDoubleValue( ry );
        x = static_cast< gngdouble_t >( SmallToLong( rx ) );
        *( VMVP ) = vm->heap().copyDouble( x / y );
        RETURN( pc + 1 );
    }
} else if ( IsDouble( rx ) ) {
    gngdouble_t x, y;
    x = gngFastDoubleValue( rx );
    if ( IsSmall( ry ) ) {
        y = static_cast< gngdouble_t >( SmallToLong( ry ) );
        *( VMVP ) = vm->heap().copyDouble( x / y );
        RETURN( pc + 1 );
    } else if ( IsDouble( ry ) ) {
        y = gngFastDoubleValue( ry );
        *( VMVP ) = vm->heap().copyDouble( x / y );
        RETURN( pc + 1 );
    } 
}
FREEZE;
pc = sysDivHelper( ++pc, vm, ry );
MELT;
RETURN( pc );

