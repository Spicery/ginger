/*****************************************************************************\
Definition
	QUO ( X : Small, Y: Small ) -> ( R ), where R = X div Y

Summary
	Computes the integer quotient of A/B
	
Unchecked Precondition
	* There are two values on the stack

Exceptions (Checked Preconditions)
	* The two inputs X & Y are Smalls.

Result (Postcondition)		
	* X & Y are removed from the stack and replaced by R
	* R = X div Y

Tags
	* Arithmetic operator

\*****************************************************************************/

Ref ry = *( VMVP-- );
Ref rx = *( VMVP );
if ( IsSmall( rx ) && IsSmall( ry ) ) {
	if ( IsZeroSmall( ry ) ) {
		throw Mishap( "MOD Instruction: Dividing by zero" ).culprit( "Numerator", refToString( rx ) );
	} else {
		long b = ToLong( ry );
		long a = ToLong( rx );
		*( VMVP ) = LongToSmall( a / b );
		RETURN( pc + 1 );
	}
} else {
	throw Mishap( "QUO Instruction: Integers only" ).culprit( "First", refToString( rx ) ).culprit( "Second", refToString( ry ) );
}

