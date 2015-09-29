/*****************************************************************************\
Definition
	QUO ( X : Small, Y: Small ) -> ( R ), where R = X div Y
	Note that this is floored quotient.

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
	if ( not IsZeroSmall( ry ) ) {
	    const long a = ToLong( rx );
	    const long b = ToLong( ry );
	    *( VMVP ) = LongToSmall( a / b );
		RETURN( pc + 1 );
	}
}
FREEZE;
pc = sysQuoHelper( ++pc, vm, ry );
MELT;
RETURN( pc );

