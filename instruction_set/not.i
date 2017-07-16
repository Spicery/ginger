/*****************************************************************************\
Definition
    NOT ( X ) -> ( R ), where R = not( X )

Code
    <not />

Summary
    Boolean negation
    
Unchecked Precondition
    There's at least one item on the stack.
    
Exceptions (Checked Preconditions)
    None
    
Result (Postcondition)
    The top of the stack is replaced by its boolean negation. Note that
    non-false values count as true for this instruction - that's a feature
    not a bug.

\*****************************************************************************/


*VMVP = ( *VMVP == SYS_FALSE ? SYS_TRUE : SYS_FALSE );
RETURN( pc + 1 );
