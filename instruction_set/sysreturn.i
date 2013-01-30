/*****************************************************************************\
Definition
	SYSRETURN

Summary
	An especially high performance return for native function objects.
	
Unchecked Precondition
	* The current function is a wrapper for a native function.
	* There is a function to return to.
	
Exceptions (Checked Preconditions)
	None.
	
Result (Postcondition)		
	* Execution continues at the calling function.

\*****************************************************************************/

VMPCFUNC = VMLINKFUNC;
RETURN( VMLINK );
