Ref r = *( VMVP-- );
*( VMVP ) = *( VMVP ) == r ? sys_false : sys_true;
RETURN( pc + 1 );
