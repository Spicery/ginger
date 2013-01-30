/*****************************************************************************\
Definition
	* PUSH_LOCAL1_RET () -> ( R ), R = LOCAL[ 1 ]
	* VMPCFUNC, VMSPC, VPC all restored from call-stack.
	
Summary
	Pushes the value of LOCAL[ 0 ] onto the stack and returns.
	
Unchecked Precondition
	* There is room on the stack for at least one more value.
	* The current frame has at least two local variables.
	* There is a calling function to return to.
	
Exceptions (Checked Preconditions)
	None.
	
Result (Postcondition)		
	* The top of stack is LOCAL[ 1 ]
	* The stack is one bigger.
	* Execution is returned to the calling function.

\*****************************************************************************/

*( ++VMVP ) = LOCAL(1);
Ref *linkptr;
VMPCFUNC = ToRefRef( VMSP[ SP_FUNC ] );
linkptr = ToRefRef( VMSP[ SP_LINK ] );				//	recover return address
VMSP = ToRefRef( VMSP[ SP_PREV_SP ] );				//	restore stack pointer
#ifdef DBG_SPECIAL
	printf( "Recover link = %x\n", ToUInt( link ) );
	printf( "Stack pointer recovered as %x\n", ToUInt( VMSP ) );
#endif
RETURN( linkptr );
