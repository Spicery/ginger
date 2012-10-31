/*****************************************************************************\
Instruction QUO_I 

Summary
	Computes the integer quotient of A/B
	
Unchecked Precondition
	
Exceptions (Checked Preconditions)
	
Result (Postcondition)		

\*****************************************************************************/

//	TODO: Overflow checks, divide by zero checks, etc.
long b = ToLong( *( VMVP-- ) );
*( VMVP ) = ToRef( ToLong( *( VMVP ) ) / b );
RETURN( pc + 1 );
