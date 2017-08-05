/*****************************************************************************\
Definition
	INCR ( X ) -> ( X1 )

Code
    <incr />
    n.b. Using the more general instruction <incr.by by="1"/>
	is fine as peephole optimisation in the Ginger Runtime
	makes them equal.
    
Summary
	Increments X, the value on top of the stack, by 1.

Stats
	Width = 1
	
Unchecked Precondition
	There is at least one value on the value stack.
	
Exceptions (Checked Preconditions)
	The value on top of the stack is a number.
	
Result (Postcondition)
	The top of the stack X1 is one greater than before.

Tags
	* Arithmetic operator

\*****************************************************************************/

Ref rx = *VMVP;
if ( IsSmall( rx ) ) {
	const long x = (long)rx;
	const long sum = x + (long)LongToSmall( 1 );
	if ( sum > x ) {
		*( VMVP ) = ToRef( sum );
		RETURN( pc + 1 );
	} 
} 
pc += 1;
FREEZE;
pc = sysAddHelper( pc, vm, LongToSmall( 1 ) );
MELT;
RETURN( pc );
