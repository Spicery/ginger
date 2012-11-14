/*****************************************************************************\
Instruction TBD

Summary
	
Unchecked Precondition
	
Exceptions (Checked Preconditions)
	
Result (Postcondition)		

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

