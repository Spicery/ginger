/*****************************************************************************\
Definition
	SELF_CONSTANT () -> ( F )

Summary
	A function can push a reference to itself very efficiently. This
	helps implement the named-lambda form.
	
Unchecked Precondition
	There is room on the stack for the push of F.
	
Exceptions (Checked Preconditions)
	None
	
Result (Postcondition)		
	The top of the stack is a reference to the calling function.

\*****************************************************************************/

*( ++VMVP ) = VMPCFUNC;
RETURN( pc + 1 );
