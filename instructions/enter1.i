Ref *prev = VMSP;
VMSP += ToLong( VMSP[ -1 ] );
VMSP[ 0 ] = VMLINK;
VMSP[ 1 ] = prev;
VMSP[ 2 ] = ToRef( pc[ -3 ] );
VMSP += 3;
if ( VMCOUNT != 1 ) {
    enter_error( VMCOUNT, 1 );
}
VMSP[ 0 ] = *( VMVP-- );
RETURN( pc + 1 );
