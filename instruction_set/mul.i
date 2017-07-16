/*****************************************************************************\
Definition
	MUL ( X:Num, Y:Num ) -> ( R:Num )

Code
    <mul />
    
Summary
	Takes two numbers X & Y off the value stack and replaces them
	with their product R, which will be a number.
	
Unchecked Precondition
	There must be at least two values on the stack.
	
Checked Preconditions
	Both X & Y are numbers.

Exceptions
	None
	
Postcondition
	X & Y are replaced by X*Y

Tags
	* Arithmetic operator

\*****************************************************************************/

Ref ry = *( VMVP-- );
Ref rx = *( VMVP );
if ( IsSmall( rx ) ) {
	if ( IsSmall( ry ) ) {
		gnglong_t hy = (gnglong_t)ry >> 1;	//	Scale down by factor of 2 & strip off low bit.
		gnglong_t hx = (gnglong_t)rx >> 1; 	//  As above.
		if ( not ( SignedOverflow::mulOverflowCheck( hx, hy ) ) ) {
			//std::cout << "Normal *" << std::endl;
			//	Note that the multiply will effectively restore the 00-tag.
			*VMVP = ToRef( hx * hy );
			RETURN( pc + 1 );
		}
	} else if ( IsDouble( ry ) ) {
		gngdouble_t x, y;
		y = gngFastDoubleValue( ry );
		x = static_cast< gngdouble_t >( SmallToLong( rx ) );
		*( VMVP ) = vm->heap().copyDouble( x * y );
		RETURN( pc + 1 );
	} 
} else if ( IsDouble( rx ) ) {
	gngdouble_t x, y;
	x = gngFastDoubleValue( rx );
	if ( IsSmall( ry ) ) {
		y = static_cast< gngdouble_t >( SmallToLong( ry ) );
		*( VMVP ) = vm->heap().copyDouble( x * y );
		RETURN( pc + 1 );
	} else if ( IsDouble( ry ) ) {
		y = gngFastDoubleValue( ry );
		*( VMVP ) = vm->heap().copyDouble( x * y );
		RETURN( pc + 1 );
	}
}
FREEZE;
pc = sysMulHelper( ++pc, vm, ry );
MELT;
RETURN( pc );
