/*****************************************************************************\
Definition
    VECTORITERATE ( i:Small, v:Vector ) -> ( x, i' )

Code
    <vectoriterate />

Summary
    The basis for vector iteration. It is a portmanteau iterator instruction
    that is fused with a return.
    
Unchecked Precondition
    * There are two items on the stack: i & v
    * i: Small
    * v: Vector
    
Exceptions (Checked Preconditions)
    None
    
Result (Postcondition)      
    * Returns the next value in the sequence x
    * and an updated index i' - except at the end of the sequence when i' is  termin
    * Returns to the calling function

\*****************************************************************************/

//  This instruction implements the fast iterator for vectors.
//  It may assume that there 2 items on the stack (i:Small, v:Vector) on input.
//  Since there were 2 inputs, it may therefore safely return 2 items
//  without a stack check.

Ref * vector = RefToPtr4( VMVP[0] );
Ref len_as_small = vector[ VECTOR_LAYOUT_OFFSET_LENGTH ];
Ref index = VMVP[-1];

//  Is is safe to compare smalls this way.
if ( index > len_as_small ) {
    VMVP[0] = SYS_TERMIN;
} else {
    VMVP[-1] = vector[ SmallToLong( index ) ];
    VMVP[0] = ToRef( (long)index + (long)LongToSmall( 1 ) );            //  It is safe to add smalls like this too.
}

VMPCFUNC = VMLINKFUNC;
RETURN( VMLINK );
