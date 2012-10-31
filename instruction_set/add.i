/*****************************************************************************\
Instruction ADDI

Summary
	Takes two smalls X & Y off the value stack and replaces them
	with their sum, which should be a small.
	
Unchecked Precondition
	There must be at least two values on the stack.
	
Checked Preconditions
	Both X & Y are small.

Exceptions
	The sum of X + Y is too large to fit in a small.
	
Postcondition
	X & Y are replaced by X+Y

\*****************************************************************************/


Ref ry = *( VMVP-- );
Ref rx = *( VMVP );
if ( IsSmall( rx ) && IsSmall( ry ) ) {
	long y = (long)ry;
	long x = (long)rx;
	long sum = x + y;
	if ( x < 0 ? sum <= y : sum >= y ) {
		*VMVP = ToRef( sum );
		RETURN( pc + 1 );
	} else {
		//	TODO: convert to Double.
    	throw Mishap( "Overflow detected in +" );
	}
} else if ( IsDouble( rx ) ) {
	gngdouble x, y;
	x = gngFastDoubleValue( rx );
	if ( IsSmall( ry ) ) {
		y = static_cast< gngdouble >( SmallToLong( ry ) );
	} else if ( IsDouble( ry ) ) {
		y = gngFastDoubleValue( ry );
	} else {
		throw Mishap( "Invalid arguments for +" );
	}
	*( VMVP ) = vm->heap().copyDouble( x + y );
	RETURN( pc + 1 );
} else {
	//	TODO: update message with arguments.
	throw Mishap( "Numbers only" );
} 
