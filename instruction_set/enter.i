/*****************************************************************************\
Instruction TBD

Summary
	
Unchecked Precondition
	
Exceptions (Checked Preconditions)
	
Result (Postcondition)		

\*****************************************************************************/

//
//	ENTER only works as the 1st instruction of a function.
//	It uses -ve offsets to access the fields before the key.
//
long A = ToLong( pc[ -2 ] ); // field before the function key.
Ref *prev = VMSP;
VMSP -= SP_OVERHEAD + ToULong( pc[ -3 ] );
VMSP[ SP_PREV_SP ] = prev;
VMSP[ SP_LINK ] = VMLINK;
VMSP[ SP_FUNC ] = VMLINKFUNC;
if ( A != VMCOUNT ) {
    enter_error( VMCOUNT, A );
}
{
    while ( --A >= 0 ) {
        LOCAL( A ) = *( VMVP-- );
    }
}
RETURN( pc + 1 );
