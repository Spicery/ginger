Ref *linkptr;
VMPCFUNC = ToRefRef( VMSP[ SP_FUNC ] );
linkptr = ToRefRef( VMSP[ SP_LINK ] );				//	recover return address
VMSP = ToRefRef( VMSP[ SP_PREV_SP ] );				//	restore stack pointer
#ifdef DBG_SPECIAL
	printf( "Recover link = %x\n", ToUInt( link ) );
	printf( "Stack pointer recovered as %x\n", ToUInt( VMSP ) );
#endif
RETURN( linkptr );
