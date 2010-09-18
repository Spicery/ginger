//	ENTER0 only works as the 1st instruction of a function.
Ref *prev = VMSP;
VMSP -= SP_OVERHEAD + ToULong( pc[ -3 ] );
VMSP[ SP_PREV_SP ] = prev;
VMSP[ SP_LINK ] = VMLINK;
VMSP[ SP_FUNC ] = VMLINKFUNC;
if ( VMCOUNT != 0 ) {
    enter_error( VMCOUNT, 0 );
}
RETURN( pc + 1 );
