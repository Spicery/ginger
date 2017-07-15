/*****************************************************************************\
DISABLED IN FAVOUR OF MOD =^= FLOORED REMAINDER

Definition
    * REM_I ( X : Small, Y : Small ) -> ( R ), where R = X mod Y
    * VPC += 1

Summary
    Computes the integer remainder R of X / Y, such that R is in the
    open interval (-Y,Y). R has the sign of X. X and Y are removed from the 
    stack and R is pushed.
    
Unchecked Precondition
    * There are two items on the stack.
    
Exceptions (Checked Preconditions)
    * X and Y are Smalls
    
Result (Postcondition)      
    * R = X mod Y is pushed.
    * Execution continues at the next instruction.

Tags
    * Arithmetic operator

\*****************************************************************************/

Ref ry = *( VMVP-- );
Ref rx = *( VMVP );
if ( IsSmall( rx ) && IsSmall( ry ) and not( IsZeroSmall( ry ) ) ) {
    const long a = ToLong( rx );
    const bool sa = a >= 0;
    const bool pa = sa ? a : -a;

    const long b = ToLong( ry );
    const bool sb = b > 0;
    const long pb = sb ? b : -b;

    *( VMVP ) = sa ? ToRef( pa % pb ) : ToRef( -( pa % pb ) );
    RETURN( pc + 1 );
}
FREEZE;
pc = sysRemHelper( ++pc, vm, ry );
MELT;
RETURN( pc );
