How to Compile Queries
======================

Intro
-----
Each query has these parts: INIT (initialise), TEST (fused test-next-fini), 
BODY and ADVN (advance). These must be called in a specific order::

	INIT is called first
	TEST follows INIT or ADVN
	BODY follows TEST
	ADVN follows BODY

The TEST part is passed two labels - one for success and the other for failure.
The TEST part should continue at the appropriate label. (The special label
fallthru allows jumps to be eliminated.)

PATTERN in EXPR
~~~~~~~~~~~~~~~

::

	INIT
		Allocates three variable:
			next.fn
			context
			state	
		
	TEST
		Equivalent to

			label again:
				next.fn( state, context ) -> ( loop.var, state )
				if state == termin then goto FAIL endif
				BIND( PATTERN, loop.var )( PASS, again )


		The iteration halts if state is TERMIN.

	ADVN
		Skip

PATTERN from A by B to C
~~~~~~~~~~~~~~~~~~~~~~~~

TODO:

Setup for Loops, Conditionals and Naked Queries
-----------------------------------------------

A loop for query Q with body B is therefore compiled like this::

		INIT[ Q ]
		goto label test

	body:
		BODY[ Q ]
		ADVN[ Q ]	# advance after body is executed. Sets up state.
	test:
		# TEST[ Q ] takes a pair of (pass,fail) labels. On success it 
		# continues at 'pass', on failure it continues at 'fail'.
		TEST[ Q ]( label body, label done )
	done:

Conditionals have three parts IFQ, THEN, ELSE and are compiled like this::

		INIT[ IFQ ]
		TEST[ IFQ ]( label pass, label fail )
	pass: 
		THEN; goto label done
	fail: 
		ELSE
	done:

Naked queries are even simpler::

		INIT[ IFQ ]
		TEST[ IFQ ]( label pass, label fail )
	label fail:
		throw 
	label pass:


Parallel Loops P // Q
---------------------

::

	INIT[ P // Q ] --> INIT[ P ]; INIT[ Q ]

	TEST[ P // Q ]( pass, fail ) -->
			TEST[ P ]( label ok, fail )
		ok:
			TEST[ Q ]( pass, label done )
		done:

	ADVN[ P // Q ] --> ADVN[ P ]; ADVN[ Q ]
	BODY[ P // Q ] --> BODY[ P ]; BODY[ Q ]

Q where C
---------
::

	INIT[ Q where C ] --> INIT[ Q ]

	TEST[ Q where C ]( pass, fail ) --> 
		start:
			TEST[ Q ]( label ok, fail )
		label ok:
			if C then goto pass
		ADV[ Q ]
			goto label start
		
	ADVN[ Q where C ] --> ADVN[ Q ]
	BODY[ Q where C ] --> BODY[ Q ]

Q do S
------
::
	INIT[ Q do S ] --> INIT[ Q ]
	TEST[ Q do S ] --> TEST[ Q ]
	ADVN[ Q do S ] --> ADV[ Q ]
	BODY[ Q do S ] --> BODY[ Q ]; S 

Q finally S
-----------
::

	INIT[ Q finally S ] --> INIT[ Q ]
	
	TEST[ Q finally S ]( pass, fail ) --> 
			TEST[ Q ]( pass, label cont )
		cont:
			S

	ADVN[ Q finally S ] --> ADV[ Q ]
	BODY[ Q finally S ] --> BODY[ Q ]

Q while C then S else T
-----------------------
::

	INIT[ Q while C then S ] --> INIT[ Q ]

	TEST[ Q while C then S ]( pass, fail ) --> 
			TEST[ Q ]( label y, fail )
		y:
			if C then goto pass;
			S
			goto fail

	
	ADVN[ Q while C then S else T ] --> ADV[ Q ]
	BODY[ Q while C then S else T ] --> BODY[ Q ]
	
	
Q until C then S else T
-----------------------
::

	INIT[ Q until C then S ] --> INIT[ Q ]

	TEST[ Q until C then S ]( pass, fail ) --> 
			TEST[ Q ]( label y, fail )
		y:
			if not C then goto pass
			S
		n:
			goto pass

	
	ADVN[ Q until C then S else T ] --> ADV[ Q ]
	BODY[ Q until C then S else T ] --> BODY[ Q ]
	


Nested Loops P & Q
------------------
::

	INIT( P & Q ) -->
		need_testp := true
		INIT[ P ]

	TEST[ P & Q ]( pass, fail ) -->
		repeat
			if need_testp then
				TEST[ P ]( label ok, fail )
			  ok:
				need_testp ::= false
				BODY[ P ]
				INIT[ Q ]
			else
				TEST[ Q ]( pass, label n )
			  n:
				need_testp ::= true
				FINI[ Q ]
				ADVN[ P ]
			endif
		endrepeat

	ADV[ P & Q ] -->
		ADV[ Q ]

	BODY[ P & Q ] -->
		BODY[ Q ]
