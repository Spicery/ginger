/*****************************************************************************\
Instruction MOD_I 

Summary
	Computes the integer modulus of A/B
	
Unchecked Precondition
	
Exceptions (Checked Preconditions)
	
Result (Postcondition)		

\*****************************************************************************/

Ref ry = *( VMVP-- );
Ref rx = *( VMVP );
if ( IsSmall( rx ) && IsSmall( ry ) ) {
	//	TODO: Overflow checks, divide by zero checks, etc.
	long b = ToLong( ry );
	long a = ToLong( rx );
	*( VMVP ) = ToRef( a % b );
	RETURN( pc + 1 );
} else {
	throw Mishap( "MOD Instruction: Integers only" ).culprit( "First", refToString( rx ) ).culprit( "Second", refToString( ry ) );
}

