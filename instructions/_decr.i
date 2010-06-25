long x = ToLong( *( VMVP ) );
long sum = x - ToLong( IntToSmall( 1 ) );
if ( sum > x ) {
    throw Mishap( "integer overflow detected in x - 1" ).culprit( "x", "" + x );
}
*( VMVP ) = ToRef( sum );
RETURN( pc + 1 );


//*( VMVP ) = ToRef( ToLong( *( VMVP ) ) - ToLong( IntToSmall( 1 ) ) );
//RETURN( pc + 1 );
