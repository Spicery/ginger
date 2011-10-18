Ref r = *( VMVP-- );
*( VMVP ) = *( VMVP ) == r ? SYS_FALSE : SYS_TRUE;
RETURN( pc + 1 );
