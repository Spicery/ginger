/*****************************************************************************\
Instruction TBD

Summary
	
Unchecked Precondition
	
Exceptions (Checked Preconditions)
	
Result (Postcondition)		

\*****************************************************************************/

//	ENTER1 only works as the 1st instruction of a function.
Ref *prev = VMSP;
VMSP -= SP_OVERHEAD + ToULong( pc[ -3 ] );
VMSP[ SP_PREV_SP ] = prev;
VMSP[ SP_LINK ] = VMLINK;
VMSP[ SP_FUNC ] = VMLINKFUNC;
if ( VMCOUNT != 1 ) {
    enter_error( VMCOUNT, 1 );
}
LOCAL( 0 ) = *( VMVP-- );
RETURN( pc + 1 );
