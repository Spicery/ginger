Ref *prev = VMSP;
VMSP += ToLong( VMSP[ -1 ] );
VMSP[ 0 ] = VMLINKFUNC;
VMSP[ 1 ] = VMLINK;
VMSP[ 2 ] = prev;
VMSP[ 3 ] = ToRef( pc[ -3 ] );
VMSP += SP_OVERHEAD;
if ( VMCOUNT != 0 ) {
    enter_error( VMCOUNT, 0 );
}
RETURN( pc + 1 );
