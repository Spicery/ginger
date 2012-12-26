/*****************************************************************************\
Instruction SUBI

Summary
	Takes a number x off the value stack and replaces it
	with its negation, which should be number.
	
Unchecked Precondition
	There must be at least one value on the stack.
	
Checked Preconditions
	X is a number
	
Postcondition
	X is replaced by -X

\*****************************************************************************/

Ref rx = *( VMVP );
if ( IsSmall( rx ) ) {
	long x = (long)rx;
	long negx = -x;

	if ( x != negx ) {
		*VMVP = ToRef( negx );
	} else {
		*( VMVP ) = (
			vm->heap().copyDouble( 
				- static_cast< gngdouble_t >( x >> TAG )
			)
		);
	}
	RETURN( pc + 1 );
} else if ( IsDouble( rx ) ) {
	gngdouble_t x;
	x = gngFastDoubleValue( rx );
	*( VMVP ) = vm->heap().copyDouble( -x );
	RETURN( pc + 1 );
} else {
	//	TODO: update message with arguments.
	throw Mishap( "Numbers only" ).culprit( "First", refToString( rx ) );
} 