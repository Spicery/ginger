/*****************************************************************************\
Instruction TBD

Summary
	
Unchecked Precondition
	
Exceptions (Checked Preconditions)
	
Result (Postcondition)		

\*****************************************************************************/

//	This instruction implements the fast iterator for mixed record/vectors.
//	It may assume that there 2 items on the stack (i:Small, v:Mixed) on input.
//	Since there were 2 inputs, it may therefore safely return 2 items
//	without a stack check.

Ref * mixed = RefToPtr4( VMVP[0] );
const int delta = LengthOfSimpleKey( *mixed );
Ref len_as_small = AddSmall( mixed[ MIXED_LAYOUT_OFFSET_LENGTH ], LongToSmall( delta ) );
Ref index = VMVP[-1];

//	Is is safe to compare smalls this way.
if ( index > len_as_small ) {
	VMVP[0]	= SYS_TERMIN;
} else {
	VMVP[-1] = mixed[ SmallToLong( index ) + delta ];
	VMVP[0] = AddSmall( index, LongToSmall( 1 ) );			//	It is safe to add smalls like this too.
}

VMPCFUNC = VMLINKFUNC;
RETURN( VMLINK );
