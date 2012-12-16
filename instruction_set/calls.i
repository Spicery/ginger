/*****************************************************************************\

Instructions CALLS (CALL value on Stack)

Summary
	Takes one value F off the stack (unchecked) and calls it.

Unchecked Precondition
	There has to be at least one item on the stack.

Checked Preconditions
	The value F is a function object.

Postcondition
    The return address and the current function object
    are stashed (VMLINK, VMPCLINK).
    The current function object is updated to F.
    Control is then transferred to the start of F.

\*****************************************************************************/

Ref r = *( VMVP-- );
{    
    Ref *ptr;
    if ( IsObj( r ) && IsFunctionKey( *( ptr = RefToPtr4( r ) ) ) ) {
        VMLINK = pc + 1;
        VMLINKFUNC = VMPCFUNC;
        VMPCFUNC = ptr;
        RETURN( ptr + 1 );
    } else {
        VMVP += 1;
        VMCOUNT += 1;
        FREEZE;
        pc = sysApply( pc + 1, vm );
        MELT;
        RETURN( pc );
    }
}
