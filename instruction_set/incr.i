/*****************************************************************************\
Summary
	Increments the value on top of the stack by 1.
	
Unchecked Precondition
	There is at least one value on the value stack.
	
Exceptions (Checked Preconditions)
	The value on top of the stack is a number.
	
Result (Postcondition)
	The top of the stack is one greater than before.	

\*****************************************************************************/

Ref rx = *VMVP;
if ( IsSmall( rx ) ) {
	long x = (long)rx;
	long sum = x + (long)LongToSmall( 1 );
	if ( sum > x ) {
		*( VMVP ) = ToRef( sum );
	} else {
		*( VMVP ) = (
			vm->heap().copyDouble( 
				static_cast< gngdouble_t >( x >> TAG ) + 1.0
			)
		);
	}
	RETURN( pc + 1 );
} else if ( IsDouble( rx ) ) {
	*( VMVP ) = vm->heap().copyDouble( gngFastDoubleValue( rx ) + 1.0 );
	RETURN( pc + 1 );
} else {
	throw Mishap( "INCR instruction: Number needed" ).culprit( "Value", refToString( rx ) );
}

