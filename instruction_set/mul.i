/*****************************************************************************\
Instruction MULI

Summary
	Takes two smalls X & Y off the value stack and replaces them
	with their product, which should be a small.
	
Unchecked Precondition
	There must be at least two values on the stack.
	
Checked Preconditions
	Both X & Y are small.

Exceptions
	The product X * Y is too large to fit in a small.
	
Postcondition
	X & Y are replaced by X*Y

\*****************************************************************************/

Ref ry = *( VMVP-- );
Ref rx = *( VMVP );
if ( IsSmall( rx ) && IsSmall( ry ) ) {
	//  TODO: This needs to be given overflow detection!
	gnglong_t y = (gnglong_t)ry >> 1;	//	Scale down by factor of 2 & strip off low bit.
	gnglong_t x = (gnglong_t)rx >> 1; 	//  As above.
	if ( SignedOverflow::mulOverflowCheck( x, y ) ) {
		x = static_cast< gngdouble_t >( SmallToLong( rx ) );
		y = static_cast< gngdouble_t >( SmallToLong( ry ) );
		*( VMVP ) = vm->heap().copyDouble( x * y );
	} else {
		//	Note that the multiply will effectively restore the 00-tag.
		*VMVP = ToRef( x * y );
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
		throw Mishap( "Invalid arguments for *" );
	}
	*( VMVP ) = vm->heap().copyDouble( x * y );
	RETURN( pc + 1 );
} else {
	//	TODO: update message with arguments.
	throw Mishap( "Numbers only" );
} 
