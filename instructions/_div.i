long b = ToLong( *( VMVP-- ) );
*( VMVP ) = ToRef( ToLong( *( VMVP ) ) / b );
RETURN( pc + 1 );
