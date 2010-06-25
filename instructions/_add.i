long y = ToLong( *( VMVP-- ) );
long x = ToLong( *( VMVP ) );
long sum = x + y;
if ( x < 0 ? sum > y : sum < y ) {
    throw Mishap( "Overflow detected in +" ).culprit( "Argument1", "" + x ).culprit( "Argument2", "" + y );
}
*VMVP = ToRef( sum );
RETURN( pc + 1 );

//int y = ToLong( *( VMVP-- ) );
//*VMVP = ToRef( ToLong( *( VMVP ) ) + y );
//RETURN( pc + 1 );

