/*****************************************************************************\
Instruction TBD

Summary
	
Unchecked Precondition
	
Exceptions (Checked Preconditions)
	
Result (Postcondition)		

\*****************************************************************************/

Ref r = *( VMVP-- );
*( VMVP ) = *( VMVP ) == r ? SYS_TRUE : SYS_FALSE;
RETURN( pc + 1 );
