/*****************************************************************************\
Definition
	DUP

Summary
	Duplicates the topmost item of the value stack.
	
Unchecked Precondition
	There is at least one value on the value stack.
	There is room for one more item on the value stack.
	
Exceptions (Checked Preconditions)
	
Result (Postcondition)
	The value stack is one shorter.

\*****************************************************************************/

{
	VMVP[ 1 ] = VMVP[ 0 ];	//	Duplicate the top of stack to next-top.
	VMVP += 1;				//	Increment the value-stack pointer.
	RETURN( pc + 1 );		//	Return the next instruction.
}
