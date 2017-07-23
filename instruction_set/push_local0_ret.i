/*****************************************************************************\
Definition
    PUSH_LOCAL0_RET () -> ( R ), R = LOCAL[ 0 ]
    
Code
    No representation needed as backend will peephole optimise 
    <push.local.ret local="0" />.

Summary
    Pushes the value of LOCAL[ 0 ] onto the stack and returns.
    
Unchecked Precondition
    * There is room on the stack for at least one more value.
    * The current frame has at least one local variable.
    * There is a calling function to return to.
    
Exceptions (Checked Preconditions)
    None.
    
Result (Postcondition)      
    * The top of stack is LOCAL[ 0 ]
    * The stack is one bigger.
    * VMPCFUNC, VMSPC, VPC all restored from call-stack.
    * Execution is returned to the calling function.

\*****************************************************************************/

*( ++VMVP ) = LOCAL(0);
Ref *linkptr;
VMPCFUNC = ToRefRef( VMSP[ SP_FUNC ] );
linkptr = ToRefRef( VMSP[ SP_LINK ] );              //  recover return address
VMSP = ToRefRef( VMSP[ SP_PREV_SP ] );              //  restore stack pointer
#ifdef DBG_SPECIAL
    cout << "Recover link = " << hex << ToUInt( link ) << dec << endl;
    cout << "Stack pointer recovered as " << hex << ToUInt( VMSP ) << dec << endl;
#endif
RETURN( linkptr );
