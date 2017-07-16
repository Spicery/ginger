/*****************************************************************************\
Definition
    LISTITERATE ( L: List, DUMMY ) -> ( Head, Tail : List )

Code
    <listiterate />
    
Summary
    The basis for list iteration. It is a portmanteau iterator instruction
    that is fused with a return.
    
Unchecked Precondition
    * There are two items on the stack: L & DUMMY
    * L is a reference that is marched down the list
    * DUMMY is ignored
    
Exceptions (Checked Preconditions)
    None
    
Result (Postcondition)      
    * Returns the next value in the sequence Head
    * and an updated list Tail - except at the end of the sequence when termin
      is returned
    * Returns to the calling function

\*****************************************************************************/

//  This instruction implements the fast iterator for lists.
//  It may assume that there 2 items on the stack (L:List, _) on input.
//  Since there were 2 inputs, it may therefore safely return 2 items
//  without a stack check.

Ref list = VMVP[-1];    //  Ignore VMVP[0], which is a dummy value.
if ( IsPair( list ) ) {
    VMVP[0] = RefToPtr4( list )[ PAIR_TAIL_OFFSET ];
    VMVP[-1] = RefToPtr4( list )[ PAIR_HEAD_OFFSET ];
} else {
    VMVP[0] = SYS_TERMIN;   //  Signals exit.
}


VMPCFUNC = VMLINKFUNC;
RETURN( VMLINK );
