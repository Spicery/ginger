/*****************************************************************************\
Instruction LTE (A, B) -> ( R ), where R = A <= B

Summary
	Takes two numbers A & B off the stack and pushs a boolean result
	A <= B onto the stack.
	
Unchecked Precondition
	*	There are at least two values on the stack. 
	
Exceptions (Checked Preconditions)
	*	The values aren't both numbers.
	
Result (Postcondition)		
	*	The two items are removed and replaced by a single boolean value.

\*****************************************************************************/

Ref b = *( VMVP-- );
Ref a = *( VMVP );

if ( IsSmall( a ) ) {
	if ( IsSmall( b ) ) {
		*( VMVP ) = ToLong( a ) <= ToLong( b ) ? SYS_TRUE : SYS_FALSE;
		RETURN( pc + 1 );
	} else if ( IsDouble( b ) ) {
		*( VMVP ) = static_cast< gngdouble_t >( SmallToLong( a ) ) <= gngFastDoubleValue( b ) ? SYS_TRUE : SYS_FALSE;
		RETURN( pc + 1 );
	}
} else if ( IsDouble( a ) ) {
	if ( IsSmall( b ) ) {
		*( VMVP ) = gngFastDoubleValue( a ) <= static_cast< gngdouble_t >( SmallToLong( b ) ) ? SYS_TRUE : SYS_FALSE;
		RETURN( pc + 1 );
	} else if ( IsDouble( b ) ) {
		*( VMVP ) = gngFastDoubleValue( a ) <= gngFastDoubleValue( b ) ? SYS_TRUE : SYS_FALSE;
		RETURN( pc + 1 );
	} 
} 

throw Mishap( "LTE: Numbers needed" ).culprit( "First", refToString( a ) ).culprit( "Second", refToString( b ) );
RETURN( NULL ); // sop for compiler.
