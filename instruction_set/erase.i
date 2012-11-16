/*****************************************************************************\
Instruction ERASE

Summary
	Removes the topmost item from the value stack.
	
Unchecked Precondition
	There is at least one value on the value stack.
	
Exceptions (Checked Preconditions)
	
Result (Postcondition)
	The value stack is one shorter.

\*****************************************************************************/

{
	VMVP--;					//	Decrement the value-stack pointer.
	RETURN( pc + 1 );		//	Return the next instruction.
}
