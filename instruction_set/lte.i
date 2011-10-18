long b = ToLong( *( VMVP-- ) );
*( VMVP ) = ToLong( *( VMVP ) ) <= b ? SYS_TRUE : SYS_FALSE;
RETURN( pc + 1 );
