/*****************************************************************************\
Definition
    RESET

Code
    <reset />

Summary
    Exits the interpreter loop.

Unchecked Precondition
    None.
    
Exceptions (Checked Preconditions)
    None.
    
Result (Postcondition)      
    * The interpreter loop immediately exits.
    * The VM state is untouched.
    * If execution was to continue, the VPC would be the next instruction.

\*****************************************************************************/

pc += 1;
FREEZE;
throw NormalExit();
