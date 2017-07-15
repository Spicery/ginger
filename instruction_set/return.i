/*****************************************************************************\
Definition
    RETURN

Summary
    Returns from a function call.
    
Unchecked Precondition
    
Exceptions (Checked Preconditions)
    
Result (Postcondition)      

\*****************************************************************************/

Ref *linkptr;
VMPCFUNC = ToRefRef( VMSP[ SP_FUNC ] );
linkptr = ToRefRef( VMSP[ SP_LINK ] );              //  recover return address
VMSP = ToRefRef( VMSP[ SP_PREV_SP ] );              //  restore stack pointer
#ifdef DBG_SPECIAL
    cout << "Recover link = " << hex << ToUInt( link ) << dec << endl;
    cout << "Stack pointer recovered as " << hex << ToUInt( VMSP ) << dec << endl;
#endif
RETURN( linkptr );
