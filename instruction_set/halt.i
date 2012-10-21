/*****************************************************************************\
Instruction TBD

Summary
	
Unchecked Precondition
	
Exceptions (Checked Preconditions)
	
Result (Postcondition)		

\*****************************************************************************/

#ifdef DBG_SPECIAL	
printf( "\nhalt\n" );
#endif
fprintf( stderr, "Compilation error - fell off the end of a function\n" );
exit( EXIT_FAILURE );
