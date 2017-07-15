/*****************************************************************************\
Definition
    SELF_CALL

Summary
    Used by directly recursive functions as they have a bit better self
    knowledge and can call themselves efficiently.
    
Unchecked Precondition
    VMCOUNT is correctly set.

Exceptions (Checked Preconditions)
    None
    
Result (Postcondition)      
    * The currently active function is re-entered.

\*****************************************************************************/

{    
    VMLINK = pc + 1;
    VMLINKFUNC = VMPCFUNC;
    RETURN( VMPCFUNC + 1 );
}
