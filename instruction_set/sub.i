/*****************************************************************************\
Definition
	* SUB ( X : Num, Y : Num ) -> ( R : Num ), where R = X - Y
	* VPC += 1

Summary
	Takes two Nums X & Y off the value stack and replaces them
	with their difference, which should be a Num.
	
Unchecked Precondition
	There must be at least two values on the stack.
	
Checked Preconditions
	Both X & Y are Nums.

Exceptions
	None
	
Postcondition
	* X & Y are replaced by X - Y
	* Execution continues at the next instruction.

\*****************************************************************************/

Ref ry = *( VMVP-- );
Ref rx = *( VMVP );
if ( IsSmall( rx ) && IsSmall( ry ) ) {
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
	throw Mishap( "Numbers only" ).culprit( "First", refToString( rx ) ).culprit( "Second", refToString( ry ) );
} 