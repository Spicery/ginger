/*****************************************************************************\
Definition
    ENTER

Summary
    This is the most general instruction for setting up a non-leaf
    function call i.e. one that pushes onto the callstack. It should
    be matched with a RETURN instruction to cut the callstack back.
    
Unchecked Precondition
    This MUST be the first instruction of a function so that the fields
    of the function object can be accessed by an offset.
    
Exceptions (Checked Preconditions)
    VMCOUNT is not consistent with the number of required arguments.
    
Result (Postcondition)      
    * The callstack pointer, the link pointer and the link-function pointer
    * are pushed onto the callstack.
    * The arguments are pushed in order onto the callstack.

\*****************************************************************************/

//
//  ENTER only works as the 1st instruction of a function.
//  It uses -ve offsets to access the fields before the key.
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
