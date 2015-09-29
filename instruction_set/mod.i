/*****************************************************************************\
Definition
	* MOD_I ( X : Small, Y : Small ) -> ( R ), where R = X mod Y
	* VPC += 1

Summary
	Computes the integer modulus R of X and Y, such that R is in the
	half open interval [0,Y). X and Y are removed from the stack and 
	R is pushed.
	
Unchecked Precondition
	* There are two items on the stack.
	
Exceptions (Checked Preconditions)
	* X and Y are Smalls
	
Result (Postcondition)		
	* R = X mod Y is pushed.
	* Execution continues at the next instruction.

Tags
	* Arithmetic operator

\*****************************************************************************/

Ref ry = *( VMVP-- );
Ref rx = *( VMVP );
if ( IsSmall( rx ) && IsSmall( ry ) and not( IsZeroSmall( ry ) ) ) {
    const long a = ToLong( rx );
    const long b = ToLong( ry );
    *( VMVP ) = ToRef( a % b );
    RETURN( pc + 1 );
}
FREEZE;
pc = sysModHelper( ++pc, vm, ry );
MELT;
RETURN( pc );
