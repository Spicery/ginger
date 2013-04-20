/*****************************************************************************\
Definition
	* MOD_I ( X : Small, Y : Small ) -> ( R ), where R = X mod Y
	* VPC += 1

Summary
	Computes the integer modulus R of X and Y. X and Y are removed from
	the stack and R is pushed.
	
Unchecked Precondition
	* There are two items on the stack.
	
Exceptions (Checked Preconditions)
	* X and Y are Smalls
	
Result (Postcondition)		
	* R = X mod Y is pushed.
	* Execution continues at the next instruction.

Tags
	* Arithmetic operator

\*****************************************************************************/

Ref ry = *( VMVP-- );
Ref rx = *( VMVP );
if ( IsSmall( rx ) && IsSmall( ry ) ) {
	if ( IsZeroSmall( ry ) ) {
		throw Mishap( "MOD Instruction: Dividing by zero" ).culprit( "Numerator", refToShowString( rx ) );
	} else {
		long b = ToLong( ry );
		long a = ToLong( rx );
		*( VMVP ) = ToRef( a % b );
		RETURN( pc + 1 );
	}
} else {
	throw Mishap( "MOD Instruction: Integers only" ).culprit( "First", refToShowString( rx ) ).culprit( "Second", refToShowString( ry ) );
}

