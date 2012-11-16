/*****************************************************************************\
Instruction LTE (A, B) -> A < B

Summary
	Takes two numbers A & B off the stack and pushs a boolean result
	A < B onto the stack.
	
Unchecked Precondition
	*	There are at least two values on the stack. 
	
Exceptions (Checked Preconditions)
	*	The values aren't both numbers.
	
Result (Postcondition)		
	*	The two items are removed and replaced by a single boolean value.

\*****************************************************************************/

Ref b = *( VMVP-- );
Ref a = *( VMVP );

if ( not( IsSmall( a ) ) or not( IsSmall( b ) ) ) {
	throw Mishap( "Small integers needed" );
}

*( VMVP ) = ToLong( a ) <= ToLong( b ) ? SYS_TRUE : SYS_FALSE;
RETURN( pc + 1 );
