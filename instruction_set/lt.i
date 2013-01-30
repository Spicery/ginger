/*****************************************************************************\
Definition
	* LT ( A, B ) -> ( R ), where R = A < B
	* VPC += 1

Summary
	Removes the top two items from the value stack, which must be
	numbers, and compares them using the greater than or equal to operator.
	
Unchecked Precondition
	* There are two values A & B on the stack.
	
Exceptions (Checked Preconditions)
	* A and B must both be numbers.
	
Result (Postcondition)		
	* A & B are removed from the stack and replaced by R.
	* R is a boolean.
	* R is true if A < B, else false.
	* Execution continues at the next instruction.

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
