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
if ( VMCOUNT != 0 ) {
    enter_error( VMCOUNT, 0 );
}
RETURN( pc + 1 );
