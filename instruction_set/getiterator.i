/*****************************************************************************\
Definition
	* GETITERATOR ( OBJ ) -> ( state, context, next_fn )

Code
    <getiterator />
    
Summary
	This provides an interface to sysFastGetFastIterator, which is the
	basis for looping.
	
Unchecked Precondition
	* There is an object OBJ on the state to iterate over.
	
Exceptions (Checked Preconditions)
	None
	
Result (Postcondition)		
	* The OBJ is popped from the stack and replaced by state, context
	  and next_fn. For more details read `the section on iterators 
	  <iterators.html>`_.

\*****************************************************************************/

FREEZE;
pc = sysFastGetFastIterator( pc, vm );
MELT;
RETURN( pc + 1 );
