/*****************************************************************************\
Definition
	ESCAPE

Summary
	Escapes from a function call. The ESCAPE instruction will inspect the 
	instruction that the parent procedure is executing and if it is a 
	BYPASS it will modify the return address to continue past (i.e. bypass) 
	that instruction. 
	
Unchecked Precondition
	
Exceptions (Checked Preconditions)
	
Result (Postcondition)		

\*****************************************************************************/

Ref * linkptr = ToRefRef( VMSP[ SP_LINK ] );				//	recover return address

//	Apart from this test, it should be the same as a
//	regular return instruction.
if ( vm->instructionSet().isByPassInstruction( linkptr ) ) {
	//	Skip the bypass instruction.
	linkptr += 2;	//	Width of the bypass instruction.
	VMPCFUNC = ToRefRef( VMSP[ SP_FUNC ] );
	VMSP = ToRefRef( VMSP[ SP_PREV_SP ] );				//	restore stack pointer
	#ifdef DBG_SPECIAL
		cout << "Recover link = " << hex << ToUInt( link ) << dec << endl;
		cout << "Stack pointer recovered as " << hex << ToUInt( VMSP ) << dec << endl;
	#endif
	RETURN( linkptr );
} else {
	Ref event = VMVP[ 0 ];
	Mishap mishap( "Untrapped escape" );
	mishap.culprit( "Event", refToShowString( event ) );
	Cell args( VMVP[ -1 ] );
	if ( args.isVectorObject() ) {
		VectorObject vargs( args.asHeapObject() );
		for ( VectorObject::generator g( vargs ); !!g; ++g ) {
			Cell x = *g;
			mishap.culprit( "Event Value", x.toShowString() );
		}
	}
	throw mishap;
}