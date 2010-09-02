long b = ToLong( *( VMVP-- ) );
*( VMVP ) = ToLong( *( VMVP ) ) > b ? sys_true : sys_false;
RETURN( pc + 1 );
