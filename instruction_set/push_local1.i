/*****************************************************************************\
Definition
	* PUSH_LOCAL1 () -> ( R ), where R = LOCAL[ 1 ]
	* VPC += 1
	
Summary
	Pushes the value of LOCAL[ 1 ] onto the stack.
	
Unchecked Precondition
	* There's room on the value stack for at least one more value.
	* There's at least 2 variable in the current call-stack frame.
	
Exceptions (Checked Preconditions)
	None.
	
Result (Postcondition)		
	* The top of stack is LOCAL[ 1 ]
	* The stack is one bigger.
	* Execution continues at the next instruction.

\*****************************************************************************/

*( ++VMVP ) = LOCAL(1);
RETURN( pc + 1 );
