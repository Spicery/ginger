if( *( VMVP-- ) != SYS_FALSE ) {
	RETURN( pc + 1 );									//	Go onto the next instruction.
} else {
	Ref *linkptr;
	VMPCFUNC = ToRefRef( VMSP[ SP_FUNC ] );
	linkptr = ToRefRef( VMSP[ SP_LINK ] );				//	recover return address
	VMSP = ToRefRef( VMSP[ SP_PREV_SP ] );				//	restore stack pointer
	RETURN( linkptr );
}
