/*****************************************************************************\
Definition
	EQ ( A, B ) -> ( A == B )

Code
    <eq />
    
Summary
	Removes two items from the top of the stack and compares them for 
	identity. If they are identical then true is pushed, otherwise false.
	
Unchecked Precondition
	There are at least two values on the stack.
	
Exceptions (Checked Preconditions)
	None
	
Result (Postcondition)
	The two items were removed.
	A single boolean result replaces them.
	Which is true iff the two values were identical.

\*****************************************************************************/

Ref r = *( VMVP-- );
*( VMVP ) = *( VMVP ) == r ? SYS_TRUE : SYS_FALSE;
RETURN( pc + 1 );
