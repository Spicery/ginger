/*****************************************************************************\
Definition
    MIXEDITERATE ( i:Small, m:Mixed ) -> ( x, i' )

Summary
    The basis for mixed record/vector iteration. It is a portmanteau 
    iterator instruction that is fused with a return.
    
Unchecked Precondition
    * There are two items on the stack: i & m
    * i: Small
    * m: Mixed Record/Vector
    
Exceptions (Checked Preconditions)
    None
    
Result (Postcondition)      
    * Returns the next value in the sequence x
    * and an updated index i' - except at the end of the sequence when i' is termin
    * Returns to the calling function

\*****************************************************************************/

//  This instruction implements the fast iterator for mixed record/vectors.
//  It may assume that there 2 items on the stack (i:Small, v:Mixed) on input.
//  Since there were 2 inputs, it may therefore safely return 2 items
//  without a stack check.

Ref * mixed = RefToPtr4( VMVP[0] );
const int delta = LengthOfSimpleKey( *mixed );
Ref len_as_small = AddSmall( mixed[ MIXED_LAYOUT_OFFSET_LENGTH ], LongToSmall( delta ) );
Ref index = VMVP[-1];

//  Is is safe to compare smalls this way.
if ( index > len_as_small ) {
    VMVP[0] = SYS_TERMIN;
} else {
    VMVP[-1] = mixed[ SmallToLong( index ) + delta ];
    VMVP[0] = AddSmall( index, LongToSmall( 1 ) );          //  It is safe to add smalls like this too.
}

VMPCFUNC = VMLINKFUNC;
RETURN( VMLINK );
