/*****************************************************************************\
Instruction SUBI

Summary
	Takes two smalls X & Y off the value stack and replaces them
	with their difference, which should be a small.
	
Unchecked Precondition
	There must be at least two values on the stack.
	
Checked Preconditions
	Both X & Y are small.

Exceptions
	The difference X - Y is too large to fit in a small.
	
Postcondition
	X & Y are replaced by X - Y

\*****************************************************************************/

Ref ry = *( VMVP-- );
Ref rx = *( VMVP );
if ( IsSmall( rx ) && IsSmall( ry ) ) {
	//  TODO: This needs to be given overflow detection!
	long y = (long)ry;
	long x = (long)rx;
	long diff = x - y;
	if ( y < 0L ? diff > x : diff <= x ) {
		*VMVP = ToRef( diff );
	} else {
		*( VMVP ) = (
			vm->heap().copyDouble( 
				static_cast< gngdouble_t >( x >> TAG ) - 
				static_cast< gngdouble_t >( y >> TAG )
			)
		);
	}
	RETURN( pc + 1 );
} else if ( IsDouble( rx ) ) {
	gngdouble_t x, y;
	x = gngFastDoubleValue( rx );
	if ( IsSmall( ry ) ) {
		y = static_cast< gngdouble_t >( SmallToLong( ry ) );
	} else if ( IsDouble( ry ) ) {
		y = gngFastDoubleValue( ry );
	} else {
		throw Mishap( "Invalid arguments for -" );
	}
	*( VMVP ) = vm->heap().copyDouble( x - y );
	RETURN( pc + 1 );
} else {
	//	TODO: update message with arguments.
	throw Mishap( "Numbers only" ).culprit( "First", refToString( rx ) ).culprit( "Second", refToString( ry ) );
} 