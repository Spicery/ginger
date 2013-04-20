/*****************************************************************************\
Definition
	DECR 

Summary
	Decrements the value on top of the stack by 1.
	
Unchecked Precondition
	There is at least one value on the value stack.
	
Exceptions (Checked Preconditions)
	The value on top of the stack is a number.
	
Result (Postcondition)
	The top of the stack is one less than before.

Tags
	* Arithmetic operator

\*****************************************************************************/

Ref rx = *VMVP;
if ( IsSmall( rx ) ) {
	long x = (long)rx;
	long sum = x - (long)LongToSmall( 1 );
	if ( sum >= x ) {
		*( VMVP ) = (
			vm->heap().copyDouble( 
				static_cast< gngdouble_t >( x >> TAG ) - 1.0
			)
		);
	} else {
		*( VMVP ) = ToRef( sum );
	}
	RETURN( pc + 1 );
} else if ( IsDouble( rx ) ) {
	*( VMVP ) = vm->heap().copyDouble( gngFastDoubleValue( rx ) - 1.0 );
	RETURN( pc + 1 );
} else {
	throw Mishap( "DECR instruction: Number needed" ).culprit( "Value", refToShowString( rx ) );
}