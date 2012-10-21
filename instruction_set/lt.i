/*****************************************************************************\
Instruction TBD

Summary
	
Unchecked Precondition
	
Exceptions (Checked Preconditions)
	
Result (Postcondition)		

\*****************************************************************************/

Ref b = *( VMVP-- );
Ref a = *( VMVP );

if ( not IsSmall( a ) or not IsSmall( b ) ) {
	throw Mishap( "Small integers only" );
}

*( VMVP ) = ToLong( a ) < ToLong( b ) ? SYS_TRUE : SYS_FALSE;
RETURN( pc + 1 );
