Ref r = *( VMVP-- );
{    
    Ref *ptr;
    if ( !IsPtr4( r ) ) call_error( r );
    ptr = RefToPtr4( r );
    if ( !IsFunction( *ptr ) ) call_error( r );
    VMLINK = pc + 1;
    RETURN( ptr + 1 );
}
