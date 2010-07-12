long A = ToLong( pc[ -2 ] ); // field before the function key.
Ref *prev = VMSP;
VMSP += ToLong( VMSP[ -1 ] );
VMSP[ 0 ] = VMLINKFUNC;
VMSP[ 1 ] = VMLINK;
VMSP[ 2 ] = prev;
VMSP[ 3 ] = ToRef( pc[ -3 ] );
VMSP += SP_OVERHEAD;
if ( A != VMCOUNT ) {
    enter_error( VMCOUNT, A );
}
{
    while ( --A >= 0 ) {
        VMSP[ A ] = *( VMVP-- );
    }
}
RETURN( pc + 1 );
