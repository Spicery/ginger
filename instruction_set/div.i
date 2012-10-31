/*****************************************************************************\
Instruction DIV_I A, B -> A / B

Summary
	Computes A divided by B in floating point arithmetic.
	TODO: If the result is an exact integer, should it convert it back down?
	
Unchecked Precondition
	
Exceptions (Checked Preconditions)
	
Result (Postcondition)		

\*****************************************************************************/

Ref ry = *( VMVP-- );
Ref rx = *( VMVP );
if ( IsSmall( rx ) && IsSmall( ry ) ) {
	gngdouble y = static_cast< gngdouble >( SmallToLong( ry ) );
	gngdouble x = static_cast< gngdouble >( SmallToLong( rx ) );
	*VMVP = vm->heap().copyDouble( x / y );
	RETURN( pc + 1 );
} else if ( IsDouble( rx ) ) {
	gngdouble x, y;
	x = gngFastDoubleValue( rx );
	if ( IsSmall( ry ) ) {
		y = static_cast< gngdouble >( SmallToLong( ry ) );
	} else if ( IsDouble( ry ) ) {
		y = gngFastDoubleValue( ry );
	} else {
		throw Mishap( "Invalid arguments for /" );
	}
	*( VMVP ) = vm->heap().copyDouble( x / y );
	RETURN( pc + 1 );
} else {
	//	TODO: update message with arguments.
	throw Mishap( "Numbers only" );
} 

