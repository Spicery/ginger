/*****************************************************************************\
Definition
    RETURN_IFNOT ( X ) -> ()

Summary
    Conditionally returns from a function depending on the value on top
    of the stack.

    Q: I am unconvinced that this is a useful instruction. Check
    where it is used in the compiler.
    
Unchecked Precondition
    * There is at least one value on the top of the stack to pop.
    * There is a calling function to return to (if top of stack is not false)

Exceptions (Checked Preconditions)
    None
    
Result (Postcondition)      
    * If X was non-false
        * Execution contines at the next instruction.
    * Otherwise
        * Control is returned to the calling function.

\*****************************************************************************/

if( *( VMVP-- ) != SYS_FALSE ) {
    RETURN( pc + 1 );                                   //  Go onto the next instruction.
} else {
    Ref *linkptr;
    VMPCFUNC = ToRefRef( VMSP[ SP_FUNC ] );
    linkptr = ToRefRef( VMSP[ SP_LINK ] );              //  recover return address
    VMSP = ToRefRef( VMSP[ SP_PREV_SP ] );              //  restore stack pointer
    RETURN( linkptr );
}
