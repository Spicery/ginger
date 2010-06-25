long A = ToLong( pc[ -2 ] );
Ref *prev = VMSP;
VMSP += ToLong( VMSP[ -1 ] );
VMSP[ 0 ] = VMLINK;
VMSP[ 1 ] = prev;
VMSP[ 2 ] = ToRef( pc[ -3 ] );
VMSP += 3;
if ( A != VMCOUNT ) enter_error( VMCOUNT, A );
{    int i;
    for ( i = 0; i < A; i++ ) {
        VMSP[ i ] = *( VMVP-- );
    }
}
RETURN( pc + 1 );
