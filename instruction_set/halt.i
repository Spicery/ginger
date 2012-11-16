/*****************************************************************************\
Instruction HALT

Summary
	Forces the virtual machine to exit with a failure.
	
Unchecked Precondition
	None
	
Exceptions (Checked Preconditions)
	None
	
Result (Postcondition)		
	End of the VM session.

\*****************************************************************************/

#ifdef DBG_SPECIAL	
printf( "\nhalt\n" );
#endif
fprintf( stderr, "Compilation error - fell off the end of a function\n" );
exit( EXIT_FAILURE );
