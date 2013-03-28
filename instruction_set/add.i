/*****************************************************************************\
Definition
	ADD ( X:Num, Y:Num ) -> ( X+Y )

Summary
	Takes two numbers X & Y off the value stack and replaces them
	with their sum, which should be a number.
	
Unchecked Precondition
	There must be at least two values on the stack.
	
Checked Preconditions
	Both X & Y are numbers.

Exceptions
	None
	
Postcondition
	X & Y are replaced by X+Y

Tags
	* Arithmetic operator

\*****************************************************************************/


Ref ry = *( VMVP-- );
Ref rx = *( VMVP );
if ( IsSmall( rx ) ) {
	if ( IsSmall( ry ) ) {
		long y = (long)ry;
		long x = (long)rx;
		long sum = x + y;
		//std::cout << "two smalls: " << x << ", " << y << std::endl;
		if ( x < 0L ? sum < y : sum >= y ) {
			//std::cout << "x, y, sum, sum >= y, ( x < 0L ? sum <= y : sum >= y ), sum < 0" << std::endl;
			//std::cout << x << ", " << y << ", " << sum << ", " << ( sum >= y ) << ", " << ( x < 0L ? sum <= y : sum >= y ) << ", " << ( sum < 0L ) << std::endl;
			//std::cout << "result was small: " << sum << " n.b. " << ( x < 0L ) << ", " << ( sum >= y ) << std::endl;
			*VMVP = ToRef( sum );
		} else {
			*( VMVP ) = (
				vm->heap().copyDouble( 
					static_cast< gngdouble_t >( x >> TAG ) + 
					static_cast< gngdouble_t >( y >> TAG )
				)
			);
		}
		RETURN( pc + 1 );
	} else if ( IsDouble( ry ) ) {
		gngdouble_t x, y;
		y = gngFastDoubleValue( ry );
		x = static_cast< gngdouble_t >( SmallToLong( rx ) );
		*( VMVP ) = vm->heap().copyDouble( x + y );
		RETURN( pc + 1 );
	} else {
		throw Mishap( "Bad arguments for + operator" ).culprit( "First", refToString( rx ) ).culprit( "Second", refToString( ry ) );
	}
} else if ( IsDouble( rx ) ) {
	gngdouble_t x, y;
	x = gngFastDoubleValue( rx );
	if ( IsSmall( ry ) ) {
		y = static_cast< gngdouble_t >( SmallToLong( ry ) );
	} else if ( IsDouble( ry ) ) {
		y = gngFastDoubleValue( ry );
	} else {
		throw Mishap( "Bad arguments for + operator" ).culprit( "First", refToString( rx ) ).culprit( "Second", refToString( ry ) );
	}
	//std::cout << "two doubles: " << x << ", " << y << std::endl;
	*( VMVP ) = vm->heap().copyDouble( x + y );
	RETURN( pc + 1 );
} else {
	throw Mishap( "Bad arguments for + operator" ).culprit( "First", refToString( rx ) ).culprit( "Second", refToString( ry ) );
} 
