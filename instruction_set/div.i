/*****************************************************************************\
Instruction TBD

Summary
	
Unchecked Precondition
	
Exceptions (Checked Preconditions)
	
Result (Postcondition)		

\*****************************************************************************/

long b = ToLong( *( VMVP-- ) );
*( VMVP ) = ToRef( ToLong( *( VMVP ) ) / b );
RETURN( pc + 1 );
