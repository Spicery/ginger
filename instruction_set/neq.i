/*****************************************************************************\
Definition
    NEQ ( A, B ) -> ( R ), where R = A != B

Summary
    Removes two items from the top of the stack and compares them for 
    non-identity. If they are identical then false is pushed, otherwise true.
    
Unchecked Precondition
    There are at least two values on the stack.
    
Exceptions (Checked Preconditions)
    None
    
Result (Postcondition)
    The two items were removed.
    A single boolean result replaces them.
    Which is true iff the two values were not identical.

\*****************************************************************************/

Ref r = *( VMVP-- );
*( VMVP ) = *( VMVP ) == r ? SYS_FALSE : SYS_TRUE;
RETURN( pc + 1 );
