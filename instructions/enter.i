long A = ToLong( pc[ -2 ] ); // field before the function key.
Ref *prev = VMSP;
#if CALL_STACK_LAYOUT_STYLE == CSLS_ORIGINAL
	VMSP += ToLong( VMSP[ SP_NSLOTS ] );
	VMSP[ 0 ] = VMLINKFUNC;
	VMSP[ 1 ] = VMLINK;
	VMSP[ 2 ] = prev;
	VMSP[ 3 ] = ToRef( pc[ -3 ] );
	VMSP += SP_OVERHEAD;
#elif CALL_STACK_LAYOUT_STYLE == CSLS_VARIANT
	VMSP -= SP_OVERHEAD + ToULong( pc[ -3 ] );
	VMSP[ SP_NSLOTS ] = pc[ -3 ];
	VMSP[ SP_PREV_SP ] = prev;
	VMSP[ SP_LINK ] = VMLINK;
	VMSP[ SP_FUNC ] = VMLINKFUNC;
#elif CALL_STACK_LAYOUT_STYLE == CSLS_NO_NSLOT
	VMSP -= SP_OVERHEAD + ToULong( pc[ -3 ] );
	VMSP[ SP_PREV_SP ] = prev;
	VMSP[ SP_LINK ] = VMLINK;
	VMSP[ SP_FUNC ] = VMLINKFUNC;
#else
	#error
#endif
if ( A != VMCOUNT ) {
    enter_error( VMCOUNT, A );
}
{
    while ( --A >= 0 ) {
        LOCAL( A ) = *( VMVP-- );
    }
}
RETURN( pc + 1 );
