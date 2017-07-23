/*****************************************************************************\
Definition
    ENTER1

Code
	No representation needed - the backend automatically generates the
	optimal enter instruction.

Summary
    A simplified version of ENTER for unary function objects.
    
Unchecked Precondition
    Must be the first instruction of a function object.
    
Exceptions (Checked Preconditions)
    0 or 2+ number of arguments were passed.
    
Result (Postcondition)  
    As for ENTER    

\*****************************************************************************/

//  ENTER1 only works as the 1st instruction of a function.
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
