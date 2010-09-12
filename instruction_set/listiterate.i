//	This instruction implements the fast iterator for lists.
//	It may assume that there 2 items on the stack (L:List, _) on input.
//	Since there were 2 inputs, it may therefore safely return 2 items
//	without a stack check.

Ref list = VMVP[-1];	//	Ignore VMVP[0], which is a dummy value.
if ( IsPair( list ) ) {
	VMVP[0] = RefToPtr4( list )[ PAIR_TAIL_OFFSET ];
	VMVP[-1] = RefToPtr4( list )[ PAIR_HEAD_OFFSET ];
} else {
	VMVP[0] = sys_termin;	//	Signals exit.
}


VMPCFUNC = VMLINKFUNC;
RETURN( VMLINK );
