/*****************************************************************************\
Definition
    * PUSH_LOCAL0 () -> ( X ), where X = LOCAL[ 0 ]
    * VPC += 1
    
Code
	<push.local0 />

Summary
    Pushes the value of local variable 0 onto the stack.
    
Unchecked Precondition
    * There is room for at least one more value on the stack.
    * There is at least one local variable in the current call-stack frame.
    
Exceptions (Checked Preconditions)
    None.
    
Result (Postcondition)      
    * The top of the stack is LOCAL[ 0 ]
    * The stack is one bigger.
    * Execution continues on the next instruction.

\*****************************************************************************/

*( ++VMVP ) = LOCAL(0);
RETURN( pc + 1 );
