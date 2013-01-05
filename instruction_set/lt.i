/*****************************************************************************\
Instruction LT ( A, B ) -> ( R ), where R = A < B

Summary
	Removes the top two items from the value stack, which must be
	numbers, and compares them using the greater than or equal to operator.
	
Unchecked Precondition
	
Exceptions (Checked Preconditions)
	
Result (Postcondition)		

\*****************************************************************************/

Ref b = *( VMVP-- );
Ref a = *( VMVP );

if ( IsSmall( a ) ) {
	if ( IsSmall( b ) ) {
		*( VMVP ) = ToLong( a ) < ToLong( b ) ? SYS_TRUE : SYS_FALSE;
		RETURN( pc + 1 );
	} else if ( IsDouble( b ) ) {
		*( VMVP ) = static_cast< gngdouble_t >( SmallToLong( a ) ) < gngFastDoubleValue( b ) ? SYS_TRUE : SYS_FALSE;
		RETURN( pc + 1 );
	}
} else if ( IsDouble( a ) ) {
	if ( IsSmall( b ) ) {
		*( VMVP ) = gngFastDoubleValue( a ) < static_cast< gngdouble_t >( SmallToLong( b ) ) ? SYS_TRUE : SYS_FALSE;
		RETURN( pc + 1 );
	} else if ( IsDouble( b ) ) {
		*( VMVP ) = gngFastDoubleValue( a ) < gngFastDoubleValue( b ) ? SYS_TRUE : SYS_FALSE;
		RETURN( pc + 1 );
	} 
} 

throw Mishap( "LT: Numbers needed" ).culprit( "First", refToString( a ) ).culprit( "Second", refToString( b ) );
RETURN( NULL ); // sop for compiler.
