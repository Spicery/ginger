/*****************************************************************************\
Instruction ADDI

Summary
	Takes two smalls X & Y off the value stack and replaces them
	with their sum, which should be a small.
	
Unchecked Precondition
	There must be at least two values on the stack.
	
Checked Preconditions
	Both X & Y are small.

Exceptions
	The sum of X + Y is too large to fit in a small.
	
Postcondition
	X & Y are replaced by X+Y

\*****************************************************************************/


//	Instruction ADDI 
//	Summary
//		Takes two smalls off the value stack and replaces them
//		with their sum, which should be a small.
//	Preconditions
//		There must be at least two values on the stack.
//	Exceptions
//		1.	One or more of the values on the stack is not small.
//		2.	The total is too large to fit in a small.
//
Ref ry = *( VMVP-- );
Ref rx = *( VMVP );
if ( IsSmall( rx ) && IsSmall( ry ) ) {
	long y = (long)ry;
	long x = (long)rx;
	long sum = x + y;
	if ( x < 0 ? sum <= y : sum >= y ) {
		*VMVP = ToRef( sum );
		RETURN( pc + 1 );
	} else {
    	throw Mishap( "Overflow detected in +" );
	}
} else {
	throw Mishap( "Small integers only" );
} 
