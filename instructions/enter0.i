Ref *prev = VMSP;
VMSP += ToLong( VMSP[ -1 ] );
VMSP[ 0 ] = VMLINK;
VMSP[ 1 ] = prev;
VMSP[ 2 ] = ToRef( pc[ -3 ] );
VMSP += 3;
if ( VMCOUNT != 0 ) {
    enter_error( VMCOUNT, 0 );
}
RETURN( pc + 1 );
