Ref r = *( VMVP-- );
{    
    Ref *ptr;
    if ( !IsObj( r ) ) call_error( r );
    ptr = RefToPtr4( r );
    if ( !IsFunction( *ptr ) ) call_error( r );
    VMLINK = pc + 1;
    VMLINKFUNC = VMPCFUNC;
    VMPCFUNC = ptr;
    RETURN( ptr + 1 );
}
