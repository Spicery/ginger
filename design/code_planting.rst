Tidying up Code Planting 
========================

Instructions::

	vmiPUSH( VIdent src )
	llvmiPUHC( Ref r )
	llvmiPUSHS()
	llvmiPUSHL( int n )
	llvmiPUSHG( Valof v )

	vmiCOPY( VIdent src, VIdent dst )
	llvmiPOPL( int i )
	llvmiPOPG( Valof v )
	llvmiPUSHC_POPL( Ref r, int i )
	llvmiPUSHC_POPG( Ref r, Valof v )


	  dst C      S      L        G
	src +---+-------+-------+-------+
	C   |   | PUSHC | PUSHS | PUSHC |
	    |   |       | POPL  | POPG  |
	    +---+-------+-------+-------+
	S   |   | n/a   | POPL  | POPG  |
	    +---+-------+-------+-------+
	L   |   | PUSHL | PUSHL | PUSHL |
	    |   |       | POPL  | POPG  |
	    +---+-------+-------+-------+
	G   |   | PUSHG | PUSHG | PUSHG |
	    |   |       | POPL  | POPG  |
	    +---+-------+-------+-------+
