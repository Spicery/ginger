Ref *link;
link = (Ref *)VMSP[ -3 ];	//	recover return address
VMSP = (Ref *)VMSP[ -2 ];				//	restore stack pointer
#ifdef DBG_SPECIAL
	printf( "Recover link = %x\n", ToUInt( link ) );
	printf( "Stack pointer recovered as %x\n", ToUInt( VMSP ) );
#endif
RETURN( link );
