/*****************************************************************************\
Definition
    STRINGITERATE ( i : Small, s : String ) -> ( c, i' )

Code
    <stringiterate />
    
Summary
    The basis for string iteration. It is a portmanteau iterator instruction
    that is fused with a return.
    
Unchecked Precondition
    * There are two items on the stack: i & s
    * i: Small
    * s: String
    
Exceptions (Checked Preconditions)
    None
    
Result (Postcondition)      
    * Returns the next character in the string c
    * and an updated index i' - except at the end of the string when i' is termin
    * Returns to the calling function

\*****************************************************************************/

//  This instruction implements the fast iterator for vectors.
//  It may assume that there 2 items on the stack (i:Small, v:Vector) on input.
//  Since there were 2 inputs, it may therefore safely return 2 items
//  without a stack check.

Ref * string = RefToPtr4( VMVP[0] );
Ref len_as_small = string[ STRING_OFFSET_LENGTH ];
Ref index = VMVP[-1];

//  Is is safe to compare smalls this way.
if ( index < len_as_small ) {
    VMVP[-1] = CharToCharacter( reinterpret_cast< unsigned char * >( string + 1 )[ SmallToLong( index ) ] );
    VMVP[0] = ToRef( (long)index + (long)LongToSmall( 1 ) );            //  It is safe to add smalls like this too.
} else {
    VMVP[0] = SYS_TERMIN;
}

VMPCFUNC = VMLINKFUNC;
RETURN( VMLINK );
