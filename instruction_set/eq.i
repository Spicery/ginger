Ref r = *( VMVP-- );
*( VMVP ) = *( VMVP ) == r ? sys_true : sys_false;
RETURN( pc + 1 );
