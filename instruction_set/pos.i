/*****************************************************************************\
Definition
	* POSITIVATE ( X : Num ) -> ( X )

	There's really no virtue in having this as a machine instruction.
	This was created simply by cloning the code for negate, which also
	doesn't really deserve to be a machine instruction either. So
	a good refactoring might be to transform them into system functions.

Summary
	Checks the top of the stack is a number and leaves it
	unchanged.
	
Unchecked Precondition
	There must be at least one value on the stack.
	
Checked Preconditions
	X is a number
	
Postcondition
	None

\*****************************************************************************/

Ref rx = *( VMVP );
if ( not( IsSmall( rx ) || IsDouble( rx ) || IsBigInt( rx ) ) ) {
	throw Mishap( "Number needed (for +x)" ).culprit( "Value", refToShowString( rx ) );
}
RETURN( pc + 1 );