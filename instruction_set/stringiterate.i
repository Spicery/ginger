/*****************************************************************************\
Instruction TBD

Summary
	
Unchecked Precondition
	
Exceptions (Checked Preconditions)
	
Result (Postcondition)		

\*****************************************************************************/

//	This instruction implements the fast iterator for vectors.
//	It may assume that there 2 items on the stack (i:Small, v:Vector) on input.
//	Since there were 2 inputs, it may therefore safely return 2 items
//	without a stack check.

Ref * string = RefToPtr4( VMVP[0] );
Ref len_as_small = string[ STRING_OFFSET_LENGTH ];
Ref index = VMVP[-1];

//	Is is safe to compare smalls this way.
if ( index < len_as_small ) {
	VMVP[-1] = CharToCharacter( reinterpret_cast< unsigned char * >( string + 1 )[ SmallToLong( index ) ] );
	VMVP[0] = ToRef( (long)index + (long)LongToSmall( 1 ) );			//	It is safe to add smalls like this too.
} else {
	VMVP[0]	= SYS_TERMIN;
}

VMPCFUNC = VMLINKFUNC;
RETURN( VMLINK );
