/*****************************************************************************\
Instruction TBD 

Summary
	
Unchecked Precondition
	
Exceptions (Checked Preconditions)
	
Result (Postcondition)		

\*****************************************************************************/

Ref rx = *VMVP;
if ( IsSmall( rx ) ) {
	long x = (long)rx;
	long sum = x - (long)LongToSmall( 1 );
	if ( sum > x ) {
		throw Mishap( "integer overflow detected in x - 1" ).culprit( "x", "" + x );
	}
	*( VMVP ) = ToRef( sum );
	RETURN( pc + 1 );
} else {
	throw Mishap( "Small integer needed" );
}

//*( VMVP ) = ToRef( ToLong( *( VMVP ) ) - ToLong( IntToSmall( 1 ) ) );
//RETURN( pc + 1 );
