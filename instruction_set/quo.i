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
	    const bool sa = a >= 0;
	    const long pa = sa ? a : -a;

	    const long b = ToLong( ry );
	    const bool sb = b > 0;
	    const long pb = sb ? b : -b;

	    *( VMVP ) = LongToSmall( ( sa == sb ) ? pa / pb : ( -1 - pa / pb ) );
		RETURN( pc + 1 );
	}
}
FREEZE;
pc = sysFlooredQuotientHelper( ++pc, vm, ry );
MELT;
RETURN( pc );

