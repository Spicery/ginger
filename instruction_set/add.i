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
	//std::cout << "two smalls: " << x << ", " << y << std::endl;
	if ( x < 0L ? sum <= y : sum >= y ) {
		//std::cout << "x, y, sum, sum >= y, ( x < 0L ? sum <= y : sum >= y ), sum < 0" << std::endl;
		//std::cout << x << ", " << y << ", " << sum << ", " << ( sum >= y ) << ", " << ( x < 0L ? sum <= y : sum >= y ) << ", " << ( sum < 0L ) << std::endl;
		//std::cout << "result was small: " << sum << " n.b. " << ( x < 0L ) << ", " << ( sum >= y ) << std::endl;
		*VMVP = ToRef( sum );
	} else {
		*( VMVP ) = (
			vm->heap().copyDouble( 
				static_cast< gngdouble_t >( x ) + 
				static_cast< gngdouble_t >( y )
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
		throw Mishap( "Invalid arguments for +" );
	}
	//std::cout << "two doubles: " << x << ", " << y << std::endl;
	*( VMVP ) = vm->heap().copyDouble( x + y );
	RETURN( pc + 1 );
} else {
	throw Mishap( "Numbers only" ).culprit( "First", refToString( rx ) ).culprit( "Second", refToString( ry ) );
} 
