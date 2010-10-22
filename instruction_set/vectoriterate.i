//	This instruction implements the fast iterator for vectors.
//	It may assume that there 2 items on the stack (i:Small, v:Vector) on input.
//	Since there were 2 inputs, it may therefore safely return 2 items
//	without a stack check.

Ref * vector = RefToPtr4( VMVP[0] );
Ref len_as_small = vector[ VECTOR_OFFSET_LENGTH ];
Ref index = VMVP[-1];

//	Is is safe to compare smalls this way.
if ( index > len_as_small ) {
	VMVP[0]	= sys_termin;
} else {
	VMVP[-1] = vector[ SmallToLong( index ) ];
	VMVP[0] = ToRef( (long)index + (long)LongToSmall( 1 ) );			//	It is safe to add smalls like this too.
}

VMPCFUNC = VMLINKFUNC;
RETURN( VMLINK );
